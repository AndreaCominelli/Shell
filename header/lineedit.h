#ifndef LINEEDIT_H
#define LINEEDIT_H

/*
 * Legge una riga mostrando il prompt, con supporto a backspace e
 * cronologia (freccia su/giu).
 * Ritorna una stringa allocata (da liberare con free), oppure NULL su EOF
 * (Ctrl-D a riga vuota). Con Ctrl-C ritorna una stringa vuota.
 */
char *readLine(const char *prompt);

/* Aggiunge una riga alla cronologia (copia la stringa, ignora vuote/duplicate consecutive). */
void addHistory(const char *line);

/* Libera la cronologia (da chiamare prima di uscire). */
void freeHistory(void);

void printHist(void);

#endif