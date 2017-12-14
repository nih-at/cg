`cg` is a semi-automatic newsgroup binary downloader.  It assembles
parts based on subject headers and then offers them in an editor for
the user to choose which files he really wants.

It supports decoding data in the following formats:
- uuencode (both single- and multi-posting binaries)
- MIME (multipart/mixed, message/partial; base64, quoted printable, x-uuencode)
- yEnc

Development on `cg` has stopped.

Start it with `cg somenewsgroup`; `cg -h` offers a short help, should
you need it.

If you make a binary distribution, please include a pointer to the
distribution site:
>    https://nih.at/cg/

Mail suggestions and bug reports to cg@nih.at.
