###############################################################################
#                       Peripheral UCLA Robotics Toolkit                      #
#                              By Michael Sechooler                           #
#                         For the Robotics Club at UCLA                       #
#                                                                             #
# Required variables:                                                         #
#   * MCU: The MicroController Unit (atmega168, etc.).                        #
#                                                                             #
# Optional variables:                                                         #
#   * CFLAGS: The flags with which to compile object code. The defaults       #
#              are probably the best though; it is suggested not to override  #
#              them.                                                          #
#                                                                             #
# Targets:                                                                    #
#   * libpurt.a (default): Compile the library.                               #
#   * clean: Remove the libpurt.a and all object files.                       #
#                                                                             #
# Suggested use:                                                              #
#   * To compile standalone: make MCU={mcu} [-j]                              #
#     Including the "-j" will compile the library in parallel.                #
#   * To compile recursively as part of another Makefile:                     #
#       PURT/libpurt.a:                                                       #
#            @make -C PURT MCU={mcu}                                          #
#     The @ sign simply prevents make from echoing the recursive command.     #
#                                                                             #
###############################################################################

##################### Default variables (change sparingly) ####################
CC = avr-gcc
AR = avr-ar

CFLAGS = 	-g                  # add debugging information
CFLAGS +=   -O3                 # highest performance optimization
CFLAGS +=   -Wall               # add all (most?) warnings
CFLAGS +=   -Wextra
CFLAGS +=   -fpack-struct       # be efficient with enum and struct sizes
CFLAGS +=   -fshort-enums
CFLAGS +=   -mshort-calls       # use more efficient rjmp and rcall
CFLAGS +=   -funit-at-a-time	# read whole file before optimizing
            
######## Stable variables (change only when PURT sources change enough) #######
OBJECTS = src/raw.o src/raw_stdio.o src/statedevice_common.o \
	src/statedevice_direct.o src/statedevice_buffer.o src/datagram.o
	
################################ Target Recipes ###############################

# Actual targets
libpurt.a: exists_mcu not_overriden_cflags libpurt.a_header $(OBJECTS)
	@echo
	@echo Creating static library archive libpurt.a
	@$(AR) rcs libpurt.a $(OBJECTS)
	
clean:
	rm -f $(OBJECTS) libpurt.a
	
# Overloads for UI purposes and to include MCU	
%.o: %.c exists_mcu not_overriden_cflags
	@echo Compiling $@
	@$(CC) -c $(CFLAGS) -mmcu=$(MCU) -o $@ $<

# UI "targets"
libpurt.a_header:
	@echo Compiling object files for libpurt.a with the following flags:
	@echo $(CFLAGS) -mmcu=$(MCU)
	
# Variable checks
exists_mcu:
ifeq ($(origin MCU), undefined)
	$(error Required variable MCU is not defined.)
endif

not_overriden_cflags:
ifneq ($(origin CFLAGS), file)
	$(warning CFLAGS overriden for building libpurt.a)
endif

.PHONY: clean libpurt.a_header check_mcu


