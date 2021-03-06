TARGET              ?= autopilot_main
ARCH                ?= arm7
INCLUDE_BUILD_INFO  ?= 1
OBJDIR              = bin/$(ARCH)
Q                   = @
TOOLS               = ../tools
RADIO               = config/radio/T6EXAP.xml
# Be very careful if you change these, by default the groundstation 
# looks for config/{messages,settings}.xml
MESSAGES        = config/messages.xml
SETTINGS        = config/settings.xml

include arch/Makefile.$(ARCH)

SRC =                               \
    $(TARGET).c                     \
    math/pprz_trig_int.c            \
    math/pprz_geodetic_int.c        \
    math/pprz_geodetic_float.c      \
    lib/debug.c                     \
    rc.c                            \
    imu.c                           \
    fms.c                           \
    autopilot.c                     \
    bomb.c                          \
    ins/booz2_ins.c                 \
    ins/booz2_hf_float.c            \
    ins/booz2_vf_float.c            \
    ahrs/booz_ahrs_aligner.c                \
    ahrs/booz2_filter_attitude_cmpl_euler.c \
    autopilot/control/booz2/booz2_guidance.c       \
    autopilot/control/booz2/booz2_stabilization_rate.c      \
    autopilot/control/booz2/booz2_stabilization_attitude.c  \
    autopilot/booz2_autopilot.c     \
    supervision/booz2_supervision.c \
    comm.c                          \
    comm_autopilot.c                \
    settings.c

# Include arch specific sources
SRC += $(ARCHSRC)
ASMSRC = $(ARCHASMSRC)

# Define programs and commands.
REMOVE      = rm -f

# Include dirs. Encourage full path includes in c files
INCLUDES    = -I . -I math -I arch -I arch/$(ARCH)/include

# Compiler flags.
CFLAGS = $(INCLUDES)
CFLAGS += -Wall
CFLAGS += -Wcast-qual
CFLAGS += -Wimplicit
CFLAGS += -Wcast-align
CFLAGS += -Wpointer-arith
CFLAGS += -Wswitch
CFLAGS += -Wredundant-decls
CFLAGS += -Wreturn-type
CFLAGS += -Wshadow
CFLAGS += -Wunused
CFLAGS += -Wa,-adhlns=$(OBJDIR)/$(notdir $(subst $(suffix $<),.lst,$<))
CFLAGS += -Wstrict-prototypes
CFLAGS += -Wmissing-declarations
CFLAGS += -Wmissing-prototypes
CFLAGS += -Wnested-externs 
CFLAGS += -std=gnu99
CFLAGS += -fshort-enums
CFLAGS += -ffunction-sections
CFLAGS += -DARCH=$(ARCH)
CFLAGS += -DINCLUDE_BUILD_INFO=$(INCLUDE_BUILD_INFO)

# Linker flags.
LDFLAGS = -Wl,-Map=$(OBJDIR)/$(TARGET).map,--cref,-gc-sections
LDFLAGS += -lc
LDFLAGS += -lm

# Include arch specific flags
CFLAGS  += $(ARCHCFLAGS)
LDFLAGS += $(ARCHLDFLAGS)
ASFLAGS = $(ARCHASFLAGS)

# Define all object files.
COBJARM   = $(SRC:%.c=$(OBJDIR)/%.o)
AOBJARM   = $(ASMSRC:%.S=$(OBJDIR)/%.o)

# Define all list files
LST       = $(SRC:%.c=$(OBJDIR)/%.lst)
LST       += $(ASMSRC:%.S=$(OBJDIR)/%.lst)

# Default target.
all: build size

build: generated elf lss sym

elf: $(OBJDIR)/$(TARGET).elf
lss: $(OBJDIR)/$(TARGET).lss 
sym: $(OBJDIR)/$(TARGET).sym

# Display MD5 of binary
md5: $(OBJDIR)/$(TARGET).elf
	@echo MD5 $(OBJDIR)/$(TARGET).elf
	$(Q)md5sum $(OBJDIR)/$(TARGET).elf

# Create cflow callgraph
cflow: $(SRC)
	@echo cflow -o $(OBJDIR)/$(TARGET).cflow
	$(Q)cflow --tree --brief --cpp $(INCLUDES) $(SRC) -o $(OBJDIR)/$(TARGET).cflow 2> /dev/null

