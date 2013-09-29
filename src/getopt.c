/* @(#)getopt.c	2.1 88/08/01 4.0 RPCSRC */

/* this is derived from a public domain version of getopt */

#include <string.h>
#include "getopt.h"


int	    opterr = 1;
int	    optind = 1;
int	    optopt;
char	*optarg;



int getopt(int	argc, char	**argv, char *opts )
{
	static int sp = 1;
	int c;
	char *cp;

	if( sp == 1 )
    {
		if( optind >= argc ||
		    argv[optind][0] != '-' ||
            argv[optind][1] == '\0' )
        {
			return GETOPT_EOF;
        }
		else if( strcmp( argv[optind], "--" ) == NULL )
        {
			optind++;
			return GETOPT_EOF;
		}
    }
    
    
	optopt = c = argv[optind][sp];
    
	if(c == ':' || ( cp=strchr(opts, c) ) == NULL )
    {
		if(argv[optind][++sp] == '\0')
        {
			optind++;
			sp = 1;
		}
		return '?';
	}

    
	if(*++cp == ':')
    {
		if(argv[optind][sp+1] != '\0')
        {
			optarg = &argv[optind++][sp+1];
        }
		else if(++optind >= argc)
        {
			sp = 1;
			return('?');
		}
        else
        {
			optarg = argv[optind++];
        }
		sp = 1;
	}
    else
    {
		if(argv[optind][++sp] == '\0')
        {
			sp = 1;
			optind++;
		}
		optarg = NULL;
	}
	return(c);
}

