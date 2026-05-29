global _start

main:
	push rbp
	mov rbp, rsp

	sub rsp, 8  ; a
	mov [rbp - 8], qword 15
	sub rsp, 8  ; b
	lea rax, [rbp - 8]
	mov [rbp - 16], rax
	sub rsp, 8  ; c
	mov rcx, [rbp - 16]
	mov rdx, [rcx]
	mov [rbp - 24], rdx
	mov [rbp - 16], rax
	mov [rax], qword 5
	mov rcx, [rbp - 16]
	mov rdx, [rcx]
	mov [rbp - 24], rdx

	mov rsp, rbp
	pop rbp
	ret

_start:
	mov rbp, rsp

	call main

	mov rdi, rax
	mov rax, 60
	syscall
