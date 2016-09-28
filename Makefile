# Project Build flags
WARNINGS := -Wno-long-long -Wall -pedantic -Werror # -Wswitch-enum -pedantic -Werror
CXXFLAGS := -pthread -std=gnu++11 $(WARNINGS)

#
# Compute tool paths
#
GETOS := python $(NACL_SDK_ROOT)/tools/getos.py
OSHELPERS = python $(NACL_SDK_ROOT)/tools/oshelpers.py
OSNAME := $(shell $(GETOS))
RM := $(OSHELPERS) rm

PNACL_TC_PATH := $(abspath $(NACL_SDK_ROOT)/toolchain/$(OSNAME)_pnacl)
PNACL_CXX := $(PNACL_TC_PATH)/bin/pnacl-clang++
PNACL_FINALIZE := $(PNACL_TC_PATH)/bin/pnacl-finalize
PNACL_CXXFLAGS := -I$(NACL_SDK_ROOT)/include $(CXXFLAGS) 
LDFLAGS := -lopencv_objdetect -lopencv_imgproc -lopencv_video -lopencv_core -lz 
PNACL_LDFLAGS := -L$(NACL_SDK_ROOT)/lib/pnacl/Release -lppapi_cpp -lppapi -lpthread $(LDFLAGS)

IMPROC_HEADERS := imageprocessing.cpp camera.h detection.h instance_factory.hpp imageprocessing_instance.h singleton_factory.hpp timer.h

# Declare the ALL target first, to make the 'all' target the default build
all: cys-pattern-reco.pexe

clean:
	$(RM) cys-pattern-reco.pexe image_processing.bc

image_processing.bc:  $(IMPROC_HEADERS) camera.cpp detection.cpp timer.cpp
	$(PNACL_CXX) -o $@ $< camera.cpp detection.cpp timer.cpp -O2 $(PNACL_CXXFLAGS) $(PNACL_LDFLAGS)

cys-pattern-reco.pexe: image_processing.bc
	$(PNACL_FINALIZE) -o $@ $<

serve:
	python -m SimpleHTTPServer 8000
