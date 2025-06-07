#include "stdio.h"
#include "defs.h"
#include "debug.h"
#include "string.h"

int threeFoldRep(const S_BOARD *pos){
  ASSERT(CheckBrd(pos));

  int i = 0, r = 0;
  for (i = 0; i < pos->histPly; ++i) {
    if (pos->history[i].posKey == pos->posKey) r++;
  }
  return r;
}

int DrawMaterial(const S_BOARD *pos){
  ASSERT(CheckBrd(pos));

  if (pos->pceNum[wP] || pos->pceNum[bP]) return FALSE;
  if (pos->pceNum[wQ] || pos->pceNum[bQ] || pos->pceNum[wR] || pos->pceNum[bR]) return FALSE;
  if (pos->pceNum[wB] > 1 || pos->pceNum[bB] > 1) return FALSE;
  if (pos->pceNum[wN] > 1 || pos->pceNum[bN] > 1) return FALSE;
  if (pos->pceNum[wN] && pos->pceNum[wB] ) return FALSE;
  if (pos->pceNum[bN] && pos->pceNum[bB] ) return FALSE;

  return TRUE;
}

int checkresult(S_BOARD *pos){
  ASSERT(CheckBrd(pos));
  if (pos->fiftyMv > 100) {
    printf(" 1/2 - 1/2 {fifty move rule [claimed by %s]}\n", NAME); return TRUE;
  }
  
  if (threeFoldRep(pos)>= 2) {
    printf(" 1/2 - 1/2 {3-fold repetition [claimed by %s]}\n",NAME); return TRUE;
  }
  
  if (DrawMaterial(pos) == TRUE) {
    printf(" 1/2 - 1/2 {Insufficient mating material [claimed by %s]}\n", NAME); return TRUE;
  }

  S_MOVELIST list[1];
  GenerateAllMvs(pos, list);
  
  int MvNum = 0, found = 0;

  for (MvNum = 0; MvNum < list->count; ++MvNum) {
    if (!makeMv(pos, list->moves[MvNum].mv)) continue; 
    found++;
    takeMv(pos);
    break;
  }

  if (found != 0 ) return FALSE;

  int InCheck = SqAttacked(pos->KingSq[pos->side], pos->side^1, pos);


  if (InCheck == TRUE) {
    if (pos->side == WHITE) {
      printf(" 0 - 1 {black mates [claimed by %s]}\n",NAME);return TRUE;
    } else {
      printf(" 0 - 1 {white mates [claimed by %s]}\n",NAME);return TRUE;
    }
  } else {
    printf("\n 1/2 - 1/2 {stalemate [claimed by %s]}\n", NAME); return TRUE;
  }

  return FALSE;
}

void PrintOpt(){
      printf("feature ping=1 setboard=1 colors=0 usermove=1\n");
      printf("feature done=1\n");
}

void xBoard_Loop(S_BOARD *pos, S_HASHTABLE *table, S_SEARCHINFO *info){
  info->GAME_MODE = XBOARDMODE;
	info->POST_THINKING = TRUE;
	setbuf(stdin, NULL);
  setbuf(stdout, NULL);
  PrintOpt(); // HACK

	int depth = -1, movestogo[2] = {30,30 }, movetime = -1;
	int time = -1, inc = 0;
	int engineSide = BLACK;
	int timeLeft;
	int sec;
	int mps;
	int move = NOMOVE;
	char inBuf[80], command[80];
	int MB;

	ParseFen(START_FEN, pos);

	while(TRUE) {

		fflush(stdout);

		if(pos->side == engineSide && checkresult(pos) == FALSE) {
			info->start_time = GetTimeMS();
			info->depth = depth;

			if(time != -1) {
				info->t_set = TRUE;
				time /= movestogo[pos->side];
				time -= 50;
				info->stop_time = info->start_time + time + inc;
			}

			if(depth == -1 || depth > MAXDEPTH) {
				info->depth = MAXDEPTH;
			}

			printf("time:%d start:%d stop:%d depth:%d timeset:%d movestogo:%d mps:%d\n",
				time,info->start_time,info->stop_time,info->depth,info->t_set, movestogo[pos->side], mps);
				SearchPosition(pos, table, info);

			if(mps != 0) {
				movestogo[pos->side^1]--;
				if(movestogo[pos->side^1] < 1) {
					movestogo[pos->side^1] = mps;
				}
			}
		}

		fflush(stdout);

		memset(&inBuf[0], 0, sizeof(inBuf));
		fflush(stdout);

		if (!fgets(inBuf, 80, stdin)) continue;

		sscanf(inBuf, "%s", command);

		printf("command seen:%s\n",inBuf);

		if(!strcmp(command, "quit")) {
			info->quit = TRUE;
			break;
		}

		if(!strcmp(command, "force")) {
			engineSide = BOTH;
			continue;
		}

		if(!strcmp(command, "protover")){
			PrintOpt();
		    continue;
		}

		if(!strcmp(command, "sd")) {
			sscanf(inBuf, "sd %d", &depth);
		    printf("DEBUG depth:%d\n",depth);
			continue;
		}

		if(!strcmp(command, "st")) {
			sscanf(inBuf, "st %d", &movetime);
		    printf("DEBUG movetime:%d\n",movetime);
			continue;
		}

		if(!strcmp(command, "time")) {
			sscanf(inBuf, "time %d", &time);
			time *= 10;
		    printf("DEBUG time:%d\n",time);
			continue;
		}

		if(!strcmp(command, "memory")) {			
			sscanf(inBuf, "memory %d", &MB);		
		    if(MB < 4) MB = 4;
			if(MB > MAXHASH) MB = MAXHASH;
			printf("Set Hash to %d MB\n",MB);
			InitHashTable(table, MB);
			continue;
		}


		if(!strcmp(command, "level")) {
			sec = 0;
			movetime = -1;
			if( sscanf(inBuf, "level %d %d %d", &mps, &timeLeft, &inc) != 3) {
			  sscanf(inBuf, "level %d %d:%d %d", &mps, &timeLeft, &sec, &inc);
		      printf("DEBUG level with :\n");
			}	else {
		      printf("DEBUG level without :\n");
			}
			timeLeft *= 60000;
			timeLeft += sec * 1000;
			movestogo[0] = movestogo[1] = 30;
			if(mps != 0) movestogo[0] = movestogo[1] = mps;
			time = -1;
		    printf("DEBUG level timeLeft:%d movesToGo:%d inc:%d mps%d\n",timeLeft,movestogo[0],inc,mps);
			continue;
		}

		if(!strcmp(command, "ping")) {
			printf("pong%s\n", inBuf+4);
			continue;
		}

		if(!strcmp(command, "new")) {
			ClearHashTable(table);
			engineSide = BLACK;
			ParseFen(START_FEN, pos);
			depth = -1;
			time = -1;
			continue;
		}

    if(!strcmp(command, "polykey")){
      PrintBoard(pos);
      getBookMv(pos);
    }

		if(!strcmp(command, "setboard")){
			engineSide = BOTH;
			ParseFen(inBuf+9, pos);
			continue;
		}

		if(!strcmp(command, "go")) {
			engineSide = pos->side;
			continue;
		}

		if(!strcmp(command, "usermove")){
			movestogo[pos->side]--;
			move = ParseMv(inBuf+9, pos);
			if(move == NOMOVE) continue;
			makeMv(pos, move);
            pos->ply=0;
		}
    }
}

