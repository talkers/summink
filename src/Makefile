BIN         = ../bin
EW2BIN      = summink
ABIN        = angel
OBJECT-DIR  = ../junk
CC          = gcc
CFLAGS      = -O2
# Need this for Linux
LIBS        = -lg
# For Solaris
#LIBS        = -lmalloc -lsocket -lnsl
#LIBS        =
HDRS        = 
DEFS        =

OBJS        = $(OBJECT-DIR)/admin.o \
              $(OBJECT-DIR)/commands.o \
              $(OBJECT-DIR)/dynamic.o \
              $(OBJECT-DIR)/editor.o \
              $(OBJECT-DIR)/globals.o \
              $(OBJECT-DIR)/glue.o \
              $(OBJECT-DIR)/lists.o \
              $(OBJECT-DIR)/mail.o \
              $(OBJECT-DIR)/parse.o \
              $(OBJECT-DIR)/plists.o \
              $(OBJECT-DIR)/room.o \
              $(OBJECT-DIR)/session.o \
              $(OBJECT-DIR)/socket.o \
              $(OBJECT-DIR)/tag.o \
              $(OBJECT-DIR)/track.o

all:            $(EW2BIN) $(ABIN)
				@echo 'Made all'

mguard:         $(ABIN)


$(ABIN):   angel.c Makefile
				$(CC) $(CFLAGS) -o $(ABIN) $(DEFS) angel.c $(LIBS)

$(EW2BIN):      $(OBJS) Makefile
				$(CC) $(CFLAGS) -o $(EW2BIN) $(DEFS) $(OBJS) $(LIBS)

clean:
	rm -f ../junk/* $(EW2BIN) $(ABIN)

vclean: clean
	rm -f ../logs/*.log ../logs/scripts/* ../logs/scripts/.??* \
	../logs/emergency/* ../logs/emergency/.??*


install:           $(EW2BIN) $(ABIN)
				mv $(EW2BIN) $(BIN)
				chmod 700 $(BIN)/$(EW2BIN)
				mv $(ABIN) $(BIN)
				chmod 700 $(BIN)/$(ABIN)
				@echo 'Made install'

program:           $(EW2BIN)
				@echo 'Made program'

dist:	vclean
	rm -f ../files/players/* ../files/notes/* ../files/defrag/* ../files/rooms/*
	mkdir -p /tmp/summink
	cd ../.. ;\
		tar cvf /tmp/summink/summink.tar summink/* ; \
		cd /tmp/summink ; \
		gzip -9 -c summink.tar > summink.tar.gz ; \
		compress -c  summink.tar > summink.tar.Z ; \
		rm -f /tmp/summink/summink.tar

$(OBJECT-DIR)/%.o:   config.h player.h
				$(CC) $(CFLAGS) $(DEFS) -c $*.c -o $(OBJECT-DIR)/$*.o

###
$(OBJECT-DIR)/glue.o: glue.c 
$(OBJECT-DIR)/socket.o: socket.c 
$(OBJECT-DIR)/globals.o: globals.c 
$(OBJECT-DIR)/plists.o: plists.c compaction.c
$(OBJECT-DIR)/parse.o: parse.c clist.h
$(OBJECT-DIR)/lists.o: lists.c 
$(OBJECT-DIR)/commands.o: commands.c 
$(OBJECT-DIR)/admin.o: admin.c
$(OBJECT-DIR)/room.o: room.c
$(OBJECT-DIR)/mail.o: mail.c
$(OBJECT-DIR)/tag.o: tag.c
$(OBJECT-DIR)/malloc.o: malloc.c
$(OBJECT-DIR)/editor.o: editor.c
$(OBJECT-DIR)/session.o: session.c
$(OBJECT-DIR)/dynamic.o: dynamic.c dynamic.h
$(OBJECT-DIR)/track.o: track.c


