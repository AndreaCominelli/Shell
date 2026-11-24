# Custom shell

A small Unix shell written in C, built for learning purposes. It reads commands from the command line, parses them with a lexer and a parser, and runs them using `fork`/`execvp`.

## Features

- **External programs** (`ls -l`, `grep foo`, ...) via `fork` + `execvp` + `waitpid`
- **Pipes**: `ls -l | grep foo`
- **Redirections**: `<`, `>`, `>>`
- **Control operators**: `;`, `&&`, `||`
- **Builtin commands**, registered in a hashmap (name → function)
- **Command history**, navigable with the up/down arrow keys
- Exit status propagated between commands (used by `&&` and `||`)

## How it works

Input goes through four stages:

```
input line → Lexer → Parser → Executor
              tokens    AST     fork / pipe / dup2 / exec
```

1. **Lexer**: turns the input line into a list of tokens (words and operators), always taking the longest match (`>>` before `>`, `&&` before `&`).
2. **Parser**: *recursive descent*, with one function per level of the grammar. It builds a tree whose leaves are simple commands (`argv` + redirections) and whose inner nodes are operators. Precedence follows a real shell: `|` binds tighter than `&&`/`||`, which bind tighter than `;`.
3. **Executor**: walks the tree. Builtins run in the shell process (saving and restoring file descriptors when redirections are present), while external programs run in a child process. Redirections and pipes are implemented with `open`, `pipe`, `dup2` and `close`.

### Grammar

```
list        → pipeline ( (';' | '&&' | '||') pipeline )*
pipeline    → command ( '|' command )*
command     → WORD+ redirection*
redirection → ('<' | '>' | '>>') WORD
```

## Building

You need a C compiler and a POSIX system (Linux or macOS).

```sh
gcc -Wall -Wextra -g -o myshell main.c src/*.c
./myshell
```

To catch memory errors during development:

```sh
gcc -Wall -Wextra -g -fsanitize=address -o myshell main.c src/*.c
```

## Examples

```sh
ls -l | grep hello > out.txt
sort < in.txt >> sorted.txt
mkdir build && cd build || echo "error"
echo one ; echo two
```

## Project layout

```
.
├── main.c        # main loop: prompt, input, lexer, parser, executor
├── header/       # module headers
└── src/
    ├── lexer.c       # tokenization
    ├── parser.c      # AST construction
    ├── executor.c    # command, pipe and redirection execution
    ├── lineedit.c    # line input with history (raw mode)
    └── ...           # utilities and builtin hashmap
```

## Known limitations

- The cursor always stays at the end of the input line: left/right arrow keys are not handled.
- Line editing only supports ASCII characters.

## Possible improvements

- Background execution with `&`
- Environment variable expansion and globbing
- Explicit file descriptors in redirections (`2>`, `2>&1`)
- Here-documents (`<<`)