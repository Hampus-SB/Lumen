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

	sub rsp, 24  ; s1
	sub rsp, 8  ; s2
	lea rax, [rbp - 32]
	mov [rbp - 56], rax
	mov [rbp - 32], qword 1
	mov [rbp - 40], qword 2
	mov [rbp - 48], qword 3
	mov rax, [rbp - 56]
	sub rax, 0
	mov [rax], qword 4
	mov rax, [rbp - 56]
	sub rax, 8
	mov [rax], qword 5
	mov rax, [rbp - 56]
	sub rax, 16
	mov [rax], qword 6

	mov rsp, rbp
	pop rbp
	ret

_start:
	mov rbp, rsp

	call main

	mov rdi, rax
	mov rax, 60
	syscall
