CC = gcc
CFLAGS = -O2 -finline-functions -g
CFLAGS += -ansi -pedantic -Wall -Winline
LDFLAGS += -g
AR = ar
LIBS = -lm -lGL -lGLU -lglut

#
# For cygwin, uncomment the next one
#
# LIBS = -lm -lopengl32 -lglu32 -lglut32

#
# For debugging, uncomment the next one
#
# CFLAGS += -O0 -DDEBUG -g3 -gdwarf-2

PROGRAMS = catmull-clark

LIB_H = buf.h util.h geometry.h mesh.h meshrend.h obj.h gl.h gl_util.h subd.h editor.h
LIB_OBJS = geometry.o mesh.o meshrend.o obj.o gl_util.o subd.o editor.o
LIB_FILE = libsurf.a

#
# Pretty print
#
V	      = @
Q	      = $(V:1=)
QUIET_CC      = $(Q:@=@echo    '     CC       '$@;)
QUIET_AR      = $(Q:@=@echo    '     AR       '$@;)
QUIET_GEN     = $(Q:@=@echo    '     GEN      '$@;)
QUIET_LINK    = $(Q:@=@echo    '     LINK     '$@;)

all: $(PROGRAMS)

catmull-clark: main.o $(LIB_FILE)
	$(QUIET_LINK)$(CC) $(LDFLAGS) -o $@ $< $(LIB_FILE) $(LIBS)

geometry.o: $(LIB_H)
mesh.o: $(LIB_H)
meshrend.o: $(LIB_H)
gl_util.o: $(LIB_H)
obj.o: $(LIB_H)
subd.o: $(LIB_H)
editor.o: $(LIB_H)
main.o: $(LIB_H)

$(LIB_FILE): $(LIB_OBJS)
	$(QUIET_AR)$(AR) rcs $@ $(LIB_OBJS)

.c.o:
	$(QUIET_CC)$(CC) -o $@ -c $(CFLAGS) $<

clean:
	rm -f *.[oa] *.so $(PROGRAMS) $(LIB_FILE)
