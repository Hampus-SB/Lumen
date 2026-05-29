global _start

foo:
	push rbp
	mov rbp, rsp


	mov rax, qword 10

	mov rsp, rbp
	pop rbp
	ret

	mov rsp, rbp
	pop rbp
	ret

bar:
	push rbp
	mov rbp, rsp

	sub rsp, 8  ; x
	mov [rbp - 8], rax
	sub rsp, 8  ; y
	mov [rbp - 16], rbx
	sub rsp, 8  ; z
	mov rax, [rbp - 8]
	mov rbx, [rbp - 16]
	add eax, ebx
	mov [rbp - 24], rax

	mov rax, [rbp - 24]

	mov rsp, rbp
	pop rbp
	ret

	mov rsp, rbp
	pop rbp
	ret

roof:
	push rbp
	mov rbp, rsp

	sub rsp, 8  ; q
	mov [rbp - 8], qword 5
	mov [rbp - 8], qword 10

	mov rsp, rbp
	pop rbp
	ret

main:
	push rbp
	mov rbp, rsp

	sub rsp, 8  ; a
	call foo
	mov [rbp - 8], rax
	sub rsp, 8  ; b
	mov rax, qword 1
	mov rbx, qword 2
	call bar
	mov [rbp - 16], rax
	call roof

	mov rsp, rbp
	pop rbp
	ret

_start:
	mov rbp, rsp

	call main

	mov rdi, rax
	mov rax, 60
	syscall
