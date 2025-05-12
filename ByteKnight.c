#include "defs.h"
#include "debug.h"
#include "stdio.h"
#include "stdlib.h"

#define FEN1 "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1" 
#define FEN2 "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2"
#define FEN3 "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2"
#define FEN4 "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"
#define FEN5 "8/3q1p2/8/5P2/4Q3/8/8/8 w KQkq - 0 1"

void showSqAtBySide(const int side, const S_BOARD *pos){

  int rank, file, sq = 0;

  printf("\n\nSquares attacked by:%c\n",SideChar[side]);
  for (rank = RANK_8; rank >= RANK_1; --rank) {
    for (file = FILE_A; file <= FILE_H; ++file) {
      sq = FR2SQ(file, rank);
      if(SqAttacked(sq, side, pos) == TRUE){
        printf("X");
      } else {
        printf("-");
      }
    }
    CR;
  }
  CR;CR;
}

int main(){
  AllInit();
  int i;
  S_BOARD board[1];

  // ParseFen(START_FEN, board);
  // PrintBoard(board);
  //
  // ParseFen(FEN1, board);
  // PrintBoard(board);
  //
  // ParseFen(FEN2, board);
  // PrintBoard(board);
  //
  // ParseFen(FEN3, board);
  // PrintBoard(board);

  ParseFen(FEN5, board);
  PrintBoard(board);

  printf("\n\nWhite Attacking:\n");
  showSqAtBySide(WHITE, board);

  // ASSERT(CheckBrd(board));

  printf("\n\n Black Attacking:\n");
  showSqAtBySide(BLACK, board);



  return 0;
}
