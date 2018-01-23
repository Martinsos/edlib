#include <stdio.h>
#include <sys/file.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define WORD long

#define SIGMA    128
#define BUF_MAX 2048

#include "parse.i"

static int W;

static unsigned WORD All   = -1;
static unsigned WORD Ebit;

static unsigned WORD *TRAN[SIGMA]; // TRAN could be TRANSLATION?
static unsigned WORD Pc[SIGMA];
static int seg, rem;

void setup_search()
{ register unsigned WORD *b, bvc, one;
  register int a, p, i, k;

  W = sizeof(unsigned WORD)*8;

  seg = (patlen-1)/W + 1;   // (segments) Number of blocks in column
  rem = seg*W - patlen;  // (reminder) Number of cells in padding


  b = (unsigned WORD *) malloc(sizeof(unsigned WORD)*(SIGMA*seg+1)); // Allocate SIGMA columns + 1 block
  for (a = 0; a < SIGMA; a++) // for each of SIGMA columns
    { TRAN[a] = b; // TRAN[a] is set to start of column
      for (p = 0; p < patlen; p += W) // p is index of block start cell
        { bvc = 0;
          one = 1;
          k = p+W; // k is index of block end cell
          if (patlen < k) k = patlen;
          for (i = p; i < k; i++)  // for each cell in block
            { if (patvec[i].type == CHAR)
                { if (a == *(patvec[i].value))
                    bvc |= one;
                }
              else
                { if (patvec[i].value[a>>3] & 1<<a%8)
                    bvc |= one;
                }
              one <<= 1;
            }
          k = p+W;
          while (i++ < k)
            { bvc |= one;
              one <<= 1;
            }
          *b++ = bvc;
        }
    }

  for (a = 0; a < SIGMA; a++)
    Pc[a] = TRAN[a][0];

  Ebit = (((long) 1) << (W-1));
  
}

// I do not really understand code above, but I believe it is initialization of query profile

typedef struct { unsigned WORD P;
                 unsigned WORD M;
                 int           V;  // This is score I guess
               } Scell;

void search(ifile,dif) int ifile, dif;  // dif is k
{ int num, i, base, diw, a, Cscore;
  Scell *s, *sd;
  unsigned WORD pc, mc;
  register unsigned WORD *e;
  register unsigned WORD P, M, U, X, Y;
  Scell *S, *SE;
  static char buf[BUF_MAX];

    printf("Started search function\n");

  S  = (Scell *) malloc(sizeof(Scell)*seg); // Allocate one column
  SE = S + (seg-1);  // Pointer to end of column

  diw = dif + W;  // diw = k + WORD_SIZE    What is diw?

  sd = S + (dif-1)/W;  // sd is last block of column
  for (s = S; s <= sd; s++) // Initialize first column
    { s->P = All;
      s->M =  0;
      s->V = ((s-S)+1)*W;
    }

  for (base = 1-rem; (num = read(ifile,buf,BUF_MAX)) > 0; base += num) // read num letters from target
    { i = 0;
      if (sd == S)  // if last block is first block
        { P = S->P;
          M = S->M;
          Cscore = S->V;
          for (; i < num; i++) // for letters of target that I read
            { a = buf[i];  //  letter

              U  = Pc[a]; // Eq for that letter -> how does it get correct for that part of query?
              X  = (((U & P) + P) ^ P) | U;
              U |= M;
    
              Y = P;
              P = M | ~ (X | Y);
              M = Y & X;

              if (P & Ebit)
                Cscore += 1;
              else if (M & Ebit)
                Cscore -= 1;

              Y = P << 1;
              P = (M << 1) | ~ (U | Y);
              M = Y & U;
              
              // Part above is similar like mine but with few lines less, hm why?
              
              if (Cscore <= dif)    // Why break if score < k?
                break;
            }
          S->P = P;
          S->M = M;
          S->V = Cscore;

          if (i >= num) continue;

          if (sd == SE) {
              //printf("  Match at %d\n",base+i);
              //printf("Score: %d\n", Cscore);
          }

          i += 1;
        }

      for (; i < num; i++) // for all characters that I read from target
        { e  = TRAN[buf[i]];

          pc = mc = 0;
          s  = S;
          while (s <= sd) // For all blocks in column
            { U  = *e++;
              P  = s->P;
              M  = s->M;

              Y  = U | mc;
              X  = (((Y & P) + P) ^ P) | Y;
              U |= M;

              Y = P;
              P = M | ~ (X | Y);
              M = Y & X;
    
              Y = (P << 1) | pc;
              s->P = (M << 1) | mc | ~ (U | Y);
              s->M = Y & U;
    
              U = s->V;
              pc = mc = 0;
              if (P & Ebit)
                { pc = 1; s->V = U+1; }
              else if (M & Ebit)
                { mc = 1; s->V = U-1; }

              s += 1;
            }

          if (U == dif && (*e & 0x1 | mc) && s <= SE) // U is score here!
            { // Prepare to calculate one more block in next turn
              s->P = All;
              s->M = 0;
              if (pc == 1)
                s->M = 0x1;
              if (mc != 1)
                s->P <<= 1;
              s->V = U = diw-1;
              sd   = s;
            }
          else
            { U = sd->V;
              while (U > diw)  // while there are blocks out of band, reduce band
                { U = (--sd)->V;
                }
            }
          if (sd == SE && U <= dif) {
              //printf("  Match at %d\n",base+i);
              //printf("Score: %d\n", U);
          }
        } // Here I finished with num characters I read from target

      while (sd > S) // This is executed each num columns
        { // While all cells of last block are > k (except last cell?), lastBlock--
          i = sd->V; 
          P = sd->P;
          M = sd->M;
          
          Y = Ebit;
          for (X = 0; X < W; X++)
            { if (P & Y)
                { i -= 1;
                  if (i <= dif) break;
                }
              else if (M & Y)
                i += 1;
              Y >>= 1;
            }
          if (i <= dif) break;
          sd -= 1;
        }
    }

  // From last column fetch results for last rem columns (because target is not padded)
  if (sd == SE)
    { P = sd->P;
      M = sd->M;
      U = sd->V;
      for (i = 0; i < rem; i++)
        { if (P & Ebit)
            U -= 1;
          else if (M & Ebit)
            U += 1;
          P <<= 1;
          M <<= 1;
          if (U <= dif) {
              //printf("  Match at %d\n",base+i);
              //printf("Score: %d\n", U);
          }
        }
    }
}

#include "main.i"
