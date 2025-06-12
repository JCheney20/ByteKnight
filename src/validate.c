#include "defs.h"
#include "debug.h"

int MoveListOk(const S_MOVELIST *list,  const S_BOARD *pos) {
	if(list->count < 0 || list->count >= MAXPOSITIONMOVES) {
		return FALSE;
	}

	int MoveNum;
	int from = 0;
	int to = 0;
	for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {
		to = TOSQ(list->moves[MoveNum].mv);
		from = FROMSQ(list->moves[MoveNum].mv);
		if(!SqOnBoard(to) || !SqOnBoard(from)) {
			return FALSE;
		}
		if(!PieceValid(pos->pieces[from])) {
			PrintBoard(pos);
			return FALSE;
		}
	}

	return TRUE;
}

int SqIs120(const int sq) {
	return (sq>=0 && sq<120);
}

int PceValidEmptyOffbrd(const int pce) {
	return (PieceValidEmpty(pce) || pce == NO_SQ);
}

int SqOnBoard(const int sq){
  return FilesBrd[sq]==NO_SQ ? 0:1;
}

int SideValid(const int side){
  return (side == WHITE || side == BLACK) ? 1: 0;
}

int FileRankValid(const int fr){
  return (fr >= 0 || fr <= 7) ? 1: 0;
}

int PieceValidEmpty(const int pce){
  return (pce >= EMPTY || pce <= bK) ? 1: 0;
}

int PieceValid(const int pce){
  return (pce >= wP || pce <= bK) ? 1: 0;
}
