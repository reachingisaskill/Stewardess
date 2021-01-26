#
# Makefile
#

# Installation
INSTALL_DIR =

# The Basics:
INC_DIR = inc
BIN_DIR = bin
SRC_DIR = src
TMP_DIR = .temp

# CCC_FLAGS = 
CCC = @g++ -g -Wall -pedantic -Wextra

# Default Final Program
EXE_SRC = TCPServer.cxx



# Find The Files
SRC_FILES = ${shell ls $(SRC_DIR)}
INC_FILES = ${shell ls $(INC_DIR)}

#Also Includes and Libs
INC_FLAGS += -I${INC_DIR}
LIB_FLAGS += -levent





INCLUDE = $(patsubst %.h,${INC_DIR}/%.h,$(filter %.h,$(INC_FILES)))
INCLUDE+= $(patsubst %.hpp,${INC_DIR}/%.hpp,$(filter %.hpp,$(INC_FILES)))

SOURCES = $(patsubst %.cpp,${SRC_DIR}/%.cpp,$(filter %.cpp,$(SRC_FILES)))
#SOURCES+= $(patsubst %.cxx,${SRC_DIR}/%.cpp,$(filter %.cxx,$(SRC_FILES)))

OBJECTS = $(patsubst %.cpp,$(TMP_DIR)/%.o,$(filter %.cpp,$(SRC_FILES)))
EXE_OBJ = $(patsubst %.cxx,$(TMP_DIR)/%.o,$(filter %.cxx,$(SRC_FILES)))

PROGRAM = ${BIN_DIR}/$(basename ${EXE_SRC})


.PHONY : program all _all build install clean buildall directories includes intro check_install

all : _all



_all : directories intro ${PROGRAM}
	@echo
	@echo "Target : '"$(basename ${EXE_SRC})"' Succesfully Built"

intro:
	@echo "Building Target : " $(basename ${EXE_SRC})
	@echo "Please Wait..."


${PROGRAM} : ${OBJECTS} ${EXE_OBJ}
	@echo " - Building Target"
	${CCC} -o $@ $^ ${INC_FLAGS} ${LIB_FLAGS}


${EXE_OBJ} : ${SRC_DIR}/${EXE_SRC} ${INCLUDE}
	@echo " - Compiling Target : " $(notdir $(basename $@))
	${CCC} -c $< -o $@ ${INC_FLAGS}


${OBJECTS} : ${TMP_DIR}/%.o : ${SRC_DIR}/%.cpp ${INCLUDE}
	@echo " - Compiling Source : " $(notdir $(basename $@))
	${CCC} -c $< -o $@ ${INC_FLAGS}



directories : ${BIN_DIR} ${SRC_DIR} ${INC_DIR} ${TMP_DIR}


${BIN_DIR} :
	mkdir -p ${BIN_DIR}

${SRC_DIR} :
	mkdir -p ${SRC_DIR}

${INC_DIR} :
	mkdir -p ${INC_DIR}

${TMP_DIR} :
	mkdir -p ${TMP_DIR}



clean :
	rm -f ${TMP_DIR}/*
	rm -f ${BIN_DIR}/*


install : check_install
	@echo
	@echo "Installing Program/Libraries"
	@cp ${PROGRAM} ${INSTALL_DIR}/bin/$(notdir $(CURDIR))_$(basename ${EXE_SRC})
	@cp ${INC_DIR}/*.h* ${INSTALL_DIR}/include
	@echo

check_install :
	@if [ -z "${INSTALL_DIR}" ]; then          \
		echo                                    ;\
		echo "  INSTALLATION DIRECTORY NOT SET" ;\
		echo                                    ;\
		exit 1                                  ;\
		fi


