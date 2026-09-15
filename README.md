# Tab5 Terminal App (`tab5-app-terminal`)

Aplicativo isolado de terminal interativo para o **Tab5 OS**.

## Compilação e Empacotamento

```bash
chmod +x tools/build.sh
./tools/build.sh
```

O pacote `.tab5pkg` será gerado em `dist/com.tab5.terminal.tab5pkg`.

O terminal oferece os comandos `help`, `ls`, `free`, `df`, `date` e `clear`
por meio da entrada de comandos no textarea e da confirmação com Enter.
