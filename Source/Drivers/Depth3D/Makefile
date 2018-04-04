include ../../../ThirdParty/PSCommon/BuildSystem/CommonDefs.mak

BIN_DIR = ../../../Bin

INC_DIRS = \
	../../../Include \
	../../../ThirdParty/PSCommon/XnLib/Include 

SRC_FILES = \
	*.cpp

ifeq ("$(OSTYPE)","Darwin")
	INC_DIRS += /opt/local/include
	LIB_DIRS += /opt/local/lib
	LDFLAGS += -framework CoreFoundation -framework IOKit
endif

LIB_NAME = Depth3D

LIB_DIRS = ../../../ThirdParty/PSCommon/XnLib/Bin/$(PLATFORM)-$(CFG)
	
USED_LIBS = XnLib dl pthread uvc
						
ifneq ("$(OSTYPE)","Darwin")
        USED_LIBS += rt  
endif

CFLAGS += -fPIC -Wall -std=c++11

OUT_DIR := $(OUT_DIR)/OpenNI2/Drivers

include ../../../ThirdParty/PSCommon/BuildSystem/CommonCppMakefile
