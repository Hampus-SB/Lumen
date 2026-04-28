torch: src/torch.c src/generation.h src/parser.h src/token.h src/arena.h
	mkdir -p build
	cc src/torch.c -o build/torch

ember: src/ember/lexer.h src/ember/lexer.c src/ember/main.c
	mkdir -p build
	cc src/ember/lexer.c src/ember/main.c -o build/ember

clean:
	rm -rf build/

all: torch
	echo "hello world!"
