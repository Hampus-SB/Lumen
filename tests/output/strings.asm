global _start

mem_alloc:
	push rbp
	mov rbp, rsp

	sub rsp, 8  ; __size_bytes
	mov [rbp - 8], rax
	mov rdi, 0
	mov rsi, [rbp - 8]
	mov rdx, 0x1
	or rdx, 0x2
	mov r10, 0x2
	or r10, 0x20
	mov r8, -1
	mov r9, 0
	mov rax, 9
	syscall

	mov rsp, rbp
	pop rbp
	ret

string_init:
	push rbp
	mov rbp, rsp

	sub rsp, 8  ; s2
	mov [rbp - 8], rax
	mov rax, qword 4096
	call mem_alloc
	mov rbx, [rbp - 8]
	sub rbx, 0
	mov [rbx], rax
	mov rax, [rbp - 8]
	sub rax, 8
	mov [rax], qword 0
	mov rax, [rbp - 8]
	sub rax, 16
	mov [rax], qword 4096

	mov rsp, rbp
	pop rbp
	ret

string_print:
	push rbp
	mov rbp, rsp

	sub rsp, 8  ; s1
	mov [rbp - 8], rax
	mov rsi, [rax]
	sub rax, qword 8
	mov rdx, [rax]
	mov rax, 1
	mov rdi, rax
	syscall

	mov rsp, rbp
	pop rbp
	ret

main:
	push rbp
	mov rbp, rsp

	sub rsp, 24  ; s
	lea rax, [rbp - 8]
	call string_init
	mov rax, qword 0
	mov rbx, qword 65
	imul rax, 1
	add rax, [rbp - 8]
	mov [rax], qword rbx
	mov rax, qword 1
	mov rbx, qword 66
	imul rax, 1
	add rax, [rbp - 8]
	mov [rax], qword rbx
	mov rax, qword 2
	mov rbx, qword 67
	imul rax, 1
	add rax, [rbp - 8]
	mov [rax], qword rbx
	mov rax, qword 3
	mov rbx, qword 11
	imul rax, 1
	add rax, [rbp - 8]
	mov [rax], qword rbx
	mov [rbp - 16], qword 4
	lea rax, [rbp - 8]
	call string_print

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
