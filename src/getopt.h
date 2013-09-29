/*
 * Header for public domain version of getopt.c
 */
#if !defined( _GETOPT_H__ )
#define _GETOPT_H__

#define GETOPT_EOF 0

extern int  opterr;
extern int  optind;
extern int  optopt;
extern char	*optarg;


int getopt(int argc, char **argv, char *opts );

#endif
