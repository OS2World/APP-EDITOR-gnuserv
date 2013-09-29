/* -*-C-*-
 Client code to allow local and remote editing of files by GNU Emacs.

 This file is part of GNU Emacs. 

 Copying is permitted under those conditions described by the GNU
 General Public License.

 Copyright (C) 1997 Free Software Foundation, Inc.

 Orginal Author: Andy Norman (ange@hplb.hpl.hp.com), based on 
                 'etc/emacsclient.c' from the GNU Emacs 18.52
                 distribution.

 OS/2 Version Author: Christopher McKillop
                      (cdmckill@novice.uwaterloo.ca),
                      based on the version by Andy Norman.

                      
 Please mail bugs and suggestions to the OS/2 author at the above address.
*/
#define INCL_DOS
#define INCL_WINSWITCHLIST
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>

#include "gnuserv.h"
#include "getopt.h"


static char         ClientMutexName[] = "\\SEM32\\GNUSERV_MUTEX";
static char         ClientSemName[] = "\\SEM32\\GNUSERV_CLIENT";
static char         ServerSemName[] = "\\SEM32\\GNUSERV_SERVER";
static char         SharedBuffName[] = "\\SHAREMEM\\GNUSERV_BUFFER";
static HEV          ClientDoneSem, ServerDoneSem;
static HMTX         ClientMutex;
static void         *SharedMem;
static int          quick_flag = 0;



int gnuclient_handle_args( int argc, char **argv )
{
    int     c;

    while( ( c = getopt( argc, argv, "hq" ) ) != GETOPT_EOF )
    {
        switch( c )
        {
          case 'h':
            printf( "\n\ngnuclient for OS/2\n\n" );
            printf( "  Usage:\n" );
            printf( "    gnuclient [-h] [-q] [[+line] path]\n" );
            printf( "    -h:               Print this message.\n" );
            printf( "    -q:               Don't wait for user to finish edit.\n" );
            printf( "    +line:            Optional line number to start edit at.\n" );
            printf( "    path:             Path to file to edit.\n" );
            printf( "\n\n" );
            printf( "  Example:\n" );
            printf( "    gnuclient -q +50 foo.c bar.c\n\n" );
            fflush( stdout );
            exit( 0 );
            break;

          case 'q':
            quick_flag = 1;
            break;

          default:
            break;
        }
    }

    return 0;
}






int gnuclient_deinit( void )
{
    ULONG ret1, ret2, ret3, ret4;

    /*
     * Destroy local copies of all opened kernel objects.
     */
    ret1 = DosCloseEventSem( ClientDoneSem );
    ret2 = DosCloseEventSem( ServerDoneSem );
    ret3 = DosFreeMem( SharedMem );
    ret4 = DosCloseMutexSem( ClientMutex );
    
    if( ret1 || ret2 || ret3 || ret4 )
    {
        fprintf( stderr,
                 "gnuclient: Error destroying kernel objects.\n" );
        
        fflush( stderr );
        exit( 0 );
    }

    return 0;
}




int gnuclient_init( void )
{
    ULONG   ret1, ret2, ret3, ret4;
    
    /*
     * Open copies of all the server created kernel objects.
     */
    ret1 = DosOpenEventSem( ClientSemName, &ClientDoneSem );
    ret2 = DosOpenEventSem( ServerSemName, &ServerDoneSem );
    ret3 = DosGetNamedSharedMem( &SharedMem, SharedBuffName,
                                 PAG_WRITE );
    ret4 = DosOpenMutexSem( ClientMutexName, &ClientMutex );
    
    
    if( ret1 || ret2 || ret3 || ret4 )
    {
        fprintf( stderr, "gnuclient: Error opening named pipe.\n" );
        fflush( stderr );
        exit( 0 );
    }

    return 0;
}





