/* -*- c -*- */
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

<GRUPPE>[0-9]+-[0-9]+  {
    if (sscanf(yytext, "%ld-%ld", &lower, &upper) != 2) {
	fprintf(stderr, "%s: error lexing\n", prg);
	exit(1);
    }

    if (lower < min)
	lower = min;
    if (upper > max)
	upper = max;

    if (lower <= upper) {
	if (rcmap == NULL) {
	    if (lower == min) {
		rcmap = range_init(upper+1, (upper == max) ? upper+1 : max,
				   no_art);
	    }
	    else {
		rcmap = range_init(min, max, no_art);
		range_fill(rcmap, lower, upper, 1);
	    }
	}
	else
            range_fill(rcmap, lower, upper, 1);
    }
}

<GRUPPE>[0-9]+         {
    if (sscanf(yytext, "%ld", &lower) != 1) {
	fprintf(stderr, "%s: error lexing\n", prg);
	exit(1);
    }
    if (lower >= min && lower <= max) {
	if (rcmap == NULL)
	    rcmap = range_init((lower == min) ? min+1 : min, max, no_art);
	range_set(rcmap, lower);
    }
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




