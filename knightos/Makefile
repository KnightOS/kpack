OUTDIR:=bin/
BINDIR:=$(OUTDIR)bin/
VARDIR:=$(OUTDIR)var/

all: $(BINDIR)kpkg

$(BINDIR)kpkg: kpkg.asm
	mkdir -p $(BINDIR)
	$(AS) $(ASFLAGS) --define "$(PLATFORM)" --include "$(INCLUDE);$(PACKAGEPATH)/kpkg/" kpkg.asm $(BINDIR)kpkg

clean:
	rm -rf $(OUTDIR)

.PHONY: all clean
