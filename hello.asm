.intel_syntax noprefix
.global _start

.section .data
msg: .ascii "Hello, world!\n"
len = . - msg

.section .text
_start:
    mov rax, 1
    mov rdi, 1
    lea rsi, msg
    mov rdx, len
    syscall

    mov rax, 60
    mov rdi, 0
    syscall