int gnuclient_make_full_path( char *given_path, char *full_path )
{
    char    cwd[ PATH_MAX + 1 ];
    char    *fixer;
    
    getcwd( cwd, PATH_MAX + 1 );
    strcat( cwd, "/" );

    /*
     * Check to see if the path given is drive relative.
     */
    if( given_path[0] == '\\' || given_path[0] == '/' )
    {
        if( given_path[1] != '/' && given_path[1] != '\\' )
        {
            full_path[0] = cwd[0];
            full_path[1] = ':';
            full_path[2] = '\0';
        }

        strcat( full_path, given_path );
    }
    /*
     * If not, check to see if there is a drive letter
     *  already, if not, must be in cwd.
     */
    else if( given_path[1] != ':')
    {
        strcpy( full_path, cwd );
        strcat( full_path, given_path );
    }
    /*
     * Use what is given to us!
     */
    else
    {
        strcpy( full_path, given_path );
    }
    

    /*
     * Emacs expects all paths to use /'s and not \'s
     */
    fixer = full_path;
    while( *fixer != '\0' )
    {
        if( *fixer == '\\' )
            *fixer = '/';

        fixer = fixer + 1;
    }

    return 0;
}







int main( int argc, char **argv )
{

    ULONG           switch_list;
    ULONG           ret, start_line, i;
    ULONG           post_count;
    char            list_entry[300], full_path[300];
    
    
    
    /*
     * Handle Command Line Args.
     */
    gnuclient_handle_args( argc, argv );

    
    /*
     * Setup the named pipe.
     */
    gnuclient_init();
    

    /*
     * Only allow one gnuclient process at a time to
     *  go any further.
     */
    ret = DosRequestMutexSem( ClientMutex, SEM_INDEFINITE_WAIT );
    if( ret )
    {
        fprintf( stderr, "gnuclient(%d): Error getting mutex.\n",
                 ret );
        fflush( stderr );
        exit( 0 );
    }
    

    /*
     * Clear shared memory.
     */
    memset( SharedMem, 0, 4096 );



    if( quick_flag )
    {
        strcat( SharedMem, "(server-edit-files-quickly '(" );
    }
    else
    {
        strcat( SharedMem, "(server-edit-files '(" );
    }


    for( i=optind; i<argc; i++ )
    {
        start_line = 1;
        
        if( *argv[i] == '+' )
        {
            start_line = atoi( argv[i] + 1 );
            i++;            

            if( i >= argc )
            {
                fprintf( stderr, "gnuclient: Error, no enough parameters.\n" );
                fflush( stderr );
                exit( 0 );
            }
        }
        
        gnuclient_make_full_path( argv[i], full_path );

        sprintf( list_entry, "(%d . \"%s\")", start_line, full_path );

        strcat( SharedMem, list_entry );
    }

    
    strcat( SharedMem, "))" );


    /*
     * Tell server that SharedMem has edit request in it.
     */
    ret = DosPostEventSem( ClientDoneSem );
    if( ret )
    {
        fprintf( stderr, "gnuclient(%d): Error signaling server.\n", ret );
        fflush( stderr );
        exit( 0 );
    }
    

    /*
     * Block until server/emacs is finished with request.
     */
    ret = DosWaitEventSem( ServerDoneSem, SEM_INDEFINITE_WAIT );
    if( ret )
    {
        fprintf( stderr, "gnuclient(%d): Error waiting for server.\n", ret );
        fflush( stderr );
        exit( 0 );
    }

    /*
     * Reset signal for next gnuclient instance.
     */
    DosResetEventSem( ServerDoneSem, &post_count );
    

    /*
     * Free up the mutex, allow another gnuclient process
     *  to procede.
     */
    ret = DosReleaseMutexSem( ClientMutex );
    if( ret )
    {
        fprintf( stderr, "gnuclient(%d): Error releasing mutex.\n",
                 ret );
        fflush( stderr );
        exit( 0 );
    }

    
    /*
     * Clean up the kernel objects.
     */
    gnuclient_deinit();

    
    return 0;
}

        

    
        
        
    
    









