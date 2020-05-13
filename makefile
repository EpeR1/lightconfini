
.PHONY: all clean debug distclean

#LDLIBS= -lm -lpulse-simple -lpulse -lavformat -lavcodec -lswscale -lavutil -lavfilter -lswresample -lavdevice -lpostproc  -lz -lx264  -lva -lrt

LDLIBS= 

# gcc a fordításhoz
CC = gcc
# gcc-vel is linkelje
LD = gcc

# fordító flagek
debug: CFLAGS = -g -Wall -g3 -ggdb -O0  -fmessage-length=0 -lefence # CodeBlocks-nak
#debug: CFLAGS = -O0 -g3 -Wall -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)"
#       -fmessage-length=0 -> "N" karakter hibaüzenet, N=0 akkor minden üzenet új sorba
#       -g3 -> Level 3 debug info
#       -MT"$(@)" -> Target
#       -c nyelvvel kapcsolatos

# fordíto flagek
CFLAGS = -Wall -ggdb -lefence
# Linker flagek
LDFLAGS = -lefence

# tisztára törlés
RM = /bin/rm -rf
MKD = /bin/mkdir -p

# OBJECT mappa
OBJDIR = bin
DEBUGDIR = bin/debug 

# .o fájlok
OBJ =  main.o ini_read.o 
OBJS = $(addprefix $(OBJDIR)/,$(OBJ))


# program neve
#PROG = maxini
PROG = $(OBJDIR)/lightconfini
PROG_D = $(DEBUGDIR)/lightconfini

# make elég, nem szükséges a program neve
all: $(PROG)



# Az elkészül object fájlok összepakolása a prgramba
$(PROG): $(OBJS)
#       @echo 'Linking target: $@'
#       @echo 'Invoking: $(LD) C Linker'
	$(LD) $(LDFLAGS) $(OBJS) -o $(PROG) $(LDLIBS)
#       @echo 'Finished building target: $@'
	@echo ' '

# Ha még nem létezik az obj könyvtár, létrehozza
$(OBJS): | $(OBJDIR)

# A forrásfájlok fordítása egyesével
$(OBJS): $(OBJDIR)/%.o: ./src/%.c   
#       @echo 'Building file: '$<'  
	$(CC) $(CFLAGS) -c -o "$@" "$<"
#       @echo 'Finished building: $<'  
#       @echo ' '

# Az object könyvtár létrehozása
$(OBJDIR):
	$(MKD) $(OBJDIR)


#Debugoláshoz
debug: $(PROG_D)

$(PROG_D): $(OBJS)
	$(MKD) $(DEBUGDIR)
	$(LD) $(LDFLAGS) $(OBJS) -o $(PROG_D) $(LDLIBS)



# Kis takarítás
clean:
	$(RM) $(PROG) $(OBJS) $(PROG_D)


# Teljes takarítás
distclean:
	$(RM) $(PROG) $(OBJS) $(PROG_D) $(OBJDIR) $(DEBUGDIR)




