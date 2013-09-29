/* -*-C-*-
 Client code to locally and remotely evaluate lisp forms using GNU Emacs.

 This file is part of GNU Emacs.

 Copying is permitted under those conditions described by the GNU
 General Public License.

 Copyright (C) 1997 Free Software Foundation, Inc.

 Original Author: Andy Norman (ange@hplb.hpl.hp.com).

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



int gnudoit_handle_args( int argc, char **argv )
{
    int     c;

    while( ( c = getopt( argc, argv, "hq" ) ) != GETOPT_EOF )
    {
        switch( c )
        {
          case 'h':
            printf( "\n\ngnudoit for OS/2\n\n" );
            printf( "  Usage:\n" );
            printf( "    gnudoit [-h] [-q] [sexpr]\n" );
            printf( "    -h:               Print this message.\n" );
            printf( "    -q:               Don't wait for user to finish edit.\n" );
            printf( "\n\n" );
            printf( "  Example:\n" );
            printf( "    gnudoit -q (load-library gnuserv)\n\n" );
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






int gnudoit_deinit( void )
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
                 "gnudoit: Error destroying kernel objects.\n" );
        
        fflush( stderr );
        exit( 0 );
    }

    return 0;
}




int gnudoit_init( void )
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
        fprintf( stderr, "gnudoit: Error opening named pipe.\n" );
        fflush( stderr );
        exit( 0 );
    }

    return 0;
}






int main( int argc, char **argv )
{

    ULONG           switch_list;
    ULONG           ret, start_line, i;
    ULONG           post_count;
    ULONG           stdin_fileno, len;
    char            user_input[2048];
    
    
    
    
    /*
     * Handle Command Line Args.
     */
    gnudoit_handle_args( argc, argv );

    
    /*
     * Setup the named pipe.
     */
    gnudoit_init();
    

    /*
     * Only allow one gnuclient process at a time to
     *  go any further.
     */
    ret = DosRequestMutexSem( ClientMutex, SEM_INDEFINITE_WAIT );
    if( ret )
    {
        fprintf( stderr, "gnudoit(%d): Error getting mutex.\n",
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
        strcat( SharedMem, "(server-eval-quickly '(progn " );
    }
    else
    {
        strcat( SharedMem, "(server-eval '(progn " );
    }


    if( optind < argc )
    {
        for( i=optind; i<argc; i++ )
        {
            strcat( SharedMem, argv[i] );
            strcat( SharedMem, " " );
        }
    }
    else
    {
        fgets( user_input, sizeof( user_input ), stdin );
        strcat( SharedMem, user_input );
    }
    
    strcat( SharedMem, "))" );


    /*
     * Tell server that SharedMem has edit request in it.
     */
    ret = DosPostEventSem( ClientDoneSem );
    if( ret )
    {
        fprintf( stderr, "gnudoit(%d): Error signaling server.\n", ret );
        fflush( stderr );
        exit( 0 );
    }
    

    /*
     * Block until server/emacs is finished with request.
     */
    ret = DosWaitEventSem( ServerDoneSem, SEM_INDEFINITE_WAIT );
    if( ret )
    {
        fprintf( stderr, "gnudoit(%d): Error waiting for server.\n", ret );
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
        fprintf( stderr, "gnudoit(%d): Error releasing mutex.\n",
                 ret );
        fflush( stderr );
        exit( 0 );
    }

    
    /*
     * Clean up the kernel objects.
     */
    gnudoit_deinit();

    
    return 0;
}

        

    
        
        
    
    









