BIN = qbe

V = @
OBJDIR = obj

SRC    = main.c util.c parse.c cfg.c mem.c ssa.c alias.c load.c copy.c fold.c \
         live.c spill.c rega.c
X64SRC = arch-x64/targ.c arch-x64/sysv.c arch-x64/isel.c arch-x64/emit.c

X64OBJ = $(X64SRC:%.c=$(OBJDIR)/%.o)
OBJ = $(SRC:%.c=$(OBJDIR)/%.o) $(SHROBJ) $(X64OBJ)

CFLAGS += -Wall -Wextra -std=c99 -g -pedantic

$(OBJDIR)/$(BIN): $(OBJ) $(OBJDIR)/timestamp
	@test -z "$(V)" || echo "ld $@"
	$(V)$(CC) $(LDFLAGS) $(OBJ) -o $@

$(OBJDIR)/%.o: %.c $(OBJDIR)/timestamp
	@test -z "$(V)" || echo "cc $<"
	$(V)$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/timestamp:
	@mkdir -p $(OBJDIR)
	@mkdir -p $(OBJDIR)/arch-x64
	@touch $@

$(OBJ): all.h
$(X64OBJ): arch-x64/x64.h
obj/main.o: config.h arch-x64/x64.h # fixme

config.h:
	@case `uname` in                                 \
	*Darwin*)  echo "#define Defaultasm Gasmacho" ;; \
	*)         echo "#define Defaultasm Gaself" ;;   \
	esac > $@

install: $(OBJDIR)/$(BIN)
	mkdir -p "$(DESTDIR)/$(PREFIX)/bin/"
	cp $< "$(DESTDIR)/$(PREFIX)/bin/"

uninstall:
	rm -f "$(DESTDIR)/$(PREFIX)/bin/$(BIN)"

clean:
	rm -fr $(OBJDIR)

clean-gen: clean
	rm -f config.h

check: $(OBJDIR)/$(BIN)
	tools/unit.sh all

80:
	@for F in $(SRC);                          \
	do                                         \
		awk "{                             \
			gsub(/\\t/, \"        \"); \
			if (length(\$$0) > $@)     \
				printf(\"$$F:%d: %s\\n\", NR, \$$0); \
		}" < $$F;                          \
	done

.PHONY: clean clean-gen check 80 install uninstall
