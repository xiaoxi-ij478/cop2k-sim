# COP2000 Emulator Suite

This is a emulator of COP2000 by <http://www.wave-cn.cn>. It is made to ease learning COP2000 principle and faciliate debugging programs of COP2000.

It aims to support:

- define instruction set
- assemble/disassemble program
- explain what a program would do (signals & data I/O)
- emulate the full machine
  - switch signals
  - manipulate main memory and microprogram memory
  - watch status of buses in real-time
  - clock-step & instruction-step and watch results

It does NOT support:

- external device I/O

## Library

At the core is libcop2k, the bare computer.

## Programs

It also comes with several programs to use:

- CLI
  
  allow you to take complete control over the machine,
  including switch signals,
  run the machine full-speed, and loading binaries into memory

- Assembler
  
  Allow you to write programs in a certain instruction set and
  assemble it into a program suitable for load into memory

- VM
  
  This is a simplified version of CLI that just runs a binary program
  in full speed, and print out result if desired

- COP2000 Instruction Set Decompiler
  
  This is intended to use with original COP2000 DE. It decompiles an
  instruction description file of COP2000 DE and turn it into a description file
  suitable for use in this project.
