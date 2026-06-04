# bshell — bundle shell

A custom Unix shell where instead of running single commands, you define named
process bundles — groups of commands that all run concurrently and share
the same input. Bundles can be chained into pipelines, and I/O can be
redirected to/from files.

Built for CENG 334 (Intro to Operating Systems) at METU.

## how it works

- Define a bundle with `pbc` / `pbs`, give it a name
- When executed, every command in the bundle is forked and runs in parallel
- A Repeater fans out the same input to every process via pipes
- Bundles can be piped into each other: `bundle1 | bundle2 | bundle3`
- First bundle can read from a file (`<`), last can write to one (`>`)
- All children are reaped before the shell accepts new input — no zombies

## build & run

```bash
make
./hw1
```
## example input

```bash
pbc upper
tr /a-z/ /A-Z/
pbs

pbc counter
wc -l
pbs

upper | counter < input.txt > result.txt

quit
```
