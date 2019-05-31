# Compiler

CXX = mpic++
INCLUDE_FLAGS = \
	-I $(MOSEK_DIR)/h \
	-I $(MOSEK_DIR)/src/fusion_cxx \
	-I $(MOSEK_DIR)/include \
	-I $(EIGEN_DIR)/include
LINK_FLAGS = \
	-L $(MOSEK_DIR)/bin \
	-Wl,-rpath-link,$(MOSEK_DIR)/bin
CXX_FLAGS = -std=c++11 -O3 -march=native -fopenmp


# Targets

SRC = \
	sdp.cpp \
	round.cpp \
	branch.cpp \
	node.cpp \
	node_queue.cpp \
	triangle_inequality.cpp \
	freeze_map.cpp \
	mcbb_impl.cpp \
	mcbb.cpp \
	brute_force.cpp \
	mpi_util.cpp \
	eigen_util.cpp

MOSEK_SRC = \
	$(MOSEK_DIR)/src/fusion_cxx/BaseModel.cc \
	$(MOSEK_DIR)/src/fusion_cxx/fusion.cc \
	$(MOSEK_DIR)/src/fusion_cxx/IntMap.cc \
	$(MOSEK_DIR)/src/fusion_cxx/mosektask.cc \
	$(MOSEK_DIR)/src/fusion_cxx/SolverInfo.cc \
	$(MOSEK_DIR)/src/fusion_cxx/StringBuffer.cc \
	$(MOSEK_DIR)/src/fusion_cxx/Debug.cc

OBJ = $(patsubst %.cpp,%.o,$(SRC))
MOSEK_OBJ = $(notdir $(patsubst %.cc,%.o,$(MOSEK_SRC)))

TARGETS = mcbb


# MOSEK Fusion Rules

%.o: $(MOSEK_DIR)/src/fusion_cxx/%.cc $(MOSEK_DIR)/src/fusion_cxx/*.h
	$(CXX) -c -o $@ $(CXX_FLAGS) $(INCLUDE_FLAGS) $<


# Rules

%.o: %.cpp *.h $(MOSEK_DIR)/src/fusion_cxx/*.h
	$(CXX) -c -o $@ $(CXX_FLAGS) $(INCLUDE_FLAGS) $<

mcbb: $(OBJ) $(notdir $(MOSEK_OBJ))
	$(CXX) -o $@ $(CXX_FLAGS) \
		$(INCLUDE_FLAGS) $(LINK_FLAGS) \
		$(OBJ) $(notdir $(MOSEK_OBJ)) \
		-lmosek64

all: $(TARGETS)

clean:
	-$(RM) *.o $(TARGETS) *~

.PHONY: all, clean
