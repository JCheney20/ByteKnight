#include "defs.h"
#include "debug.h"
#include "polykeys.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  U64 key;
  unsigned short move;
  unsigned short weight;
  unsigned int learn;

} S_POLYBOOK_ENTRY;

long NumEntries = 0;

S_POLYBOOK_ENTRY *entries;

const int PolyKindofPiece[13] = {
  -1, 1, 3, 5, 7, 9, 11, 0, 2, 4, 6, 8, 10
};

void InitPolyBook(){
  EngineOpt->USE_BOOK = FALSE;
  FILE *pFile = fopen("./books/gm2600.bin", "rb");

  if (pFile == NULL) {
    printf("Book File Not Read\n");
  } else {
    fseek(pFile, 0, SEEK_END);
    long pos = ftell(pFile);

    if (pos < sizeof(S_POLYBOOK_ENTRY)) { printf("No Entries Found\n"); return;}

    NumEntries = pos / sizeof(S_POLYBOOK_ENTRY);
    printf("%ld Entries Found In File\n", NumEntries);

    entries = (S_POLYBOOK_ENTRY*)malloc(NumEntries * sizeof(S_POLYBOOK_ENTRY));
    rewind(pFile);

    size_t returnVal;
    returnVal = fread(entries, sizeof(S_POLYBOOK_ENTRY), NumEntries, pFile);
    printf("fread() %ld Entries Read in from file\n", returnVal);

    if(NumEntries > 0) EngineOpt->USE_BOOK = TRUE;
    
  }
}

void CleanPolyBook(){
  free(entries);
}

int hasPawnforCap(const S_BOARD *pos){
  int sqwPawn = 0;
  int t_pce = (pos->side == WHITE) ? wP : bP;
  if(pos->enPas != NO_SQ){
    sqwPawn = (pos->side == WHITE) ? pos->enPas - 10 : pos->enPas + 10;
    if (pos->pieces[sqwPawn + 1] == t_pce || pos->pieces[sqwPawn - 1] == t_pce) return TRUE; 
  }
  return FALSE;
}

U64 PolyKeyfrmBrd(const S_BOARD *pos){
  int sq = 0, rank = 0, file = 0;
  int piece = EMPTY, polyPce = 0, offset = 0;
  U64 finalKey = 0;

  for (sq=0; sq<BRD_SQ_NUM; ++sq) {
    piece = pos->pieces[sq];
    if (piece!=NO_SQ && piece!=EMPTY) {
      ASSERT(piece>=wP && piece<=bK);
      polyPce = PolyKindofPiece[piece];
      rank = RanksBrd[sq];
      file = FilesBrd[sq];
      finalKey ^= Random64_POLY[(64*polyPce) + (8*rank)+file];
    }
  }

  //castling
  offset = 768;
  if (pos->castlePerm & WKCA) finalKey ^= Random64_POLY[offset + 0];
  if (pos->castlePerm & WQCA) finalKey ^= Random64_POLY[offset + 1];
  if (pos->castlePerm & BKCA) finalKey ^= Random64_POLY[offset + 2];
  if (pos->castlePerm & BQCA) finalKey ^= Random64_POLY[offset + 3];


  //EP
  offset = 772;

  if (hasPawnforCap(pos) == TRUE) {
    file = FilesBrd[pos->enPas];
    finalKey ^= Random64_POLY[offset + file];
  }

  if (pos->side == WHITE) finalKey ^= Random64_POLY[780];

  return finalKey;
}

unsigned short endian_swap_u16(unsigned short x) 
{ 
    x = (x>>8) | 
        (x<<8); 
    return x;
} 

unsigned int endian_swap_u32(unsigned int x) 
{ 
    x = (x>>24) | 
        ((x<<8) & 0x00FF0000) | 
        ((x>>8) & 0x0000FF00) | 
        (x<<24); 
    return x;
} 

U64 endian_swap_u64(U64 x) 
{ 
    x = (x>>56) | 
        ((x<<40) & 0x00FF000000000000) | 
        ((x<<24) & 0x0000FF0000000000) | 
        ((x<<8)  & 0x000000FF00000000) | 
        ((x>>8)  & 0x00000000FF000000) | 
        ((x>>24) & 0x0000000000FF0000) | 
        ((x>>40) & 0x000000000000FF00) | 
        (x<<56); 
    return x;
}

int ConvertPolyMvtoInterMv(unsigned short polyMv, S_BOARD *pos){
  int ff = (polyMv >> 6) & 7;
  int fr = (polyMv >> 9) & 7;
  int tf = (polyMv >> 0) & 7;
  int tr = (polyMv >> 3) & 7;
  int pp = (polyMv >> 12) & 7;

  char mvStr[6];

  if (pp == 0) {
    sprintf(mvStr, "%c%c%c%c",
            FileChar[ff], RankChar[fr],
            FileChar[tf], RankChar[tr]);
  } else {
    char promChar = 'q';
    switch (pp) {
      case 1: promChar = 'q'; break;
      case 2: promChar = 'b'; break;
      case 3: promChar = 'r'; break;
    }
    sprintf(mvStr, "%c%c%c%c%c",
            FileChar[ff], RankChar[fr],
            FileChar[tf], RankChar[tr],
            promChar);
  }
  return ParseMv(mvStr, pos);
}

int getBookMv(S_BOARD *pos){
  int i = 0;
  S_POLYBOOK_ENTRY *entry;
  unsigned short move;
  const int MAXBOOKMOVES = 32;
  int bookMv[MAXBOOKMOVES];
  int t_Mv = NOMOVE;
  int count = 0;
  
  U64 polyKey = PolyKeyfrmBrd(pos);

  for (entry = entries; entry < entries + NumEntries; entry++) {
    if (polyKey == endian_swap_u64(entry->key)) {
      move = endian_swap_u16(entry->move);
      t_Mv = ConvertPolyMvtoInterMv(move, pos);
      if (t_Mv != NOMOVE) {
        bookMv[count++] = t_Mv;
        if (count > MAXBOOKMOVES) break;
      }
    }
  }
  if (count != 0) {
    int randMv = rand() % count;
    return bookMv[randMv];
  } else {
    return NOMOVE;
  }
}


