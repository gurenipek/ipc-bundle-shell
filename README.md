# bshell — bundle shell

a custom Unix shell where instead of running single commands, you define named
process bundles: groups of commands that all run concurrently and share
the same input. bundles can be chained into pipelines, and I/O can be
redirected to/from files.

## how it works

- define a bundle with `pbc` / `pbs`, give it a name
- when executed, every command in the bundle is forked and runs in parallel
- a Repeater fans out the same input to every process via pipes
- bundles can be pipelined: `bundle1 | bundle2 | bundle3`
- first bundle can read from a file (`<`), last can write to one (`>`)
- all children are reaped before the shell accepts new input — no zombies

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
---
*CENG 334 — Operating Systems, METU, Spring 2022*