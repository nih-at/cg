#include <ctype.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>

#include <zlib.h>

#include "header.h"
#include "stream.h"
#include "stream_types.h"
#include "util.h"

#define YENC_BEGIN	(_yenc_sym[0])
#define YENC_CRC32	(_yenc_sym[1])
#define YENC_END	(_yenc_sym[2])
#define YENC_LINE	(_yenc_sym[3])
#define YENC_NAME	(_yenc_sym[4])
#define YENC_PART	(_yenc_sym[5])
#define YENC_PCRC32	(_yenc_sym[6])
#define YENC_SIZE	(_yenc_sym[7])
#define YENC_MAX	8

static const char *_yenc_str[YENC_MAX] = {
    "begin",
    "crc32",
    "end",
    "line",
    "name",
    "part",
    "pcrc32",
    "size",
};

static symbol _yenc_sym[YENC_MAX];
static int _yenc_inited = 0;

enum yenc_state {
    Y_HEADER,	/* reading header */
    Y_PRE,	/* body lines before =ybegin */
    Y_BEGIN,	/* just after =ybegin line */
    Y_DATA,	/* in data part */
    Y_POST	/* after =yend line */
};

struct stream_yenc {
    stream st;

    enum yenc_state state;	/* state stream is in */
    int part;			/* current next part */
    long size;			/* file size, from header */
    long pbegin, pend;		/* beginning/ending position of this part */
    long pos;			/* position in file (amount decoded) */
    unsigned long crc, pcrc;	/* computed crc and part crc */
    char *name;			/* file name */
};

static int yenc_close(struct stream_yenc *st);
static token *yenc_get(struct stream_yenc *st);
static unsigned long _yenc_get_hex(char *s);
static int _yenc_get_int(char *s);
static unsigned long _yenc_get_long(char *s);
static void _yenc_handle_begin(struct stream_yenc *this, char *ybegin,
			       int firstp);
static void _yenc_handle_end(struct stream_yenc *this, char *line);
static void _yenc_handle_part(struct stream_yenc *this, char *line);
static void _yenc_init(void);
static symbol _yenc_parse(char **linep);



stream *
stream_yenc_open(struct stream *source, char *ybegin)
{
    struct stream_yenc *this;

    this = (struct stream_yenc *)stream_new(sizeof(struct stream_yenc),
					   yenc_get, yenc_close, source);

   this->state = Y_BEGIN;

    if (!_yenc_inited)
	_yenc_init();

    _yenc_handle_begin(this, ybegin, 1);
    
    return (stream *)this;
}



static int
yenc_close(struct stream_yenc *this)
{
    /* XXX: skip to EOF? */
      
    stream_free((stream *)this);

    return 0;
}



static token *
yenc_get(struct stream_yenc *this)
{
    token *t;
    enum yenc_state state, old_state;
    int i;
    char *p;

    old_state = this->state;

    for (;;) {
	t = stream_get(this->st.source);

	switch (t->type) {
	case TOK_LINE:
	    if (old_state == Y_HEADER)
		break;
	    else if (strncmp(t->line, "=ybegin ", 8) == 0) {
		if (old_state != Y_PRE) {
		    token_set3(stream_enqueue((stream *)this), TOK_ERR, 1,
			       "unexpected =ybegin line");
		    break;
		}

		_yenc_handle_begin(this, t->line, 0);

		state = Y_BEGIN;
	    }
	    else if (strncmp(t->line, "=ypart ", 7) == 0) {
		if (old_state != Y_BEGIN) {
		    token_set3(stream_enqueue((stream *)this), TOK_ERR, 1,
			       "unexpected =ypart line");
		    break;
		}
		
		_yenc_handle_part(this, t->line);

		state = Y_DATA;
	    }
	    else if (strncmp(t->line, "=yend ", 6) == 0) {
		if (old_state != Y_DATA && old_state != Y_BEGIN) {
		    token_set3(stream_enqueue((stream *)this), TOK_ERR, 1,
			       "unexpected =yend line");
		    break;
		}

		_yenc_handle_end(this, t->line);
		
		state = Y_POST;

		if (this->pos == this->size)
		    return TOKEN_EOF;
	    }
	    else {
		switch (old_state) {
		case Y_BEGIN:
		case Y_DATA:
		    i=0;
		    p = t->line;
		    while (*p) {
			if (*p == '=') {
			    if (*p++ == 0) {
				token_set3(stream_enqueue((stream *)this),
					   TOK_ERR, 1, "escape char = "
					   "at end of yenc line");
				break;
			    }
			    *p -= 64;
			}
			t->line[i++] = (*p-42)&0xff;
			p++;
		    }

		    this->state = Y_DATA;

		    this->pos += i;
		    this->crc = crc32(this->crc, t->line, i);
		    this->pcrc = crc32(this->pcrc, t->line, i);

		    return token_set3(&this->st.tok, TOK_DATA, i, t->line);

		default:
		    break;
		    
		}
		break;
	    }
	    break;
	    
	case TOK_EOA:
	    if (old_state != Y_POST)
		token_set3(stream_enqueue((stream *)this), TOK_ERR, 1,
			   "missing =yend in yenc stream");
	    state = Y_HEADER;
	    break;

	case TOK_EOF:
	    token_set3(stream_enqueue((stream *)this), TOK_ERR, 1,
		       "missing =yend in yenc stream");
	    return TOKEN_EOF;
	    
	case TOK_EOH:
	    state = Y_PRE;
	    break;

	default:
	    return t;
	}

	old_state = state;
    }
}



