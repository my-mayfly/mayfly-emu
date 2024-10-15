DIFF_REF_PATH = ./spike-diff
DIFF_REF_SO = $(DIFF_REF_PATH)/build/riscv64-spike-so 
MKFLAGS = GUEST_ISA=riscv64 SHARE=1 ENGINE=interpreter

$(DIFF_REF_SO):
	$(MAKE) -s -C $(DIFF_REF_PATH) $(MKFLAGS)

run:$(DIFF_REF_SO)
.PHONY: $(DIFF_REF_SO)

