#define _POSIX_C_SOURCE 200809L
#include "../header/lineedit.h"
#include "../header/utils.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#define HISTORY_BASE_SIZE 10

/* ---------- cronologia ---------- */

static char **history = NULL;
static size_t hist_count = 0;
static size_t hist_cap = 0;

void addHistory(const char *line) {
    if (line == NULL || line[0] == '\0') return;
    if (hist_count > 0 && strcmp(history[hist_count - 1], line) == 0) return;

    if (hist_count == hist_cap) {
        size_t ncap = hist_cap ? hist_cap * 2 : 16;
        char **tmp = realloc(history, ncap * sizeof(char *));
        if (tmp == NULL) return;
        history = tmp;
        hist_cap = ncap;
    }
    char *copy = strdup(line);
    if (copy == NULL) return;
    history[hist_count++] = copy;
}

void freeHistory(void) {
    for (size_t i = 0; i < hist_count; i++) free(history[i]);
    free(history);
    history = NULL;
    hist_count = hist_cap = 0;
}

void printHist(void) {
     for (int i = 0; i < hist_count; i++) {
        printf("%d: %s\n", i+1, history[i]);
     }
}

/* ---------- modalita' raw ---------- */

static struct termios orig_termios;
static int raw_enabled = 0;

static void disableRaw(void) {
    if (raw_enabled) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
        raw_enabled = 0;
    }
}

static int enableRaw(void) {
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) return -1;

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO | ISIG); /* niente buffering, eco e segnali: gestiti da noi */
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) return -1;

    raw_enabled = 1;
    return 0;
}

/* ---------- utilita' sul buffer ---------- */

/* Garantisce che il buffer possa contenere almeno `need` byte (terminatore incluso). */
static int ensureCap(char **buf, size_t *cap, size_t need) {
    if (need <= *cap) return 0;
    size_t ncap = *cap;
    while (ncap < need) ncap *= 2;
    char *tmp = realloc(*buf, ncap);
    if (tmp == NULL) return -1;
    *buf = tmp;
    *cap = ncap;
    return 0;
}

/* Sostituisce il contenuto del buffer con s. */
static int setLine(char **buf, size_t *cap, size_t *len, const char *s) {
    size_t n = strlen(s);
    if (ensureCap(buf, cap, n + 1) == -1) return -1;
    memcpy(*buf, s, n + 1);
    *len = n;
    return 0;
}

static void redraw(const char *prompt, const char *line) {
    printf("\r%s%s\x1b[K", prompt, line);
    fflush(stdout);
}

/* Legge un byte; ritorna 1 se letto, 0 su EOF, -1 su errore. Riprova su EINTR. */
static int readByte(unsigned char *c) {
    while (1) {
        ssize_t n = read(STDIN_FILENO, c, 1);
        if (n == 1) return 1;
        if (n == 0) return 0;
        if (errno == EINTR) continue;
        return -1;
    }
}

/* ---------- fallback quando stdin non e' un terminale (pipe, file) ---------- */

static char *readLinePlain(void) {
    char *buf = NULL;
    size_t cap = 0;
    ssize_t n = getline(&buf, &cap, stdin);
    if (n == -1) {
        free(buf);
        return NULL;
    }
    if (n > 0 && buf[n - 1] == '\n') buf[n - 1] = '\0';
    return buf;
}

/* ---------- readLine ---------- */

char *readLine(const char *prompt) {
    if (!isatty(STDIN_FILENO)) return readLinePlain();

    fputs(prompt, stdout);
    fflush(stdout);

    if (enableRaw() == -1) return readLinePlain();

    size_t cap = 64, len = 0;
    char *line = malloc(cap);
    if (line == NULL) {
        disableRaw();
        return NULL;
    }
    line[0] = '\0';

    char *saved = NULL;          /* riga in corso di scrittura, salvata prima di navigare la cronologia */
    size_t idx = hist_count;     /* posizione nella cronologia (hist_count = "riga nuova") */
    int eof = 0;

    while (1) {
        unsigned char c;
        int r = readByte(&c);
        if (r <= 0) {
            eof = 1;
            break;
        }

        if (c == '\r' || c == '\n') {            /* Invio */
            fputs("\r\n", stdout);
            fflush(stdout);
            break;
        } else if (c == 3) {                     /* Ctrl-C: scarta la riga */
            fputs("^C\r\n", stdout);
            fflush(stdout);
            line[0] = '\0';
            len = 0;
            break;
        } else if (c == 4) {                     /* Ctrl-D */
            if (len == 0) {
                fputs("\r\n", stdout);
                fflush(stdout);
                eof = 1;
                break;
            }
        } else if (c == 127 || c == 8) {         /* Backspace */
            if (len > 0) {
                line[--len] = '\0';
                fputs("\b \b", stdout);
                fflush(stdout);
            }
        } else if (c == 27) {                    /* sequenza di escape */
            unsigned char s0, s1;
            if (readByte(&s0) != 1 || readByte(&s1) != 1) continue;
            if (s0 != '[' && s0 != 'O') continue;

            if (isdigit(s1)) {                   /* es. ESC [ 3 ~ (Canc): consuma il '~' */
                unsigned char tilde;
                readByte(&tilde);
                continue;
            }

            if (s1 == 'A') {
                // printf("SU\n");                    /* freccia su */
                if (idx > 0) {
                    if (idx == hist_count) {
                        free(saved);
                        saved = strdup(line);
                    }
                    idx--;
                    if (setLine(&line, &cap, &len, history[idx]) == 0) redraw(prompt, line);
                }
            } else if (s1 == 'B') {  
                // printf("GIU\n");             /* freccia giu' */
                if (idx < hist_count) {
                    idx++;
                    const char *src = (idx == hist_count) ? (saved ? saved : "") : history[idx];
                    if (setLine(&line, &cap, &len, src) == 0) redraw(prompt, line);
                }
            }
            /* destra/sinistra: ignorate (il cursore sta sempre a fine riga) */
        } else if (isprint(c)) {                 /* carattere normale */
            if (ensureCap(&line, &cap, len + 2) == -1) continue;
            line[len++] = (char)c;
            line[len] = '\0';
            fputc(c, stdout);
            fflush(stdout);
        }
    }

    disableRaw();
    free(saved);

    if (eof) {
        free(line);
        return NULL;
    }
    return line;
}