torch: src/torch.c src/token.h
	mkdir -p build
	cc src/torch.c src/token.h -o build/torch
