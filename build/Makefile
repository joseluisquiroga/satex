
# the standard shell for make

EXE_NAM = satex
BASE_DIR = ..

SRC_DIR = $(BASE_DIR)/src

BIN_DIR = $(BASE_DIR)/bin
OBJ_DIR = $(BASE_DIR)/bin/obj
INCLUDE_DIR = $(SRC_DIR)

INCLUDE_FLAGS = -I$(INCLUDE_DIR)
DBG_FLAGS = -DFULL_DEBUG -rdynamic  
#DBG_FLAGS = -rdynamic  
# no dbg times: first 0.15 second 1.5

# CMPL_FLAGS = $(INCLUDE_FLAGS) 
CMPL_FLAGS = $(DBG_FLAGS) $(INCLUDE_FLAGS) -Wall
LNK_FLAGS = $(DBG_FLAGS) -lstdc++ -lgmpxx -lgmp 
#LNK_FLAGS = $(DBG_FLAGS) -lstdc++

CPP = g++

# Suppresses display of executed commands
# $(VERBOSE).SILENT:

default_rule: $(BIN_DIR)/$(EXE_NAM)
	@echo "Finished building "$(EXE_NAM)"."

ALL_OBJS = \
$(OBJ_DIR)/test.o\
$(OBJ_DIR)/brain.o\
$(OBJ_DIR)/solver.o\
$(OBJ_DIR)/config.o\
$(OBJ_DIR)/dimacs.o\
$(OBJ_DIR)/mem.o\
$(OBJ_DIR)/tak_mak.o\
$(OBJ_DIR)/dbg_ic.o\
$(OBJ_DIR)/central.o

ALL_HEADERS = \
$(INCLUDE_DIR)/mem.h\
$(INCLUDE_DIR)/platform.h\
$(INCLUDE_DIR)/bit_row.h\
$(INCLUDE_DIR)/tools.h\
$(INCLUDE_DIR)/config.h\
$(INCLUDE_DIR)/dimacs.h\
$(INCLUDE_DIR)/solver.h\
$(INCLUDE_DIR)/brain.h

#---------------------------------------------------------
# executables
#

$(BIN_DIR)/$(EXE_NAM): $(ALL_OBJS) 
	@echo "Building executable "$(EXE_NAM)" ..."
	$(CPP) $(ALL_OBJS) $(LNK_FLAGS) -o $(BIN_DIR)/$(EXE_NAM)

help:
	@echo "Teclee 'make'"

#---------------------------------------------------------
# clean rule
#

clean: 
	rm -f $(BIN_DIR)/$(EXE_NAM) $(ALL_OBJS)


full: clean $(BIN_DIR)/$(EXE_NAM)
	@echo "Finished full build of "$(EXE_NAM)"."


# Rules to build .o files from their sources:


#---------------------------------------------------------
# object files
#

$(OBJ_DIR)/test.o: $(SRC_DIR)/test.cpp $(ALL_HEADERS)
	$(CPP) -o $(OBJ_DIR)/test.o $(CMPL_FLAGS)  -c $(SRC_DIR)/test.cpp

$(OBJ_DIR)/brain.o: $(SRC_DIR)/brain.cpp $(ALL_HEADERS)
	$(CPP) -o $(OBJ_DIR)/brain.o $(CMPL_FLAGS)  -c $(SRC_DIR)/brain.cpp

$(OBJ_DIR)/solver.o: $(SRC_DIR)/solver.cpp $(ALL_HEADERS)
	$(CPP) -o $(OBJ_DIR)/solver.o $(CMPL_FLAGS)  -c $(SRC_DIR)/solver.cpp

$(OBJ_DIR)/config.o: $(SRC_DIR)/config.cpp $(ALL_HEADERS)
	$(CPP) -o $(OBJ_DIR)/config.o $(CMPL_FLAGS)  -c $(SRC_DIR)/config.cpp

$(OBJ_DIR)/dimacs.o: $(SRC_DIR)/dimacs.cpp $(ALL_HEADERS)
	$(CPP) -o $(OBJ_DIR)/dimacs.o $(CMPL_FLAGS)  -c $(SRC_DIR)/dimacs.cpp

$(OBJ_DIR)/mem.o: $(SRC_DIR)/mem.cpp $(ALL_HEADERS)
	$(CPP) -o $(OBJ_DIR)/mem.o $(CMPL_FLAGS)  -c $(SRC_DIR)/mem.cpp

$(OBJ_DIR)/tak_mak.o: $(SRC_DIR)/tak_mak.cpp $(ALL_HEADERS)
	$(CPP) -o $(OBJ_DIR)/tak_mak.o $(CMPL_FLAGS)  -c $(SRC_DIR)/tak_mak.cpp

$(OBJ_DIR)/dbg_ic.o: $(SRC_DIR)/dbg_ic.cpp $(ALL_HEADERS)
	$(CPP) -o $(OBJ_DIR)/dbg_ic.o $(CMPL_FLAGS)  -c $(SRC_DIR)/dbg_ic.cpp

$(OBJ_DIR)/central.o: $(SRC_DIR)/central.cpp $(ALL_HEADERS)
	$(CPP) -o $(OBJ_DIR)/central.o $(CMPL_FLAGS)  -c $(SRC_DIR)/central.cpp




