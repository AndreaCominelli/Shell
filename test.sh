#!/bin/bash
# upper.sh: legge righe da stdin, le stampa numerate e in maiuscolo

n=1
while IFS= read -r line; do
    echo "$n: $(echo "$line" | tr 'a-z' 'A-Z')"
    n=$((n + 1))
done