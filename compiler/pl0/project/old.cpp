/*** PL0 COMPILER WITH CODE GENERATION ***/
//---------------------------------------------------------------------------
// hbc (2014-10-24): removed form related headers.
// hbc (2014-10-24): add necessary headers.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sstream>

// hbc (2014-10-24): replace String with string
using std::string;
typedef string String;

// hbc (2014-10-24): handcarfted itoa.
string IntToStr(int n) {
    std::stringstream ss;
    ss << n;
    return ss.str();
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// hbc (2014-10-24): replace constant with #define macros.
#define AL      10  /* LENGTH OF IDENTIFIERS */
#define NORW    19  /* # OF RESERVED WORDS */
#define TXMAX  100  /* LENGTH OF IDENTIFIER TABLE */
#define NMAX    14  /* MAX NUMBER OF DEGITS IN NUMBERS */
#define AMAX  2047  /* MAXIMUM ADDRESS */
#define LEVMAX   3  /* MAX DEPTH OF BLOCK NESTING */
#define CXMAX  200  /* SIZE OF CODE ARRAY */

typedef enum  { NUL, IDENT, NUMBER, PLUS, MINUS, TIMES,
	            SLASH, ODDSYM, EQL, NEQ, LSS, LEQ, GTR, GEQ,
	            LPAREN, RPAREN, COMMA, SEMICOLON, PERIOD,
	            BECOMES,
                    ADD_ASSIGN, SUB_ASSIGN,
                    MUL_ASSIGN, DIV_ASSIGN,
                    BEGINSYM, ENDSYM, IFSYM, THENSYM,
	            WHILESYM, WRITESYM, READSYM, DOSYM, CALLSYM,
	            CONSTSYM, VARSYM, PROCSYM, PROGSYM,
                    ELSESYM, FORSYM, STEPSYM, UNTILSYM, RETURNSYM
        } SYMBOL;
char *SYMOUT[] = {"NUL", "IDENT", "NUMBER", "PLUS", "MINUS", "TIMES",
	    "SLASH", "ODDSYM", "EQL", "NEQ", "LSS", "LEQ", "GTR", "GEQ",
	    "LPAREN", "RPAREN", "COMMA", "SEMICOLON", "PERIOD",
	    "BECOMES",
            "ADD_ASSIGN", "SUB_ASSIGN", "MUL_ASSIGN", "DIV_ASSIGN",
            "BEGINSYM", "ENDSYM", "IFSYM", "THENSYM",
	    "WHILESYM", "WRITESYM", "READSYM", "DOSYM", "CALLSYM",
	    "CONSTSYM", "VARSYM", "PROCSYM", "PROGSYM",
            "ELSESYM", "FORSYM", "STEPSYM", "UNTILSYM", "RETURNSYM" };
// hbc (2014-10-24): make symbols count a macro.
#define SYMBOLS_COUNT (sizeof(SYMOUT) / sizeof(SYMOUT[0]))
typedef  int *SYMSET; // SET OF SYMBOL;
typedef  char ALFA[11];
typedef  enum { CONSTANT, VARIABLE, PROCEDUR } OBJECTS ;
typedef  enum { LIT, OPR, LOD, STO, CAL, INI, JMP, JPC } FCT;
typedef struct {
	 FCT F;     /*FUNCTION CODE*/
	 int L;     /*0..LEVMAX  LEVEL*/
	 int A;     /*0..AMAX    DISPLACEMENT ADDR*/
} INSTRUCTION;
	  /* LIT O A -- LOAD CONSTANT A             */
	  /* OPR 0 A -- EXECUTE OPR A               */
	  /* LOD L A -- LOAD VARIABLE L,A           */
	  /* STO L A -- STORE VARIABLE L,A          */
	  /* CAL L A -- CALL PROCEDURE A AT LEVEL L */
	  /* INI 0 A -- INCREMET T-REGISTER BY A    */
	  /* JMP 0 A -- JUMP TO A                   */
	  /* JPC 0 A -- JUMP CONDITIONAL TO A       */
char   CH;  /*LAST CHAR READ*/
SYMBOL SYM; /*LAST SYMBOL READ*/
ALFA   ID;  /*LAST IDENTIFIER READ*/
int    NUM; /*LAST NUMBER READ*/
int    CC;  /*CHARACTER COUNT*/
int    LL;  /*LINE LENGTH*/
int    CX;  /*CODE ALLOCATION INDEX*/
char   LINE[81];                /* current line buffer */
INSTRUCTION  CODE[CXMAX];       /* code stack */
ALFA    KWORD[NORW+1];          /* reserved words (?) */
SYMBOL  WSYM[NORW+1];           /* keywords symbol table */
SYMBOL  SSYM['^'+1];            /* single character symbol table */
ALFA    MNEMONIC[9];            /* instructions representing string (?) */
SYMSET  DECLBEGSYS, STATBEGSYS, FACBEGSYS; /* !! symbol set (?) */

struct {
  ALFA NAME;
  OBJECTS KIND;
  union {
    int VAL;   /*CONSTANT*/
    struct { int LEVEL,ADR,SIZE; } vp;  /*VARIABLE,PROCEDUR:*/
  };
} TABLE[TXMAX];                 /* symbol table */

FILE *FIN,*FOUT;
int ERR;

void EXPRESSION(SYMSET FSYS, int LEV, int &TX);
void TERM(SYMSET FSYS, int LEV, int &TX);
//---------------------------------------------------------------------------
int SymIn(SYMBOL SYM, SYMSET S1) {
  return S1[SYM];
}
//---------------------------------------------------------------------------
SYMSET SymSetUnion(SYMSET S1, SYMSET S2) {
  SYMSET S=(SYMSET)malloc(sizeof(int)* SYMBOLS_COUNT);
  for (int i=0; i< SYMBOLS_COUNT; ++i)
	if (S1[i] || S2[i]) S[i]=1;
	else S[i]=0;
  return S;
}
//---------------------------------------------------------------------------
SYMSET SymSetAdd(SYMBOL SY, SYMSET S) {
  SYMSET S1;
  S1=(SYMSET)malloc(sizeof(int)*SYMBOLS_COUNT);
  for (int i=0; i<SYMBOLS_COUNT; ++i) S1[i]=S[i];
  S1[SY]=1;
  return S1;
}
//---------------------------------------------------------------------------
SYMSET SymSetNew(SYMBOL a) {
  SYMSET S; int i,k;
  S=(SYMSET)malloc(sizeof(int)*SYMBOLS_COUNT);
  for (i=0; i<SYMBOLS_COUNT; ++i) S[i]=0;
  S[a]=1;
  return S;
}
//---------------------------------------------------------------------------
SYMSET SymSetNew(SYMBOL a, SYMBOL b) {
  SYMSET S; int i,k;
  S=(SYMSET)malloc(sizeof(int)*SYMBOLS_COUNT);
  for (i=0; i<SYMBOLS_COUNT; ++i) S[i]=0;
  S[a]=1;  S[b]=1;
  return S;
}
//---------------------------------------------------------------------------
SYMSET SymSetNew(SYMBOL a, SYMBOL b, SYMBOL c) {
  SYMSET S; int i,k;
  S=(SYMSET)malloc(sizeof(int)*SYMBOLS_COUNT);
  for (i=0; i<SYMBOLS_COUNT; ++i) S[i]=0;
  S[a]=1;  S[b]=1; S[c]=1;
  return S;
}
//---------------------------------------------------------------------------
SYMSET SymSetNew(SYMBOL a, SYMBOL b, SYMBOL c, SYMBOL d) {
  SYMSET S; int i,k;
  S=(SYMSET)malloc(sizeof(int)*SYMBOLS_COUNT);
  for (i=0; i<SYMBOLS_COUNT; ++i) S[i]=0;
  S[a]=1;  S[b]=1; S[c]=1; S[d]=1;
  return S;
}
//---------------------------------------------------------------------------
SYMSET SymSetNew(SYMBOL a, SYMBOL b, SYMBOL c, SYMBOL d,SYMBOL e) {
  SYMSET S; int i,k;
  S=(SYMSET)malloc(sizeof(int)*SYMBOLS_COUNT);
  for (i=0; i<SYMBOLS_COUNT; ++i) S[i]=0;
  S[a]=1;  S[b]=1; S[c]=1; S[d]=1; S[e]=1;
  return S;
}
//---------------------------------------------------------------------------
SYMSET SymSetNew(SYMBOL a, SYMBOL b, SYMBOL c, SYMBOL d,SYMBOL e, SYMBOL f) {
  SYMSET S; int i,k;
  S=(SYMSET)malloc(sizeof(int)*SYMBOLS_COUNT);
  for (i=0; i<SYMBOLS_COUNT; ++i) S[i]=0;
  S[a]=1;  S[b]=1; S[c]=1; S[d]=1; S[e]=1; S[f]=1;
  return S;
}
//---------------------------------------------------------------------------
SYMSET SymSetNULL() {
  SYMSET S; int i,n,k;
  S=(SYMSET)malloc(sizeof(int)*SYMBOLS_COUNT);
  for (i=0; i<SYMBOLS_COUNT; ++i) S[i]=0;
  return S;
}
//---------------------------------------------------------------------------
// hbc (2014-10-24): clean c++ builder stuffs.
void Error(int n) {
  //string s = "***"+AnsiString::StringOfChar(' ', CC-1)+"^";
  //Form1->printls(s.c_str(),n);
  fprintf(FOUT,"%d\n", n);
  ERR++;
} /*Error*/
//---------------------------------------------------------------------------
void GetCh() {
  // Read new line buffer.
  if (CC==LL) {
    if (feof(FIN)) {
          // hbc (2014-10-24): clean c++ builder stuffs.
	  // Form1->printfs("PROGRAM INCOMPLETE");
	  fprintf(FOUT,"PROGRAM INCOMPLETE\n");
	  fclose(FOUT);
	  exit(0);
	}
	LL=0; CC=0;
	CH=' ';
	while (!feof(FIN) && CH!=10)
      { CH=fgetc(FIN);  LINE[LL++]=CH; }
	LINE[LL-1]=' ';  LINE[LL]=0;
    // ? what CX used for? where does it change?
    String s=IntToStr(CX);
    while(s.length()<3) s=" "+s;
    s=s+" "+LINE;
    // hbc (2014-10-24): cleanup c++ builder stuffs.
    // Form1->printfs(s.c_str());
    fprintf(FOUT,"%s\n",s.c_str());
  }
  CH=LINE[CC++];
} /*GetCh()*/
//---------------------------------------------------------------------------
void GetSym() {
  int i,J,K;   ALFA  A;
  while (CH<=' ') GetCh();
  if (CH>='A' && CH<='Z') { /*ID OR RESERVED WORD*/
    K=0;
	do {
	  if (K<AL) A[K++]=CH;
	  GetCh();
	}while((CH>='A' && CH<='Z')||(CH>='0' && CH<='9'));
	A[K]='\0';
	strcpy(ID,A); i=1; J=NORW;
        // Check if the indent is a keyword.
	do {
	  K=(i+J) / 2;
	  if (strcmp(ID,KWORD[K])<=0) J=K-1;
	  if (strcmp(ID,KWORD[K])>=0) i=K+1;
	}while(i<=J);
	if (i-1 > J) SYM=WSYM[K];
	else SYM=IDENT;
  }
  else
    if (CH>='0' && CH<='9') { /*NUMBER*/
      K=0; NUM=0; SYM=NUMBER;
	  do {
	    NUM=10*NUM+(CH-'0');
		K++; GetCh();
      }while(CH>='0' && CH<='9');
	  if (K>NMAX) Error(30);
    }
    else
      if (CH==':') {
	    GetCh();
		if (CH=='=') { SYM=BECOMES; GetCh(); }
		else SYM=NUL;
      }
    else
      if (CH == '+') {
            GetCh();
            // Self-assignment.
            if (CH == '=') {
                GetCh();
                SYM = ADD_ASSIGN;
            } else {
                SYM = SSYM['+'];
            }
      }
    else
      if (CH == '-') {
            GetCh();
            // Self-assignment.
            if (CH == '=') {
                GetCh();
                SYM = SUB_ASSIGN;
            } else {
                SYM = SSYM['-'];
            }
      }
    else
      if (CH == '*') {
            GetCh();
            // Self-assignment.
            if (CH == '=') {
                GetCh();
                SYM = MUL_ASSIGN;
            } else {
                SYM = SSYM['*'];
            }
      }
    else
      if (CH == '/') {
            GetCh();   
            // In comment.
            if (CH == '*') {
                GetCh();
                while (1) {
                    if (CH == '*') {
                        GetCh();
                        // End of comment.
                        if (CH == '/') {
                            GetCh();
                            break;
                        }
                    }

                    GetCh();
                }
                // Restart tokenizing.
                GetSym();
            // Self-assignment.
            } else if (CH == '=') {
                GetCh();
                SYM = DIV_ASSIGN;
            } else {
                SYM = SSYM['/'];
            }
      }
	  else /* THE FOLLOWING TWO CHECK WERE ADDED
	         BECAUSE ASCII DOES NOT HAVE A SINGLE CHARACTER FOR <= OR >= */
	    if (CH=='<') {
		  GetCh();
		  if (CH=='=') { SYM=LEQ; GetCh(); }
		  else if (CH == '>') { SYM=NEQ; GetCh(); }
                  else { SYM = LSS; }
		}
		else
		  if (CH=='>') {
		    GetCh();
			if (CH=='=') { SYM=GEQ; GetCh(); }
			else SYM=GTR;
          }
                  // Get single character symbol.
		  else { SYM=SSYM[CH]; GetCh(); }
} /*GetSym()*/
//---------------------------------------------------------------------------
void GEN(FCT X, int Y, int Z) {
  if (CX>CXMAX) {
    // hbc (2014-10-24): cleanup c++ builder stuffs.
    // Form1->printfs("PROGRAM TOO LONG");
    fprintf(FOUT,"PROGRAM TOO LONG\n");
    fclose(FOUT);
    exit(0);
  }
  CODE[CX].F=X; CODE[CX].L=Y; CODE[CX].A=Z;
  CX++;
} /*GEN*/
//---------------------------------------------------------------------------
void TEST(SYMSET S1, SYMSET S2, int N) {
  if (!SymIn(SYM,S1)) {
    Error(N);
	while (!SymIn(SYM,SymSetUnion(S1,S2))) GetSym();
  }
} /*TEST*/
//---------------------------------------------------------------------------
void ENTER(OBJECTS K, int LEV, int &TX, int &DX) { /*ENTER OBJECT INTO TABLE*/
  TX++;
  strcpy(TABLE[TX].NAME,ID); TABLE[TX].KIND=K;
  switch (K) {
	case CONSTANT:
	       if (NUM>AMAX) { Error(31); NUM=0; }
	       TABLE[TX].VAL=NUM;
	       break;
    case VARIABLE:
	       TABLE[TX].vp.LEVEL=LEV; TABLE[TX].vp.ADR=DX; DX++;
	       break;
	case PROCEDUR:
	       TABLE[TX].vp.LEVEL=LEV;
	       break;
  }
} /*ENTER*/
//---------------------------------------------------------------------------
int POSITION(ALFA ID, int TX) { /*FIND IDENTIFIER IN TABLE*/
  int i=TX;
  strcpy(TABLE[0].NAME,ID);
  while (strcmp(TABLE[i].NAME,ID)!=0) i--;
  return i;
} /*POSITION*/
//---------------------------------------------------------------------------
void ConstDeclaration(int LEV,int &TX,int &DX) {
  if (SYM==IDENT) {
    GetSym();
    if (SYM==EQL||SYM==BECOMES) {
	  if (SYM==BECOMES) Error(1);
	  GetSym();
	  if (SYM==NUMBER) { ENTER(CONSTANT,LEV,TX,DX); GetSym(); }
	  else Error(2);
    }
    else Error(3);
  }
  else Error(4);
} /*ConstDeclaration()*/
//---------------------------------------------------------------------------
void VarDeclaration(int LEV,int &TX,int &DX) {
  if (SYM==IDENT) { ENTER(VARIABLE,LEV,TX,DX); GetSym(); }
  else Error(4);
} /*VarDeclaration()*/
//---------------------------------------------------------------------------
void ListCode(int CX0) {  /*LIST CODE GENERATED FOR THIS Block*/
  // hbc (2014-10-24): cleanup c++ builder stuffs.
  // just simply turn off code listing now.
  //if (Form1->ListSwitch->ItemIndex==0)
  if (false)
    for (int i=CX0; i<CX; ++i) {
      String s=IntToStr(i);
      // hbc (2014-10-24): cleanup c++ builder stuffs.
      while(s.length()<3)s=" "+s;
      s=s+" "+MNEMONIC[CODE[i].F]+" "+IntToStr(CODE[i].L)+" "+IntToStr(CODE[i].A);
          // hbc (2014-10-24): cleanup c++ builder stuffs.
	  // Form1->printfs(s.c_str());
	  fprintf(FOUT,"%3d%5s%4d%4d\n",i,MNEMONIC[CODE[i].F],CODE[i].L,CODE[i].A);
    }
} /*ListCode()*/;
//---------------------------------------------------------------------------
void FACTOR(SYMSET FSYS, int LEV, int &TX) {
  int i;
  TEST(FACBEGSYS,FSYS,24);
  while (SymIn(SYM,FACBEGSYS)) {
	if (SYM==IDENT) {
	  i=POSITION(ID,TX);
	  if (i==0) Error(11);
	  else
		switch (TABLE[i].KIND) {
		  case CONSTANT: GEN(LIT,0,TABLE[i].VAL); break;
		  case VARIABLE: GEN(LOD,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR); break;
		  case PROCEDUR: Error(21); break;
		}
	  GetSym();
	}
	else
	  if (SYM==NUMBER) {
		if (NUM>AMAX) { Error(31); NUM=0; }
		GEN(LIT,0,NUM); GetSym();
	  }
	  else
		if (SYM==LPAREN) {
		  GetSym(); EXPRESSION(SymSetAdd(RPAREN,FSYS),LEV,TX);
		  if (SYM==RPAREN) GetSym();
		  else Error(22);
		}
	  TEST(FSYS,FACBEGSYS,23);
  }
}/*FACTOR*/
//---------------------------------------------------------------------------
void TERM(SYMSET FSYS, int LEV, int &TX) {  /*TERM*/
  SYMBOL MULOP;
  FACTOR(SymSetUnion(FSYS,SymSetNew(TIMES,SLASH)), LEV,TX);
  while (SYM==TIMES || SYM==SLASH) {
	MULOP=SYM;  GetSym();
	FACTOR(SymSetUnion(FSYS,SymSetNew(TIMES,SLASH)),LEV,TX);
	if (MULOP==TIMES) GEN(OPR,0,4);
	else GEN(OPR,0,5);
  }
} /*TERM*/;
//---------------------------------------------------------------------------
void EXPRESSION(SYMSET FSYS, int LEV, int &TX) {
  SYMBOL ADDOP;
  if (SYM==PLUS || SYM==MINUS) {
    ADDOP=SYM; GetSym();
    TERM(SymSetUnion(FSYS,SymSetNew(PLUS,MINUS)),LEV,TX);
    if (ADDOP==MINUS) GEN(OPR,0,1);
  }
  else TERM(SymSetUnion(FSYS,SymSetNew(PLUS,MINUS)),LEV,TX);
  while (SYM==PLUS || SYM==MINUS) {
    ADDOP=SYM; GetSym();
    TERM(SymSetUnion(FSYS,SymSetNew(PLUS,MINUS)),LEV,TX);
    if (ADDOP==PLUS) GEN(OPR,0,2);
    else GEN(OPR,0,3);
  }
} /*EXPRESSION*/
//---------------------------------------------------------------------------
void CONDITION(SYMSET FSYS,int LEV,int &TX) {
  SYMBOL RELOP;
  if (SYM==ODDSYM) { GetSym(); EXPRESSION(FSYS,LEV,TX); GEN(OPR,0,6); }
  else {
	EXPRESSION(SymSetUnion(SymSetNew(EQL,NEQ,LSS,LEQ,GTR,GEQ),FSYS),LEV,TX);
	if (!SymIn(SYM,SymSetNew(EQL,NEQ,LSS,LEQ,GTR,GEQ))) Error(20);
	else {
	  RELOP=SYM; GetSym(); EXPRESSION(FSYS,LEV,TX);
	  switch (RELOP) {
	    case EQL: GEN(OPR,0,8);  break;
	    case NEQ: GEN(OPR,0,9);  break;
	    case LSS: GEN(OPR,0,10); break;
	    case GEQ: GEN(OPR,0,11); break;
	    case GTR: GEN(OPR,0,12); break;
	    case LEQ: GEN(OPR,0,13); break;
	  }
	}
  }
} /*CONDITION*/
//---------------------------------------------------------------------------
// Assignment operations
void ASSIGNMENT(SYMSET FSYS, int LEV, int &TX, int var_pos) {
    EXPRESSION(FSYS, LEV, TX);

    GEN(STO, LEV - TABLE[var_pos].vp.LEVEL, TABLE[var_pos].vp.ADR);
}

void SELF_ASSIGNMENT(SYMSET FSYS, int LEV, int &TX, int var_pos, SYMBOL op) {
    int opr_type;

    // Load variable.
    GEN(LOD, LEV - TABLE[var_pos].vp.LEVEL, TABLE[var_pos].vp.ADR);

    // Calculate right hand value.
    EXPRESSION(FSYS, LEV, TX);

    // Calculate new value.
    switch (op) {
        case ADD_ASSIGN:
            opr_type = 2;
            break;
        case SUB_ASSIGN:
            opr_type = 3;
            break;
        case MUL_ASSIGN:
            opr_type = 4;
            break;
        case DIV_ASSIGN:
            opr_type = 5;
            break;
    }
    GEN(OPR, 0, opr_type);
    
    // Store new value.
    GEN(STO, LEV - TABLE[var_pos].vp.LEVEL, TABLE[var_pos].vp.ADR);
}
//---------------------------------------------------------------------------
void STATEMENT(SYMSET FSYS,int LEV,int &TX) {   /*STATEMENT*/
  int i,CX1,CX2;
  switch (SYM) {
	case IDENT:
		i=POSITION(ID,TX);
		if (i==0) Error(11);
		else
		  if (TABLE[i].KIND!=VARIABLE) { /*ASSIGNMENT TO NON-VARIABLE*/
			Error(12); i=0;
		  }
                GetSym();
		if (SYM==BECOMES) {
                    GetSym();
                    ASSIGNMENT(FSYS, LEV, TX, i);
                } else if (SYM == ADD_ASSIGN
                           || SYM == SUB_ASSIGN
                           || SYM == MUL_ASSIGN
                           || SYM == DIV_ASSIGN) {
                    SYMBOL op_sym = SYM;
                    GetSym();
                    SELF_ASSIGNMENT(FSYS, LEV, TX, i, op_sym);
                }
                else {
                    Error(13);
                }
		break;
	case READSYM:
		GetSym();
		if (SYM!=LPAREN) Error(34);
		else
		  do {
			GetSym();
			if (SYM==IDENT) i=POSITION(ID,TX);
			else i=0;
			if (i==0) Error(35);
			else {
			  GEN(OPR,0,16);
			  GEN(STO,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
			}
			GetSym();
		  }while(SYM==COMMA);
		if (SYM!=RPAREN) {
		  Error(33);
		  while (!SymIn(SYM,FSYS)) GetSym();
		}
		else GetSym();
		break; /* READSYM */
	case WRITESYM:
		GetSym();
		if (SYM==LPAREN) {
		  do {
			GetSym();
			EXPRESSION(SymSetUnion(SymSetNew(RPAREN,COMMA),FSYS),LEV,TX);
			GEN(OPR,0,14);
		  }while(SYM==COMMA);
		  if (SYM!=RPAREN) Error(33);
		  else GetSym();
		}
		GEN(OPR,0,15);
		break; /*WRITESYM*/
	case CALLSYM:
		GetSym();
		if (SYM!=IDENT) Error(14);
		else {
		  i=POSITION(ID,TX);
		  if (i==0) Error(11);
		  else
			if (TABLE[i].KIND==PROCEDUR)
			  GEN(CAL,LEV-TABLE[i].vp.LEVEL,TABLE[i].vp.ADR);
			else Error(15);
		  GetSym();
		}
		break;
	case IFSYM:
		GetSym();
		CONDITION(SymSetUnion(SymSetNew(THENSYM,DOSYM),FSYS),LEV,TX);
		if (SYM==THENSYM) GetSym();
		else Error(16);
		CX1=CX;  GEN(JPC,0,0);
		STATEMENT(FSYS,LEV,TX);  CODE[CX1].A=CX;
		break;
	case BEGINSYM:
		GetSym();
		STATEMENT(SymSetUnion(SymSetNew(SEMICOLON,ENDSYM),FSYS),LEV,TX);
		while (SymIn(SYM, SymSetAdd(SEMICOLON,STATBEGSYS))) {
		  if (SYM==SEMICOLON) GetSym();
		  else Error(10);
		  STATEMENT(SymSetUnion(SymSetNew(SEMICOLON,ENDSYM),FSYS),LEV,TX);
		}
		if (SYM==ENDSYM) GetSym();
		else Error(17);
		break;
	case WHILESYM:
		CX1=CX; GetSym(); CONDITION(SymSetAdd(DOSYM,FSYS),LEV,TX);
		CX2=CX; GEN(JPC,0,0);
		if (SYM==DOSYM) GetSym();
		else Error(18);
		STATEMENT(FSYS,LEV,TX);
		GEN(JMP,0,CX1);
		CODE[CX2].A=CX;
		break;
  }
  TEST(FSYS,SymSetNULL(),19);
} /*STATEMENT*/
//---------------------------------------------------------------------------
void Block(int LEV, int TX, SYMSET FSYS) {
  int DX=3;    /*DATA ALLOCATION INDEX*/
  int TX0=TX;  /*INITIAL TABLE INDEX*/
  int CX0=CX;  /*INITIAL CODE INDEX*/
  TABLE[TX].vp.ADR=CX; GEN(JMP,0,0);
  if (LEV>LEVMAX) Error(32);
  do {
    if (SYM==CONSTSYM) {
      GetSym();
      do {
        ConstDeclaration(LEV,TX,DX);
        while (SYM==COMMA) {
          GetSym();  ConstDeclaration(LEV,TX,DX);
        }
        if (SYM==SEMICOLON) GetSym();
        else Error(5);
      }while(SYM==IDENT);
    }
    if (SYM==VARSYM) {
      GetSym();
      do {
        VarDeclaration(LEV,TX,DX);
        while (SYM==COMMA) { GetSym(); VarDeclaration(LEV,TX,DX); }
	    if (SYM==SEMICOLON) GetSym();
	    else Error(5);
      }while(SYM==IDENT);
    }
    while ( SYM==PROCSYM) {
      GetSym();
	  if (SYM==IDENT) { ENTER(PROCEDUR,LEV,TX,DX); GetSym(); }
	  else Error(4);
	  if (SYM==SEMICOLON) GetSym();
	  else Error(5);
	  Block(LEV+1,TX,SymSetAdd(SEMICOLON,FSYS));
	  if (SYM==SEMICOLON) {
	    GetSym();
	    TEST(SymSetUnion(SymSetNew(IDENT,PROCSYM),STATBEGSYS),FSYS,6);
	  }
	  else Error(5);
    }
    TEST(SymSetAdd(IDENT,STATBEGSYS), DECLBEGSYS,7);
  }while(SymIn(SYM,DECLBEGSYS));
  CODE[TABLE[TX0].vp.ADR].A=CX;
  TABLE[TX0].vp.ADR=CX;   /*START ADDR OF CODE*/
  TABLE[TX0].vp.SIZE=DX;  /*SIZE OF DATA SEGMENT*/
  GEN(INI,0,DX);
  STATEMENT(SymSetUnion(SymSetNew(SEMICOLON,ENDSYM),FSYS),LEV,TX);
  GEN(OPR,0,0);  /*RETURN*/
  TEST(FSYS,SymSetNULL(),8);
  ListCode(CX0);
} /*Block*/
//---------------------------------------------------------------------------
int BASE(int L,int B,int S[]) {
  int B1=B; /*FIND BASE L LEVELS DOWN*/
  while (L>0) { B1=S[B1]; L=L-1; }
  return B1;
} /*BASE*/
//---------------------------------------------------------------------------
void Interpret() {
  // hbc (2014-10-24): add explict type for int constant.
  const int STACKSIZE = 500;
  int P,B,T; 		/*PROGRAM BASE TOPSTACK REGISTERS*/
  INSTRUCTION I;
  int S[STACKSIZE];  	/*DATASTORE*/
  // hbc (2014-10-24): clean up c++ builder stuffs.
  // Form1->printfs("~~~ RUN PL0 ~~~");
  fprintf(FOUT,"~~~ RUN PL0 ~~~\n");
  T=0; B=1; P=0;
  S[1]=0; S[2]=0; S[3]=0;
  do {
    I=CODE[P]; P=P+1;
    switch (I.F) {
      case LIT: T++; S[T]=I.A; break;
	  case OPR:
	    switch (I.A) { /*OPERATOR*/
	      case 0: /*RETURN*/ T=B-1; P=S[T+3]; B=S[T+2]; break;
	      case 1: S[T]=-S[T];  break;
	      case 2: T--; S[T]=S[T]+S[T+1];   break;
	      case 3: T--; S[T]=S[T]-S[T+1];   break;
	      case 4: T--; S[T]=S[T]*S[T+1];   break;
	      case 5: T--; S[T]=S[T]/S[T+1]; break;
	      case 6: S[T]=(S[T]%2!=0);        break;
	      case 8: T--; S[T]=S[T]==S[T+1];  break;
	      case 9: T--; S[T]=S[T]!=S[T+1];  break;
	      case 10: T--; S[T]=S[T]<S[T+1];   break;
	      case 11: T--; S[T]=S[T]>=S[T+1];  break;
	      case 12: T--; S[T]=S[T]>S[T+1];   break;
	      case 13: T--; S[T]=S[T]<=S[T+1];  break;
	      case 14:
                   // hbc (2014-10-24): clean up c++ builder stuffs.
                   // Form1->printls("",S[T]);
                   fprintf(FOUT,"%d\n",S[T]); T--;
                   break;
	      case 15: /*Form1->printfs("");*/ fprintf(FOUT,"\n"); break;
	      case 16:
                   T++;
                   // hbc (2014-10-24): clean up c++ builder stuffs.
                   // S[T]=InputBox("����","��������룺", 0).ToInt();
                   // Form1->printls("? ",S[T]);
                   scanf("%d", &S[T]);
                   fprintf(FOUT,"? %d\n",S[T]);
                   break;
	    }
	    break;
      case LOD: T++; S[T]=S[BASE(I.L,B,S)+I.A]; break;
      case STO: S[BASE(I.L,B,S)+I.A]=S[T]; T--; break;
	  case CAL: /*GENERAT NEW Block MARK*/
	      S[T+1]=BASE(I.L,B,S); S[T+2]=B; S[T+3]=P;
	      B=T+1; P=I.A; break;
	  case INI: T=T+I.A;  break;
	  case JMP: P=I.A; break;
      case JPC: if (S[T]==0) P=I.A;  T--;  break;
    } /*switch*/
  }while(P!=0);
  // hbc (2014-10-24): clean up c++ builder stuffs.
  // Form1->printfs("~~~ END PL0 ~~~");
  fprintf(FOUT,"~~~ END PL0 ~~~\n");
} /*Interpret*/
//---------------------------------------------------------------------------
/* hbc (2014-10-24): clean up c++ builder stuffs. */
// void __fastcall TForm1::ButtonRunClick(TObject *Sender) {
// hbc (2014-10-24): add main function for cli usage.
int main(int argc, char *argv[]) {
  for (CH=' '; CH<='^'; CH++) SSYM[CH]=NUL;

  int i;
  // Prepare keywords. (human sort ~_~)
  i = 0;
  strcpy(KWORD[++i],"BEGIN");
  strcpy(KWORD[++i],"CALL");
  strcpy(KWORD[++i],"CONST");
  strcpy(KWORD[++i],"DO");
  strcpy(KWORD[++i],"ELSE");
  strcpy(KWORD[++i],"END");
  strcpy(KWORD[++i],"FOR");
  strcpy(KWORD[++i],"IF");
  strcpy(KWORD[++i],"ODD");
  strcpy(KWORD[++i],"PROCEDURE");
  strcpy(KWORD[++i],"PROGRAM");
  strcpy(KWORD[++i],"READ");
  strcpy(KWORD[++i],"RETURN");
  strcpy(KWORD[++i],"STEP");
  strcpy(KWORD[++i],"THEN");
  strcpy(KWORD[++i],"UNTIL");
  strcpy(KWORD[++i],"VAR");
  strcpy(KWORD[++i],"WHILE");
  strcpy(KWORD[++i],"WRITE");

  i = 0;
  WSYM[++i]=BEGINSYM;
  WSYM[++i]=CALLSYM;
  WSYM[++i]=CONSTSYM;
  WSYM[++i]=DOSYM;
  WSYM[++i]=ELSESYM;
  WSYM[++i]=ENDSYM;
  WSYM[++i]=FORSYM;
  WSYM[++i]=IFSYM;
  WSYM[++i]=ODDSYM;
  WSYM[++i]=PROCSYM;
  WSYM[++i]=PROGSYM;
  WSYM[++i]=READSYM;
  WSYM[++i]=RETURNSYM;
  WSYM[++i]=STEPSYM;
  WSYM[++i]=THENSYM;
  WSYM[++i]=UNTILSYM;
  WSYM[++i]=VARSYM;
  WSYM[++i]=WHILESYM;
  WSYM[++i]=WRITESYM;

  SSYM['+']=PLUS;      SSYM['-']=MINUS;
  SSYM['*']=TIMES;     SSYM['/']=SLASH;
  SSYM['(']=LPAREN;    SSYM[')']=RPAREN;
  SSYM['=']=EQL;       SSYM[',']=COMMA;
  SSYM['.']=PERIOD;    SSYM[';']=SEMICOLON;
  strcpy(MNEMONIC[LIT],"LIT");   strcpy(MNEMONIC[OPR],"OPR");
  strcpy(MNEMONIC[LOD],"LOD");   strcpy(MNEMONIC[STO],"STO");
  strcpy(MNEMONIC[CAL],"CAL");   strcpy(MNEMONIC[INI],"INI");
  strcpy(MNEMONIC[JMP],"JMP");   strcpy(MNEMONIC[JPC],"JPC");

  DECLBEGSYS=(int*)malloc(sizeof(int)*SYMBOLS_COUNT);
  STATBEGSYS=(int*)malloc(sizeof(int)*SYMBOLS_COUNT);
  FACBEGSYS =(int*)malloc(sizeof(int)*SYMBOLS_COUNT);
  for(int j=0; j<SYMBOLS_COUNT; j++) {
	DECLBEGSYS[j]=0;  STATBEGSYS[j]=0;  FACBEGSYS[j] =0;
  }
  DECLBEGSYS[CONSTSYM]=1;
  DECLBEGSYS[VARSYM]=1;
  DECLBEGSYS[PROCSYM]=1;
  STATBEGSYS[BEGINSYM]=1;
  STATBEGSYS[CALLSYM]=1;
  STATBEGSYS[IFSYM]=1;
  STATBEGSYS[WHILESYM]=1;
  STATBEGSYS[WRITESYM]=1;
  FACBEGSYS[IDENT] =1;
  FACBEGSYS[NUMBER]=1;
  FACBEGSYS[LPAREN]=1;

  // hbc (2014-10-24): mock file io into cli args.
  if (argc < 2) {
    fprintf(stderr, "./main input.pl0");
    return 1;
  }
  if ((FIN=fopen(argv[1], "r"))!=0) {
    FOUT= stderr;
    // hbc (2014-10-24): clean up c++ builder stuffs.
    // Form1->printfs("=== COMPILE PL0 ===");
    fprintf(FOUT,"=== COMPILE PL0 ===\n");
	ERR=0;
	CC=0; CX=0; LL=0; CH=' '; GetSym();
	if (SYM!=PROGSYM) Error(0);
	else {
	  GetSym();
	  if (SYM!=IDENT) Error(0);
	  else {
		GetSym();
		if (SYM!=SEMICOLON) Error(5);
		else GetSym();
	  }
	}
	Block(0,0,SymSetAdd(PERIOD,SymSetUnion(DECLBEGSYS,STATBEGSYS)));
	if (SYM!=PERIOD) Error(9);
	if (ERR==0) Interpret();
	else {
          // hbc (2014-10-24): clean up c++ builder stuffs.
	  // Form1->printfs("ERROR IN PL/0 PROGRAM");
	  fprintf(FOUT,"ERROR IN PL/0 PROGRAM");
	}
	fprintf(FOUT,"\n");
        // hbc (2014-10-24): mock file io.
        // fclose(FOUT);
  }
}
//---------------------------------------------------------------------------
