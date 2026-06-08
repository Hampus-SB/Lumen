global _start

main:
	push rbp
	mov rbp, rsp


	mov rbx, qword 3
	mov rax, qword 2
	cmp rbx, rax
	jg L0


	mov rax, qword 1
	mov rsp, rbp
	pop rbp
	ret

	jmp L0

L0:

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
