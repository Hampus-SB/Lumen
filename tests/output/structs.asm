global _start

foo:
	push rbp
	mov rbp, rsp

	sub rsp, 8  ; a
	mov [rbp - 8], rax
	mov rax, [rbp - 8]
	sub rax, 8
	mov [rax], qword 5

	mov rsp, rbp
	pop rbp
	ret

main:
	push rbp
	mov rbp, rsp

	sub rsp, 24  ; s
	lea rax, [rbp - 8]
	call foo

	mov rax, [rbp - 16]
	mov rsp, rbp
	pop rbp
	ret

	mov rsp, rbp
	pop rbp
	ret

_start:
	mov rbp, rsp

	call main

	mov rdi, rax
	mov rax, 60
	syscall
