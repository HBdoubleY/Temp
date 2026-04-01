include makeconfig

MAKE = make
SUBDIRS_1 = $(shell find ./ -maxdepth 3 -type d)
DIR_ALL_D =  $(foreach dir,$(SUBDIRS_1),$(wildcard $(dir)/*.d)) 
DIR_ALL_O = $(foreach dir,$(SUBDIRS_1),$(wildcard $(dir)/*.o))

FILDIRS := ./depend% ./obj% $(SRCDIR)/arch/win $(SRCDIR)/drivers/win

SRCDIRS = $(filter-out $(FILDIRS), $(shell find $(SRCDIR) -maxdepth 3 -type d))

#==============obj================#
#$(subst FROM,TO,TEXT) 
OBJROOTDIR = ./obj
OBJDIR = $(subst $(SRCDIR),$(OBJROOTDIR),$(SRCDIRS))                         
#================depend=================#
DEFROOTDIR = ./depend
DEFDIR = $(subst $(SRCDIR),$(DEFROOTDIR),$(SRCDIRS)) 

all: 
	@if [ ! -d "depend" ]; then mkdir "depend" ; fi ;
	@for dir in $(DEFDIR);\
	do \
	if [ ! -d "$$dir" ]; then mkdir "$$dir" ; fi ;\
	done  
	@if [ ! -d "obj" ]; then mkdir "obj" ; fi ;
	@for dir in $(OBJDIR);\
	do \
	if [ ! -d "$$dir" ]; then mkdir "$$dir" ; fi ;\
	done  
	@$(MAKE) -f Makefile_sub; \
	cp -rf ./runcarplay ../../../AppUpdateCarplay/carplay/ 
	

.PHONY:clean cleanall

packall: 
	./autoPackall 
packexe: 
	./autoPackexe 
clean:            
	@rm -f  $(DIR_ALL_O) $(DIR_ALL_D)
	-@rm -rf $(BIN) $(COBJS) $(CPPOBJS) runcarplay.o runcarplay
#rm *.d
	find . -name "*.d" -type f -delete
cleanall:
	@rm -f  $(DIR_ALL_O) $(DIR_ALL_D)
	@rm -f $(BIN) $(COBJS) $(CPPOBJS) runcarplay.o runcarplay
	@rm -rf depend
	@rm -rf obj
