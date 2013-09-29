/* -*-C-*-
 Server code for handling requests from clients and forwarding them
 on to the GNU Emacs process.

 This file is part of GNU Emacs.

 Copying is permitted under those conditions described by the GNU
 General Public License.

 Copyright (C) 1997 Free Software Foundation, Inc.

 Original Author: Andy Norman (ange@hplb.hpl.hp.com), based on
                  'etc/server.c' from the 18.52 GNU Emacs
                  distribution.

                  
 OS/2 Version Author: Christopher McKillop
                      (cdmckill@novice.uwaterloo.ca),
                      based on the version by Andy Norman.
                      
 Please mail bugs and suggestions to the OS/2 author at the
  above address.
*/
#define INCL_DOS
#define INCL_WINSWITCHLIST
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>

#include "gnuserv.h"


static char         ClientMutexName[] = "\\SEM32\\GNUSERV_MUTEX";
static char         ClientSemName[] = "\\SEM32\\GNUSERV_CLIENT";
static char         ServerSemName[] = "\\SEM32\\GNUSERV_SERVER";
static char         SharedBuffName[] = "\\SHAREMEM\\GNUSERV_BUFFER";
static HEV          ClientDoneSem, ServerDoneSem;
static HMTX         ClientMutex;
static void         *SharedMem;






int gnuserv_init( void )
{
    ULONG   ret1, ret2, ret3, ret4;
    
    /*
     * Create all the kernel objects.
     */
    ret1 = DosCreateEventSem( ClientSemName, &ClientDoneSem,
                             0L, FALSE );

    ret2 = DosCreateEventSem( ServerSemName, &ServerDoneSem,
                              0L, FALSE );

    ret3 = DosAllocSharedMem( &SharedMem, SharedBuffName,
                              4096, PAG_COMMIT | PAG_WRITE );
    
    ret4 = DosCreateMutexSem( ClientMutexName, &ClientMutex,
                              0L, FALSE );
    
    if( ret1 || ret2 || ret3 || ret4 )
    {
        fprintf( stderr, "Error creating kernel objects.\n" );
        fflush( stderr );
        exit( 0 );
    }

    return 0;
}







int main( int argc, char **argv )
{

    ULONG           switch_list;
    ULONG           ret;
    ULONG           offset, len;
    ULONG           post_count;
    CHAR            dummy;


    /*
     * Remove GNUserv process from Window List
     */
    switch_list = WinQuerySwitchHandle( NULLHANDLE, getpid() );
    WinRemoveSwitchEntry( switch_list );

    /*
     * Setup the named pipe.
     */
    gnuserv_init();


    /*
     * Loop for as long as Emacs is still running.
     */
    while( 1 )
    {
        /*
         * Connect to a client, block until made.
         */
        ret = DosWaitEventSem( ClientDoneSem, SEM_INDEFINITE_WAIT );
        if( ret )
        {
            fprintf( stderr, "Error waiting for client.\n" );
            fflush( stderr );
            exit( 0 );
        }

        /*
         * Reset for later!
         */
        DosResetEventSem( ClientDoneSem, &post_count );
        
        /*
         * Give message to emacs.
         */
        printf( "%d %s %s", getpid(), SharedMem, EOT_STRING );
        fflush( stdout );


        /*
         * Eat responce from Emacs.
         */
		len = read( 0, &dummy, 1 );
		while( len > 0 )
		{
			if( dummy == ':' )
				break;

			len = read( 0, &dummy, 1 );
		}

        /*
         * Tell gnuclient that everything went done proper.
         */
        ret = DosPostEventSem( ServerDoneSem );
    }

    return 0;
}

        

    
        
        
    
    









