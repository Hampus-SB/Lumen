torch: src/torch.c src/generation.h src/parser.h src/token.h
	mkdir -p build
	cc src/torch.c src/generation.h src/parser.h src/token.h -o build/torch

clean:
	rm build/a.asm
	rm build/a.o
	rm a.out
