CC=wcc386
RM=rm -f
MV=mv -f
LD=wlink
LDOPTIONS=system os2v2


DEFINES=
RELEASE=gnuserv-2.1alpha
INCLUDES=-i=.
CFLAGS=$(DEFINES) $(INCLUDES) $(X11_INCLUDE) 
CFLAGS+=-bt=os2 -fp5 -5 -zq -bm -sg -mf -oneatx -zp4
DEST=. 
OBJ=.obj
EXE=.exe

# You shouldn't need to change anything below this point.

OBJS=  gnuclient$(OBJ)  gnuserv$(OBJ) gnudoit$(OBJ) getopt$(OBJ)
ALL=   gnuclient$(EXE) gnuserv$(EXE) gnudoit$(EXE) 

all:		$(ALL)


gnuclient$(EXE): gnuclient$(OBJ) gnuserv.h getopt$(OBJ)
	$(LD) $(LDOPTIONS) n gnuclient$(EXE) f gnuclient$(OBJ) f getopt$(OBJ)


gnudoit$(EXE):	gnudoit$(OBJ) gnuserv.h getopt$(OBJ)
	$(LD) $(LDOPTIONS) n gnudoit$(EXE) f gnudoit$(OBJ) f getopt$(OBJ)


gnuserv$(EXE):	gnuserv$(OBJ) gnuserv.h
	$(LD) $(LDOPTIONS) n gnuserv$(EXE) f gnuserv$(OBJ)




gnuclient$(OBJ): gnuclient.c gnuserv.h makefile
	$(CC) $(CFLAGS) gnuclient.c

gnudoit$(OBJ): gnudoit.c gnuserv.h  makefile
	$(CC) $(CFLAGS) gnudoit.c

gnuserv$(OBJ): gnuserv.c gnuserv.h makefile
	$(CC) $(CFLAGS) gnuserv.c

getopt$(OBJ): getopt.c getopt.h makefile
	$(CC) $(CFLAGS) getopt.c



dist:
	$(RM) *$(OBJ) *~ *.err
	$(MV) $(ALL) ..



clean:
		$(RM) $(ALL) *$(OBJ) *~ *.err *$(EXE)
