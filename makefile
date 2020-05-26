.PHONY: all clean test testlib install copylib ldconf lib debug distclean

# buid & link with gcc
CC = gcc
LD = gcc

# néhány apróság:
MKD = /bin/mkdir -p
# A '-' jel miatt átugorja a hibát
RM =  -/bin/rm 
CP = -/bin/cp 

# fordíto általános flagek
LASTVER = $(shell git describe)
LASTVERT = $(shell git describe --tags --abbrev=0)
LASTMAINT = $(firstword $(subst ., ,$(LASTVERT)))
CFLAGS = -Wall -c -D"GIT_LAST=$(LASTVER)" -D"GIT_LASTT=$(LASTVERT)" -D"GIT_MAINT=$(LASTMAINT)"
LDFLAGS = 
LDLIBS =

# Ha libdir-be "lib"-et írunk, akkor körbeforgó fggőség lesz belőle
BINDIR = bin
LIBDIR = libdir
OBJDIR = build
SRCDIR = src
#LASTVER = 2.0.0


# .o fájlok
PROG =  $(BINDIR)/lightconfini
#OBJ = main.o ini_read.o ini_write.o
OBJ  = $(patsubst $(SRCDIR)/%.c, %.o, $(wildcard $(SRCDIR)/*.c))
OBJS = $(addprefix $(OBJDIR)/,$(OBJ))
lib: OBJS = $(addprefix $(OBJDIR)/, $(filter-out main.o, $(OBJ))  )
lib: PROG = $(LIBDIR)/liblightconfini.so.$(LASTVER)
# Egyik targetben beállított változó nem érvényes a másik targetben!


# ALL kell legyen legelőször!
# Az sem mindegy, hogy mellette van, vagy alatta egy sorral!
all: lib
test: $(PROG)
lib:  $(PROG)
debug: clean $(PROG)
install: lib copylib 
testlib: clean lib ldconf clean
	$(CC) $(CFLAGS) -D"TESTLIB=1" -o $(OBJDIR)/main.o $(SRCDIR)/main.c
	$(LD) $(LDFLAGS) $(OBJDIR)/main.o -o $(PROG) $(LDLIBS)

# debug-hoz felüldefiniálva 
debug: CFLAGS = -Wall -c -g -g3 -ggdb -std=c89 -Wpedantic -Wmissing-prototypes -D"GIT_LAST=$(LASTVER)" -D"GIT_LASTT=$(LASTVERT)" -D"GIT_MAINT=$(LASTMAINT)"
debug: LDLIBS = -lefence
# make lib  FLAGS: not link: -c, relative addresses: -fPIC
lib: CFLAGS = -fPIC -c -Wall -D"GIT_LAST=$(LASTVER)" -D"GIT_LASTT=$(LASTVERT)" -D"GIT_MAINT=$(LASTMAINT)"
lib: LDFLAGS = -shared -Wl,-soname,liblightconfini.so.$(LASTMAIN)
lib: LDLIBS = -lc
# nem mindegy, hogy mellette van, vagy alatta egy sorral!
testlib: LDFLAGS = -ldl -rdynamic


# Ha még nem létezik az obj könyvtár, létrehozza
$(OBJS): | $(OBJDIR)
$(PROG): | $(BINDIR) 
$(PROG): | $(LIBDIR)

# Fordított sorrend: először a főprogram
$(PROG): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(PROG) $(LDLIBS)
	@echo ' '

# Utána: A forrásfájlok fordítása egyesével
$(OBJS): $(OBJDIR)/%.o: $(SRCDIR)/%.c  
	$(CC) $(CFLAGS) -o "$@" "$<"


# OBJ létrehozáshoz
$(BINDIR):  
	$(MKD) $(BINDIR)
$(OBJDIR):	
	$(MKD) $(OBJDIR)
$(LIBDIR):	
	$(MKD) $(LIBDIR)

rebuild: clean all

copylib:
	$(CP) -r $(LIBDIR)/* /usr/local/lib/
	ldconfig 

ldconf:
	ldconfig -n $(LIBDIR)

clean:
	$(RM) $(OBJS) 

distclean: clean
	$(RM) -r $(OBJS) $(LIBDIR) $(BINDIR) $(OBJDIR)




