all:
	gcc Just-in-Check.c init.c bitboards.c hashkeys.c board.c data.c attack.c io.c mvgen.c makemv.c perft.c validate.c search.c misc.c pvtable.c eval.c uci.c xboard.c -o Just-in-Check
