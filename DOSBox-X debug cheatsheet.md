
DOSBox-X Debugger Cheat Sheet
(for Watcom / protected-mode DOS programs)

--- Enter / Leave Debugger ---
Ctrl+F5            break into debugger
c                  continue execution
s                  step into
n                  step over

--- Registers ---
r                  show all registers
r eax              show specific register
r cr3              paging context (if used)

Watch especially:
- EIP (where you are)
- ESP (stack corruption shows fast)
- CS/DS/SS selectors if things go sideways

--- Breakpoints (Protected Mode = linear addresses) ---
info cpu           show current CPU state (copy EIP)
bp eip             break at current instruction
bp 0x12345678      break at linear address
bd <n>             disable breakpoint n
bc <n>             clear breakpoint n

--- Disassembly ---
u eip              disassemble at current instruction
u 0x12345678       disassemble at address

--- Memory ---
d 0x12345678       dump memory at linear address
d esp              dump stack
d ss:esp           dump stack with segment

--- Interrupt Breakpoints (very useful) ---
bp int31           DPMI calls (Watcom extender)
bp int21           DOS calls

--- Logging ---
log start          start debugger logging
log stop           stop logging

--- Practical Notes ---
- Watcom PM apps usually run flat 32-bit under DPMI
- Breakpoints are linear, not CS:IP
- MAP files help resolve symbols manually
- Debug the machine, not the language