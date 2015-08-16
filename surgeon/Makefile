CC = gcc

CFLAGS = -Wall -m32

MAKEDEPEND = -MD

DEPDIR = .dep

CSRC := $(wildcard *.c)

ASMSRC := $(wildcard *.S)

OBJ = $(CSRC:%.c=%.o) $(ASMSRC:%.S=%.o)

EXE = surgeon

INSTALLPATH = /usr/bin

PROCESSDEP = @cp $*.d $(DEPDIR)/$*.d; \
             sed -e 's/\#.*//' -e 's/^[^:]*: *//' -e 's/ * \\$$//' -e '/^$$/ d' -e 's/$$/:/' < $*.d >> $(DEPDIR)/$*.d; \
	     rm -f $*.d;

.PHONY: build
build: $(OBJ)
	$(CC) $(CFLAGS) -o $(EXE) $(OBJ)
	@echo
	@echo "*** BUILD COMPLETE ***"

.PHONY: debug
debug: CFLAGS += -g
debug: $(OBJ)
	$(CC) $(CFLAGS) -o $(EXE) $(OBJ)
	@echo
	@echo "*** DEBUG BUILD COMPLETE ***"

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
	rm -f $(OBJ) $(EXE)

