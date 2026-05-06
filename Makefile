torch: src/torch/torch.c src/torch/generation.h src/torch/parser.h src/torch/token.h src/torch/arena.h
	mkdir -p build
	cc src/torch/torch.c -o build/torch
	./build/torch x.lu

ember: src/ember/lexer.h src/ember/lexer.c src/ember/main.c
	mkdir -p build
	cc src/ember/lexer.c src/ember/main.c -o build/ember

clean:
	rm -rf build/

all: torch
	echo "hello world!"
