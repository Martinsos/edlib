#define CHAR  0
#define CLASS 1

typedef struct { char type, *value; } element;

static int      patlen;
static element *patvec;

int scan1(pat) register char *pat;
{ register int vlen;
  void error();

  patlen = vlen = 0;
  while (*pat != '\0')
    switch (*pat)
    { case '.':
        pat += 1;
        patlen += 1;
        break;
      case '[':
        if (*++pat == '^')
          pat += 1;
        while (*pat != ']')
          { if (*pat == '\\') pat += 1;
            if (*pat++ == '\0') error("prematurely terminated class");
            if (*pat == '-')
              { if (*++pat == '\\') pat += 1;
                if (*pat++ == '\0') error("prematurely terminated class");
              }
          }
        pat += 1;
        patlen += 1;
        vlen += 16;
        break;
      case '\\':
        if (*++pat == '\0') error("\\ at end of pattern");
      default:
        pat += 1;
        patlen += 1;
        vlen += 1;
        break;
    }
  return (vlen);
}

void scan2(pat,vlen) register char *pat; int vlen;
{ register int c, comp;
  register char *vpt;
  void error();

  static char dot[16] = { '\377', '\377', '\377', '\377',
                          '\377', '\377', '\377', '\377',
                          '\377', '\377', '\377', '\377',
                          '\377', '\377', '\377', '\377' };

  vpt = (char *) malloc(vlen);
  patvec = (element *) malloc(sizeof(element)*patlen);

  patlen = 0;
  while (*pat != '\0')
    switch (*pat)
    { case '.':
        pat += 1;
        patvec[patlen].type = CLASS;
        patvec[patlen].value = dot;
        patlen += 1;
        break;
      case '[':
        for (c = 0; c <= 15; c++)
          vpt[c] = 0;
        comp = 0;
        if (*++pat == '^')
          { pat += 1;
            comp = 1;
          }
        while (*pat != ']')
          { c = *pat++;
            if (c == '\\') c = *pat++;
            if (*pat == '-')
              { if (*++pat == '\\') pat += 1;
                vpt[c>>3] |= 1 << c%8;
                while (c != *pat)
                  { if (c > *pat)
                      c -= 1;
                    else
                      c += 1;
                    vpt[c>>3] |= 1 << c%8;
                  }
                pat += 1;
              }
            else
              vpt[c>>3] |= 1 << c%8;
          }
        if (comp)
          for (c = 0; c <= 15; c++)
            vpt[c] ^= 0377;
        pat += 1;
        patvec[patlen].type = CLASS;
        patvec[patlen].value = vpt;
        patlen += 1;
        vpt += 16;
        break;
      case '\\':
        if (*++pat == '\0') error("\\ at end of pattern",0);
      default:
        patvec[patlen].type = CHAR;
        patvec[patlen].value = vpt;
        *vpt++ = *pat++;
        patlen += 1;
        break;
    }
}

#ifdef SHOW
static char *slab[] = { "CHAR ", "CLASS " };

void show_pat()
{ int i, j, k;
  char c;

  for (k = 0; k < patlen; k++)
    { printf("  %3d: %s",k,slab[patvec[k].type]);
      if (patvec[k].type == CHAR)
        printf(" %c\n",*(patvec[k].value));
      else
        { for (i = 0; i <= 15; i++)
            { c = patvec[k].value[i];
              if (i%4 == 0 && i != 0)
                printf("              ");
              else
                putchar(' ');
              for (j = 0; j <= 7; j++)
                if (c & 1<<j) putchar('1'); else putchar('0');
              if (i%4 == 3) putchar('\n');
            }
        }
    }
}
#endif

void encode_pattern(pat) char *pat;
{ int vlen;
  vlen = scan1(pat);
  scan2(pat,vlen);
} 
