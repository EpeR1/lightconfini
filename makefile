.PHONY: all clean debug distclean

# buid & link with gcc
CC = gcc
LD = gcc

# néhány apróság:
MKD = /bin/mkdir -p
# A '-' jel miatt átugorja a hibát
RM =  -/bin/rm 


# fordíto általános flagek
CFLAGS = -Wall
LDFLAGS = 
LDLIBS =


BINDIR = bin
OBJDIR = build

# .o fájlok
OBJ = main.o ini_read.o
PROG =  $(BINDIR)/lightconfini
OBJS = $(addprefix $(OBJDIR)/,$(OBJ))


# ALL kell legyen legelőször!
all: $(PROG)


# debug-hoz felüldefiniálva 
debug: CFLAGS = -Wall -g -g3 -ggdb -std=c89 -Wpedantic -Wmissing-prototypes 
debug: LDFLAGS = 
debug: LDLIBS = -lefence
# nem mindegy, hogy mellette van, vagy alatta egy sorral!
debug: clean all

# Ha még nem létezik az obj könyvtár, létrehozza
$(OBJS): | $(OBJDIR)
$(PROG): | $(BINDIR)

# Fordított sorrend: először a főprogram
$(PROG): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(PROG) $(LDLIBS)
	@echo ' '

# Utána: A forrásfájlok fordítása egyesével
$(OBJS): $(OBJDIR)/%.o: ./src/%.c  
	$(CC) $(CFLAGS) -c -o "$@" "$<"

# OBJ létrehozáshoz
$(BINDIR):  
	$(MKD) $(BINDIR)
$(OBJDIR):	
	$(MKD) $(OBJDIR)

rebuild: clean all

clean:
	$(RM) $(PROG) $(OBJS)

distclean:
	$(RM) -r $(PROG) $(OBJS) bin/ build/




