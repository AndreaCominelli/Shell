#include "../header/executor.h"
#include "../header/utils.h"
#include "../header/parser.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>

size_t countRedir(Redir *r) {
    if (r == NULL) return 0;
    
    size_t count = 0;
    while(r) {
        count++;
        r = r->next;
    }

    return count;
}

int getFlag(int op) {
    switch(op) {
        case 0:
            return O_RDONLY;
        case 1:
            return O_CREAT | O_TRUNC | O_WRONLY;
        case 2:
            return O_CREAT | O_WRONLY | O_APPEND;
        default:
            return -1;
    }
}

int getChannel(int op) {
    switch(op) {
        case 0:
            return 0;
        default:
            return 1;
    }
}

int openRedir(PToken *p, int *fds) {
    if (p == NULL || p->type != PTYPE_COMMAND) {
        printf("Invalid token: ");
        printArgv(p);
        return -1;
    }
    int i = 0;
    while(p->cmd.red) {
        const char *filename = (const char *)p->cmd.red->target;
        int op = p->cmd.red->type;

        int flags = getFlag(op);
        if (flags == -1) {
            printf("Invalid operation '%d'\n", op);
            return -1;
        }

        int fd = open(filename, flags);
        if (fd == -1) {
            fprintf(stderr, "Unable to open file '%s': %s\n", filename, strerror(errno));
            return -1;
        }

        int res = dup2(fd, getChannel(op));
        if (res == -1) {
            fprintf(stderr, "Unable to dup '%d': %s\n", op, strerror(errno));
            return -1;
        }

        fds[i] = fd;
        i++;
        p->cmd.red = p->cmd.red->next;
    }
    return 0;
}

void closeRedir(int *fds, size_t len) {
    for(int i = 0; i < len; i++) {
        close(fds[i]);
    }
}

void startprg(const char *prg, char **argv) {
    if (prg != NULL) {
        int res = execvp(prg, argv);
        if (res == -1) {
            fprintf(stderr, "Unable to execute process: %s\n", strerror(errno));
            exit(1); // exit uccide il processo. Devo ritornare un intero?
        }
        // exec ritorna solo se c'è stato un'errore. Altimenti non ritorna niente
    }
    exit(1);
}

int execute(PToken *p, Map *m, int *in, int *out) {
    // check if p is null or not
    if (p->type == PTYPE_COMMAND) {
        const char *prg = p->cmd.argv[0];
        Entry *e = get(m, prg);
        if(e) {
            // need to store the original fds pointing to stdin and stdout
            int saved_in  = dup(0);
            int saved_out = dup(1);

            size_t numredir = countRedir(p->cmd.red);
            int *fds = (int *)xmalloc(sizeof(int) * numredir);
            int ret = openRedir(p, fds);
            if (ret == -1) {
                exit(1); // exit uccide il processo. Devo ritornare un intero?
            }
            int res = e->func(p->cmd.argv);

            fflush(stdout);
            dup2(saved_in, 0);
            dup2(saved_out, 1);
            close(saved_in);
            close(saved_out);

            closeRedir(fds, numredir); // closes all the file descriptors
            free(fds); //free fds array
            return res;
        } else {
            // fork / execvp
            int pid = fork();
            if (pid == -1) {
                perror("fork");
            } else if (pid != 0) { // father
                int wstatus;
                pid_t childpit = waitpid(pid, &wstatus, 0);
                if (childpit == -1) {
                    perror("waitpid");
                } else if (WIFEXITED(wstatus)) {
                    return WEXITSTATUS(wstatus);   /* exit code del figlio */
                } else if (WIFSIGNALED(wstatus)) {
                    return WTERMSIG(wstatus);       /* terminato da un segnale */
                }
            } else {
                size_t numredir = countRedir(p->cmd.red);
                int *fds = (int *)xmalloc(sizeof(int) * numredir);
                int ret = openRedir(p, fds);
                if (ret == -1) {
                    exit(1); // exit uccide il processo. Devo ritornare un intero?
                }
                if (in) {
                    int res = dup2(*in, 0);
                    if (res == -1) {
                        fprintf(stderr, "Unable to dup '%d': %s\n", *in, strerror(errno));
                        return -1;
                    }
                    close(*in);
                    startprg(prg, p->cmd.argv);
                }
                if (out) {
                    int res = dup2(*out, 1);
                    if (res == -1) {
                        fprintf(stderr, "Unable to dup '%d': %s\n", *out, strerror(errno));
                        return -1;
                    }
                    close(*out);
                    startprg(prg, p->cmd.argv);
                }
                if(in == NULL && out == NULL) {
                    startprg(prg, p->cmd.argv);
                }
                closeRedir(fds, numredir); // closes all the file descriptors
                free(fds); //free fds array
                return 0; // TO BE CHECKED
            }
            return -1;
        }
    } else {
        if (p->type == PTYPE_OPERATOR) {
            if(strcmp(p->op.name, "|") == 0) {
                int fd[2];
                pipe(fd);
                
                int retleft = execute(p->op.left, m, NULL, &fd[1]);
                close(fd[1]);
                int retright = execute(p->op.right, m, &fd[0], NULL);
                close(fd[0]);

                return retleft | retright;
            }
            if(strcmp(p->op.name, ";") == 0) {
                execute(p->op.left, m, NULL, NULL);
                int retright = execute(p->op.right, m, NULL, NULL);
                
                return retright;
            }
            if(strcmp(p->op.name, "&&") == 0) {
                int retleft = execute(p->op.left, m, NULL, NULL);
                if (retleft != 0) {
                    return retleft;
                }
                
                int retright = execute(p->op.right, m, NULL, NULL);
                return retright;
            }
            if(strcmp(p->op.name, "||") == 0) {
                int retleft = execute(p->op.left, m, NULL, NULL);
                if (retleft == 0) {
                    return retleft;
                }
                
                int retright = execute(p->op.right, m, NULL, NULL);
                return retright;
            }

        }
        return -1;
    }
}