# Display size of file.
ELFSIZE = $(SIZE) -A $(OBJDIR)/$(TARGET).elf
size:
	@if [ -f $(OBJDIR)/$(TARGET).elf ]; then echo; $(ELFSIZE); echo; fi

# All generated headers
generated: generated/radio.h generated/build.h generated/messages.h generated/settings.h

# Radio header
generated/radio.h: $(RADIO) $(TOOLS)/gen-radio.py
	@echo GENERATE $@
	$(Q)PYTHONPATH=../groundstation/ $(TOOLS)/gen-radio.py $< > $@

generated/messages.h: $(MESSAGES) $(TOOLS)/gen-messages.py
	@echo GENERATE $@
	$(Q)PYTHONPATH=../groundstation/ $(TOOLS)/gen-messages.py --messages=$< --output=$@

generated/settings.h: $(SETTINGS) $(TOOLS)/gen-settings.py
	@echo GENERATE $@
	$(Q)PYTHONPATH=../groundstation/ $(TOOLS)/gen-settings.py --settings=$< > $@

generated/build.h: $(SRC) $(TOOLS)/gen-build-info.py
	@echo GENERATE $@
	$(Q)PYTHONPATH=../groundstation/ $(TOOLS)/gen-build-info.py --target=$(TARGET) > $@

# Create extended listing file from ELF output file.
$(OBJDIR)/%.lss: $(OBJDIR)/%.elf
	@echo OBJD $@
	$(Q)$(OBJDUMP) -h -S -C $< > $@

# Create a symbol table from ELF output file.
$(OBJDIR)/%.sym: $(OBJDIR)/%.elf
	@echo NM $@
	$(Q)$(NM) -n $< > $@

# Link: create ELF output file from object files.
.SECONDARY : $(OBJDIR)/$(TARGET).elf
.PRECIOUS : $(AOBJARM) $(COBJARM)
$(OBJDIR)/%.elf:  $(AOBJARM) $(COBJARM)
	@echo LD $@
	$(Q)$(CC) $(THUMB) $(CFLAGS) $(AOBJARM) $(COBJARM)  --output $@ $(LDFLAGS)

# Compile: create object files from C source files. ARM-only
$(OBJDIR)/%.o : %.c
	@echo CC $@
	$(Q)test -d $(dir $@) || mkdir -p $(dir $@)
	$(Q)$(CC) -c $(CFLAGS) $< -o $@ 

# Assemble: create object files from assembler source files. ARM-only
$(AOBJARM) : $(OBJDIR)/%.o : %.S
	@echo AS $@
	$(Q)test -d $(dir $@) || mkdir -p $(dir $@)
	$(Q)$(CC) -c $(ASFLAGS) $< -o $@

# Include test framework
GTESTER             = gtester
TEST_PROGS          = test_math
test_math_TESTSRCS  = math/tests/*_tc.c
include Makefile.test

# Arch specific definitions
-include arch/Makefile-targets.$(ARCH)

clean:
	$(REMOVE) $(OBJDIR)/$(TARGET).obj
	$(REMOVE) $(OBJDIR)/$(TARGET).elf
	$(REMOVE) $(OBJDIR)/$(TARGET).map
	$(REMOVE) $(OBJDIR)/$(TARGET).obj
	$(REMOVE) $(OBJDIR)/$(TARGET).a90
	$(REMOVE) $(OBJDIR)/$(TARGET).sym
	$(REMOVE) $(OBJDIR)/$(TARGET).lnk
	$(REMOVE) $(OBJDIR)/$(TARGET).lss
	$(REMOVE) $(OBJDIR)/.depend
	$(REMOVE) $(COBJARM)
	$(REMOVE) $(AOBJARM)
	$(REMOVE) $(LST)
	$(REMOVE) generated/*.h
	$(REMOVE) $(CLEAN_OBJ)

reallyclean:
	$(REMOVE) --recursive ./bin/* 
	$(REMOVE) generated/*.h

# Listing of phony targets.
.PHONY : all size build elf lss sym clean upload

# Dependencies
$(OBJDIR)/.depend:
	@echo DEPEND $@
	@test -d $(OBJDIR) || mkdir -p $(OBJDIR)
	$(Q)$(CC) -MM -MG $(CFLAGS) $(SRC) | sed 's|\([^\.]*\.o\)|$(OBJDIR)/\1|' > $@

ifneq ($(MAKECMDGOALS),clean) 
ifneq ($(MAKECMDGOALS),erase) 
-include $(OBJDIR)/.depend
endif
endif