void Console_Loop(S_BOARD *pos, S_HASHTABLE *table, S_SEARCHINFO *info){
  printf("Welcome to %s in Console Mode!\n", NAME);
  printf("Type help for commands\n\n");

  info->GAME_MODE = CONSOLEMODE;
  info->POST_THINKING = TRUE;
  setbuf(stdin, NULL);
  setbuf(stdout, NULL);

  int depth = MAXDEPTH, movetime = 3000;
  int engineSide = BLACK;
  int move = NOMOVE;
  char inBuf[80], cmd[80];
  ParseFen(START_FEN, pos);

  while (TRUE) {
    fflush(stdout);

    if (pos->side == engineSide && checkresult(pos) == FALSE) {
      info->start_time = GetTimeMS();
      info->depth = depth;

      if (movetime != 0) {
        info->t_set = TRUE;
        info->stop_time = info->start_time + movetime;
      }

      SearchPosition(pos, table, info);
    }

    printf("\n%s ::>>", NAME);

    fflush(stdout);

    memset(&inBuf[0], 0, sizeof(inBuf));
    fflush(stdout);
    if (!fgets(inBuf, 80, stdin)) continue;

    sscanf(inBuf, "%s", cmd);

    if(!strcmp(cmd, "help")) {
			printf("Commands:\n");
			printf("quit - quit game\n");
			printf("force - computer will not think\n");
			printf("print - show board\n");
			printf("post - show thinking\n");
			printf("nopost - do not show thinking\n");
			printf("new - start new game\n");
			printf("go - set computer thinking\n");
			printf("depth x - set depth to x\n");
      printf("eval - evaluate the current position\n");
			printf("time x - set thinking time to x seconds (depth still applies if set)\n");
			printf("view - show current depth and movetime settings\n");
			printf("setboard x - set position to fen x\n");
			printf("** note ** - to reset time and depth, set to 0\n");
			printf("enter moves using b7b8q notation\n\n\n");
			continue;
		}

    if (!strcmp(cmd, "eval")) {
      PrintBoard(pos);
      printf("Eval: %d\n",evalPos(pos));
      MirrorBrd(pos);
      PrintBoard(pos);
      printf("Eval: %d\n",evalPos(pos));
      continue;
    }

		if(!strcmp(cmd, "setboard")){
			engineSide = BOTH;
			ParseFen(inBuf+9, pos);
			continue;
		}

		if(!strcmp(cmd, "quit")) {
			info->quit = TRUE;
			break;
		}

		if(!strcmp(cmd, "post")) {
			info->POST_THINKING = TRUE;
			continue;
		}

		if(!strcmp(cmd, "print")) {
			PrintBoard(pos);
			continue;
		}

		if(!strcmp(cmd, "nopost")) {
			info->POST_THINKING = FALSE;
			continue;
		}

		if(!strcmp(cmd, "force")) {
			engineSide = BOTH;
			continue;
		}

		if(!strcmp(cmd, "view")) {
			if(depth == MAXDEPTH) printf("depth not set ");
			else printf("depth %d",depth);

			if(movetime != 0) printf(" movetime %ds\n",movetime/1000);
			else printf(" movetime not set\n");

			continue;
		}

		if(!strcmp(cmd, "depth")) {
			sscanf(inBuf, "depth %d", &depth);
		  if(depth==0) depth = MAXDEPTH;
			continue;
		}

		if(!strcmp(cmd, "time")) {
			sscanf(inBuf, "time %d", &movetime);
			movetime *= 1000;
			continue;
		}

		if(!strcmp(cmd, "new")) {
			ClearHashTable(table);
			engineSide = BLACK;
			ParseFen(START_FEN, pos);
			continue;
		}

		if(!strcmp(cmd, "go")) {
			engineSide = pos->side;
			continue;
		}

		move = ParseMv(inBuf, pos);
		if(move == NOMOVE) {
			printf("Command unknown:%s\n",inBuf);
			continue;
		}

		makeMv(pos, move);
		pos->ply=0;
  }
}
