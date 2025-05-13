all:
	gcc ByteKnight.c init.c bitboards.c hashkeys.c board.c data.c attack.c io.c mvgen.c validate.c -o ByteKnight
