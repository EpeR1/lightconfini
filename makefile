.PHONY: all clean mkdir main test testlib testlibd install copylib ldconf lib debug distclean

# buid & link with gcc
CC = gcc
LD = gcc

# néhány apróság:
MKD = /bin/mkdir -p
# A '-' jel miatt átugorja a hibát
RM =  -/bin/rm 
CP = -/bin/cp 

# fordíto általános flagek
PWD = $(shell pwd)
#LASTVER = 2.0.0
LASTVER = $(shell git describe)
LASTVERT = $(shell git describe --tags --abbrev=0)
LASTMAINT = $(firstword $(subst ., ,$(LASTVERT)))
VERSTRING = -D"GIT_LAST=\"$(LASTVER)\"" -D"GIT_LASTT=\"$(LASTVERT)\"" -D"GIT_MAINT=\"$(LASTMAINT)\""
CFLAGS = -Wall -c $(VERSTRING)
LDFLAGS = 
LDLIBS =
PREP=

# Ha libdir-be "lib"-et írunk, akkor körbeforgó fggőség lesz belőle
BINDIR = bin
LIBDIR = libdir
OBJDIR = build
SRCDIR = src


# .o fájlok
PROG =  $(BINDIR)/lightconfini
#OBJ = main.o ini_read.o ini_write.o
OBJ  = $(patsubst $(SRCDIR)/%.c, %.o, $(wildcard $(SRCDIR)/*.c))
OBJS = $(addprefix $(OBJDIR)/,$(OBJ))
lib: OBJS = $(addprefix $(OBJDIR)/, $(filter-out main.o, $(OBJ))  )
lib: PROG = $(LIBDIR)/liblightconfini.so.$(LASTVERT)
# Egyik targetben beállított változó nem érvényes a másik targetben!


# ALL kell legyen legelőször!
# Az sem mindegy, hogy mellette van, vagy alatta egy sorral!
all: lib 
test: clean $(PROG)
lib:  $(PROG) ldconf
debug: clean $(PROG)
install: lib copylib 
#testlib: lib ldconf clean main
#testlibd: lib ldconf clean main
testlib: clean mkdir main
testlibd: clean mkdir main

# debug-hoz felüldefiniálva 
debug: CFLAGS = -Wall -c  -O0 -g -g3 -ggdb -std=c89 -Wpedantic -Wmissing-prototypes $(VERSTRING)
debug: LDLIBS = -lefence
# make lib  FLAGS: not link: -c, relative addresses: -fPIC
lib: CFLAGS = -fPIC -c -Wall $(VERSTRING)
lib: LDFLAGS = -shared -Wl,-soname,liblightconfini.so.$(LASTMAINT)
lib: LDLIBS = -lc
# nem mindegy, hogy mellette van, vagy alatta egy sorral!
testlib: LDFLAGS = -L $(PWD)/$(LIBDIR)/
testlib: LDLIBS = -llightconfini
#Dinamikus betöltés tesztelése
testlibd: LDFLAGS = -rdynamic
testlibd: LDLIBS = -ldl
testlibd: PREP = -D"TESTLIBD"
#Load lib from /user/local/lib/
#testlibd: PREP = -D"TESTLIBD" -D"LIBINSTALLED"


# Ha még nem létezik az obj könyvtár, létrehozza
$(OBJS): | $(OBJDIR)
$(PROG): | $(BINDIR) 
$(PROG): | $(LIBDIR)

# Fordított sorrend: először a főprogram
$(PROG) lnk: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(PROG) $(LDLIBS)
	@echo ' '

# Utána: A forrásfájlok fordítása egyesével
$(OBJS): $(OBJDIR)/%.o: $(SRCDIR)/%.c  
	$(CC) $(CFLAGS) -o "$@" "$<"

# A main újrafordítása
main:
	$(CC) $(CFLAGS) $(PREP) -o $(OBJDIR)/main.o $(SRCDIR)/main.c 
	$(LD) $(LDFLAGS) $(OBJDIR)/main.o -o $(PROG) $(LDLIBS)

# OBJ létrehozáshoz
$(BINDIR):  
	$(MKD) $(BINDIR)
$(OBJDIR):	
	$(MKD) $(OBJDIR)
$(LIBDIR):	
	$(MKD) $(LIBDIR)

rebuild: clean all

copylib:
	$(CP) -f $(SRCDIR)/lightconfini.h /usr/local/include/
	$(CP) -fd $(LIBDIR)/* /usr/local/lib/ 
	ln -sf /usr/local/lib/liblightconfini.so.$(LASTVERT) /usr/local/lib/liblightconfini.so
	ldconfig

ldconf:
	ln -sf $(PWD)/$(LIBDIR)/liblightconfini.so.$(LASTVERT) $(PWD)/$(LIBDIR)/liblightconfini.so
	ldconfig -n $(PWD)/$(LIBDIR)/

mkdir:
	$(MKD) $(BINDIR)
	$(MKD) $(OBJDIR)

clean:
	$(RM) $(OBJS) 

distclean: clean
	$(RM) -r $(OBJS) $(LIBDIR) $(BINDIR) $(OBJDIR)




