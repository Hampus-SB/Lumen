torch: src/torch/torch.c src/torch/tests.c src/torch/generation.c src/torch/parser.c src/torch/type_checker.c src/torch/tokenizer.c src/torch/types.c src/torch/symbols.c src/torch/imports.c
	mkdir -p build
	cc src/torch/torch.c src/torch/tests.c src/torch/generation.c src/torch/parser.c src/torch/type_checker.c src/torch/tokenizer.c src/torch/types.c src/torch/symbols.c src/torch/imports.c -o build/torch
	./build/torch x.lu

ember: src/ember/lexer.h src/ember/lexer.c src/ember/main.c
	mkdir -p build
	cc src/ember/lexer.c src/ember/main.c -o build/ember

clean:
	rm -rf build/

all: torch
	echo "hello world!"
