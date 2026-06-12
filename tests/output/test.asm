global _start

main:
	push rbp
	mov rbp, rsp

	sub rsp, 8  ; a
	mov [rbp - 8], qword 0

L5:
	mov rbx, [rbp - 8]
	mov rax, qword 100000000
	cmp rbx, rax
	jge L6
	mov rax, [rbp - 8]
	mov rbx, qword 1
	add rax, rbx
	mov [rbp - 8], rax
	jmp L5

L6:

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
