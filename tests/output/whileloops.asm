global _start

main:
	push rbp
	mov rbp, rsp

	sub rsp, 8  ; a
	mov [rbp - 8], qword 0
	sub rsp, 8  ; b
	mov [rbp - 16], qword 15

L1:
	mov rbx, [rbp - 8]
	mov rax, [rbp - 16]
	cmp rbx, rax
	jge L2
	mov rax, [rbp - 8]
	mov rbx, qword 2
	add rax, rbx
	mov [rbp - 8], rax

L3:
	mov rbx, qword 1
	mov rax, qword 1
	cmp rbx, rax
	jne L4
	mov rax, [rbp - 16]
	mov rbx, qword 1
	add rax, rbx
	mov [rbp - 16], rax
	jmp L4
	jmp L3

L4:
	jmp L1

L2:

	mov rax, [rbp - 8]
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
