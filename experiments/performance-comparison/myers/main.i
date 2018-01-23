static char *progname;

void error(msg,arg) char *msg, *arg;
{ fprintf(stderr,"%s: ",progname);
  fprintf(stderr,msg,arg);
  fprintf(stderr,"\n");
  exit(1);
}

#define SIG  70

void   srand48();
double drand48();

void main(argc,argv) int argc; char *argv[];
{ int   ifile;
  int   dif;
  char *pat;
  int   i;
  int   len, alpha;
  static char *cset = 
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()";

  progname = argv[0];

  if (argc < 2) error("Usage is '%s <pat> [ dif ] [ file ]'",progname);

  if (isdigit(argv[1][0]))
    { if (sscanf(argv[1],"%d,%d",&len,&alpha) != 2)
        error("pattern spec should be of the form #l,#a",0);
      if (len < 0 || alpha < 0)
        error("only positive arguments in pattern spec.",0);
      if (alpha > SIG)
        error("alphabet has more than %d symbols",SIG);
      pat = (char *) malloc(len+1);
      srand48(SIG*len + alpha);
      pat[len] = '\0';
      while (len-- > 0)
        pat[len] = cset[(int) (drand48()*alpha)];
    }
  else
    pat = argv[1];

  if (argc >= 3)
    { dif = atoi(argv[2]);
      if (dif < 0) error("Threshold, %d, is negative\n",dif);
    }
  else
    dif = 0;

  if (argc >= 4)
    { ifile = open(argv[3],O_RDONLY);
      if (ifile == -1) error("Can't open file %s",argv[3]);
    }
  else
    ifile = 1;

  encode_pattern(pat);
  if (dif > patlen)
    error("Threshold greater than pattern length",0);

#ifdef SHOW
  printf("Pat = '%s'(%d) dif=%d\n",pat,patlen,dif);
  show_pat();
#endif

#ifdef STATS
  setup_search();
  search(ifile,dif);

#else
  setup_search();
  if (argc >= 4)
    for (i = 0; i < 1; i++) //  I (Martin) changed 10 to 1 here
      { search(ifile,dif);
        close(ifile);
        ifile = open(argv[3],O_RDONLY);
      }
#endif

  exit (0);
}