static unsigned long
_yenc_get_hex(char *s)
{
    unsigned long l;
    char *p;

    errno = 0;
    l = strtoul(s, &p, 16);
    if (errno == ERANGE || (*p & !isspace(*p))) {
	/* XXX: error handling */
    }
    
    return l;
}



static int
_yenc_get_int(char *s)
{
    int i;
    char *p;

    errno = 0;
    i = strtol(s, &p, 10);
    if (errno == ERANGE || (*p & !isspace(*p))) {
	/* XXX: error handling */
    }

    return i;
}



static unsigned long
_yenc_get_long(char *s)
{
    unsigned long l;
    char *p;

    errno = 0;
    l = strtol(s, &p, 10);
    if (errno == ERANGE || (*p & !isspace(*p))) {
	/* XXX: error handling */
    }

    return l;
}



static void
_yenc_handle_begin(struct stream_yenc *this, char *ybegin, int firstp)
{
    symbol s;
    char *p;
    unsigned long size;

    if (firstp) {
	this->part = 0;
	this->crc = crc32(0, NULL, 0);
	this->size = -1;
	this->pos = 0;
	this->name = NULL;
    }

    this->part++;
    this->pbegin = this->pos;
    this->pend = -1;
    this->pcrc = crc32(0, NULL, 0);

    do {
	ybegin = strchr(ybegin, ' ');
	s = _yenc_parse(&ybegin);
	
	if (s == YENC_PART) {
	    if (_yenc_get_int(ybegin) != this->part) {
		token_set3(stream_enqueue((stream *)this), TOK_ERR, 1,
			   "unexpected yEnc part");
	    }
	}
	else if (s == YENC_SIZE) {
	    size = _yenc_get_long(ybegin);
	    if (firstp)
		this->size = size;
	    else if (this->size != size) {
		token_set3(stream_enqueue((stream *)this), TOK_ERR, 1,
			   "yEnc: size mismatch between parts");
	    }
	}
	else if (s == YENC_NAME) {
	    ybegin+=strspn(ybegin, "\" \t");
	    p = ybegin+strlen(ybegin)-1;
	    while (strchr("\" \t", *p))
		--p;
	    p[1] = '\0';
		
	    if (firstp) {
		
		this->name = xstrdup(ybegin);
		token_set(stream_enqueue((stream *)this), TOK_FNAME, ybegin);
	    }
	    else {
		if (strcmp(this->name, ybegin) != 0) {
		    token_set3(stream_enqueue((stream *)this), TOK_ERR, 1,
			       "yEnc: file name mismatch between parts");
		}
	    }
	    break;
	}
    } while (s);

}



static void
_yenc_handle_end(struct stream_yenc *this, char *line)
{
    symbol s;
    unsigned long size;

    do {
	line = strchr(line, ' ');
	s = _yenc_parse(&line);
	
	if (s == YENC_PART) {
	    if (_yenc_get_int(line) != this->part) {
		token_set3(stream_enqueue((stream *)this), TOK_ERR, 1,
			   "unexpected yEnc part");
	    }
	}
	else if (s == YENC_SIZE) {
	    size = _yenc_get_long(line);
	    if (this->pbegin+size != this->pos) {
		token_set3(stream_enqueue((stream *)this), TOK_ERR, 1,
			   "yEnc: part size wrong");
	    }
	}
	else if (s == YENC_PCRC32) {
	    if (this->pcrc != _yenc_get_hex(line)) {
		token_set3(stream_enqueue((stream *)this), TOK_ERR, 1,
			   "yEnc: part crc wrong");
	    }
	}
	else if (s == YENC_CRC32) {
	    if (this->crc != _yenc_get_hex(line)) {
		token_set3(stream_enqueue((stream *)this), TOK_ERR, 1,
			   "yEnc: file crc wrong");
	    }
	}
    } while (s);

    if (this->pend != -1 && this->pos != this->pend) {
	token_set3(stream_enqueue((stream *)this), TOK_ERR, 1,
		   "yEnc: part end wrong");
    }
}



static void
_yenc_handle_part(struct stream_yenc *this, char *line)
{
    symbol s;

    do {
	line = strchr(line, ' ');
	s = _yenc_parse(&line);
	
	if (s == YENC_BEGIN) {
	    if (this->pbegin != _yenc_get_int(line)-1) {
		token_set3(stream_enqueue((stream *)this), TOK_ERR, 1,
			   "yEnc part begin wrong");
	    }
	}
	else if (s == YENC_END)
	    this->pend = _yenc_get_long(line);
    } while (s);

}



static void
_yenc_init(void)
{
    int i;

    for (i=0; i<YENC_MAX; i++)
	_yenc_sym[i] = intern((char *)_yenc_str[i]);

    _yenc_inited = 1;
}



static symbol
_yenc_parse(char **linep)
{
    char *p, *q;

    q = *linep;

    if (q == NULL)
	return NULL;
    q += strspn(q, " \t");    

    if ((p=strchr(q, '=')) == NULL)
	return NULL;

    *p = '\0';

    *linep = p+1;
    return intern(q);
}
