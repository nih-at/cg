#ifndef _HAD_STREAM_TYPES_H
#define _HAD_STREAM_TYPES_H

#include <stdio.h>

#include "stream.h"
#include "checkgroup.h"
#include "mime.h"

stream *stream_article_open(struct stream *source);
stream *stream_cat_open(struct file *file);
stream *stream_decode_open(stream *source, int *tbl);
stream *stream_fcat_open(int n, char **names);
stream *stream_file_open(FILE *f, int is_nntp);
stream *stream_msg_partial_open(stream *source, struct mime_hdr *m);
stream *stream_msg_multi_open(stream *source, struct mime_hdr *m);
stream *stream_section_open(stream *source, char *boundary);
stream *stream_uuextract_open(stream *source);
stream *stream_yenc_open(struct stream *source, char *ybegin);


#endif /* stream_types.h */
