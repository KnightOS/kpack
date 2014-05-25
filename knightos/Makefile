OUTDIR:=bin/
BINDIR:=$(OUTDIR)bin/
VARDIR:=$(OUTDIR)var/

all: $(BINDIR)kpm

$(BINDIR)kpm: kpm.asm
	mkdir -p $(BINDIR)
	$(AS) $(ASFLAGS) --define "$(PLATFORM)" --include "$(INCLUDE);$(PACKAGEPATH)/kpm/" kpm.asm $(BINDIR)kpm

clean:
	rm -rf $(OUTDIR)

.PHONY: all clean
