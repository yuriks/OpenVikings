RUSTC ?= rustc
RUST_FLAGS ?= -O

LIBLVTOOLS := $(shell $(RUSTC) --crate-file-name src/lvtools/lib.rs)
TOOLS := lvt-dump-chunk lvt-decompress-data

.PHONY : all
all : $(TOOLS:%=bin/%)

.PHONY : clean
clean :
	rm -rf bin/ lib/ build/

define make-lib=
lib/$2 : src/$1/lib.rs $3
	@mkdir -p $$(dir $$@) build/
	$$(RUSTC) $$(RUST_FLAGS) -L ./lib --dep-info build/$$(notdir $$@).d $$< --out-dir $$(dir $$@)
endef

define make-bin=
bin/$1 : src/$1/main.rs $2
	@mkdir -p $$(dir $$@) build/
	$$(RUSTC) $$(RUST_FLAGS) -L ./lib --dep-info build/$$(notdir $$@).d $$< -o $$@
endef

-include $(wildcard build/*.d)

$(eval $(call make-lib,lvtools,$(LIBLVTOOLS),))
$(eval $(call make-bin,lvt-dump-chunk,lib/$(LIBLVTOOLS)))
$(eval $(call make-bin,lvt-decompress-data,lib/$(LIBLVTOOLS)))
