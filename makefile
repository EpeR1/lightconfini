.PHONY: all clean install copylib ldconf lib debug distclean

# buid & link with gcc
CC = gcc
LD = gcc

# néhány apróság:
MKD = /bin/mkdir -p
# A '-' jel miatt átugorja a hibát
RM =  -/bin/rm 
CP = -/bin/cp 

# fordíto általános flagek
CFLAGS = -Wall -c
LDFLAGS = 
LDLIBS =

# Ha libdir-be "lib"-et írunk, akkor körbeforgó fggőség lesz belőle
BINDIR = bin
LIBDIR = libdir
OBJDIR = build


# .o fájlok
OBJ = main.o ini_read.o ini_write.o
PROG =  $(BINDIR)/lightconfini
LIB = $(LIBDIR)/liblightconfini.so.1.0.1
lib: OBJ := $(filter-out main.o, $(OBJ))
OBJS = $(addprefix $(OBJDIR)/,$(OBJ))


# ALL kell legyen legelőször!
# Az sem mindegy, hogy mellette van, vagy alatta egy sorral!
all: $(PROG)
lib: clean $(LIB)
debug: clean all
install: lib copylib 
testlib: clean all


# debug-hoz felüldefiniálva 
debug: CFLAGS = -Wall -c -g -g3 -ggdb -std=c89 -Wpedantic -Wmissing-prototypes 
debug: LDFLAGS = 
debug: LDLIBS = -lefence
# make lib  FLAGS: not link: -c, relative addresses: -fPIC
lib: CFLAGS = -fPIC -c -Wall 
lib: LDFLAGS = -shared -Wl,-soname,liblightconfini.so.1
lib: LDLIBS = -lc
# nem mindegy, hogy mellette van, vagy alatta egy sorral!

# Ha még nem létezik az obj könyvtár, létrehozza
$(OBJS): | $(OBJDIR)
$(PROG): | $(BINDIR)
$(LIB):  | $(LIBDIR)

# Fordított sorrend: először a főprogram
$(PROG): $(OBJS)
	$(LD) $(LDFLAGS) -o $(PROG) $(OBJS) $(LDLIBS)
	@echo ' '

# For shared library
$(LIB): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(LIB) $(LDLIBS)
	@echo ' '

# Utána: A forrásfájlok fordítása egyesével
$(OBJS): $(OBJDIR)/%.o: ./src/%.c  
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




