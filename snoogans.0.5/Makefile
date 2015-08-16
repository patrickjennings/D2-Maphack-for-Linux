CC = gcc

CFLAGS = -Wall -m32

MAKEDEPEND = -MD

DEPDIR = .dep

SOFLAGS = -fPIC -shared

LIBS = -lpthread

CSRC := $(wildcard *.c)

ASMSRC := $(wildcard *.S)

OBJ = $(CSRC:%.c=%.o) $(ASMSRC:%.S=%.o)

EXE = snoogans.so

INSTALLPATH = /usr/local/lib

PROCESSDEP = @mkdir -p $(DEPDIR); \
	     cp $*.d $(DEPDIR)/$*.d; \
             sed -e 's/\#.*//' -e 's/^[^:]*: *//' -e 's/ * \\$$//' -e '/^$$/ d' -e 's/$$/:/' < $*.d >> $(DEPDIR)/$*.d; \
	     rm -f $*.d;

.PHONY: build
build: $(OBJ)
	$(CC) $(CFLAGS) $(SOFLAGS) -o $(EXE) $(OBJ) $(LIBS)
	@echo
	@echo "*** BUILD COMPLETE ***"

%.o: %.c
	$(CC) -c $(CFLAGS) $(MAKEDEPEND) -o $@ $<
	$(PROCESSDEP)

%.o: %.S
	$(CC) -c $(CFLAGS) $(MAKEDEPEND) -o $@ $<
	$(PROCESSDEP)

-include $(CSRC:%.c=$(DEPDIR)/%.d) $(ASMSRC:%.S=$(DEPDIR)/%.d)

.PHONY: install
install:
	cp $(EXE) $(INSTALLPATH)

.PHONY: remove
remove:
	rm -f $(INSTALLPATH)/$(EXE)

.PHONY: clean
clean:
	rm -f $(OBJ) $(EXE); \
	rm -rf $(DEPDIR);

