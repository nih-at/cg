/* -*- fundamental -*- */
%{
    #include "ranges.h"
    #include <string.h>
    static char *group;
    extern struct range *rcmap;
    extern char *prg;
    static long lower, upper, no_art, max, min;
%}

%s GRUPPE

%%
[-A-Za-z.!_+=0-9]+:   {
    yytext[strlen(yytext)-1]='\0';
    if (strcasecmp(yytext, group) == 0)
	BEGIN(GRUPPE);
}
<GRUPPE>1-[0-9]+       {
    if (sscanf(yytext, "%ld-%ld", &lower, &upper) != 2) {
	fprintf(stderr, "%s: error lexing\n", prg);
	exit(1);
    }
    rcmap=range_init((upper < min) ? min : upper, max, no_art);
}

<GRUPPE>[0-9]+-[0-9]+  {
    if (sscanf(yytext, "%ld-%ld", &lower, &upper) != 2) {
	fprintf(stderr, "%s: error lexing\n", prg);
	exit(1);
    }
    range_fill(rcmap, lower, upper, 1);
}

<GRUPPE>1	       {
    rcmap=range_init((1 < min) ? min : 1, max, no_art);
}

<GRUPPE>[0-9]+         {
    if (sscanf(yytext, "%ld", &lower) != 1) {
	fprintf(stderr, "%s: error lexing\n", prg);
	exit(1);
    }
    range_set(rcmap, lower);
}

<GRUPPE>,              ;
.                      ;
\n                     BEGIN(INITIAL);
%%
int
parserc (char *gname, FILE *rc, long lo, long hi, long no_arts)
{
    min=lo;
    max=hi;
    no_art=no_arts;
    yyin=rc;
    group=gname;
    yylex();
    return(0);
}




