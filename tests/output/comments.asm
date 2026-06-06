global _start

main:
	push rbp
	mov rbp, rsp

	sub rsp, 8  ; x
	mov [rbp - 8], qword 5
	sub rsp, 8  ; y
	mov [rbp - 16], qword 10

	mov rax, qword 0
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
