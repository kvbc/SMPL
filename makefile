GXX = gcc
OUT = smpl
SRC = $(wildcard *.c)

build:
	$(GXX) -o $(OUT) -O2 -Wno-int-to-pointer-cast $(SRC)