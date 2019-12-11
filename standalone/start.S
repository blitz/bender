        // Set stack and jump to _main

        .extern main, __exit
        .global _mbheader, _start, jmp_multiboot

        .section .text._start, "ax"
        .align  4, 0
_mbheader:
        .long 0x1BADB002                // magic
        .long 0x3                       // features (page aligned modules, mem info)
        .long -(0x1BADB002 + 0x3)       // checksum

_start:
        mov     $_stack_end, %esp
        mov     %ebx, %edx
        push    $__exit
        jmp     main

jmp_multiboot:
        xchg    %ebx, %eax
        mov     $0x2BADB002, %eax
        jmp     *%edx

        .section .bss
        .align 4, 0
_stack:
        .skip 4096
_stack_end:
