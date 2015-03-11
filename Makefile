#	Fred Smalley
#	VarDownload makefile
#	created:	12-5-2014
#

# Set Environment
export PROJECT_ROOT := $(CURDIR)
#export GSUTIL_PATH := /home/fred/Src/gsutil/gsutil
#export BZIP2_PATH := /bin/bzip2

# Set up R
R_HOME :=	$(shell R RHOME)
RCPPFLAGS :=	$(shell $(R_HOME)/bin/R CMD config --cppflags)
RLDFLAGS :=	$(shell $(R_HOME)/bin/R CMD config --ldflags)
RBLAS :=	$(shell $(R_HOME)/bin/R CMD config BLAS_LIBS)
RLAPACK :=	$(shell $(R_HOME)/bin/R CMD config LAPACK_LIBS)
RCPPINCL :=	$(shell $(R_HOME)/bin/Rscript -e "Rcpp:::CxxFlags()")
RINSIDEINCL :=     $(shell $(R_HOME)/bin/Rscript -e "RInside:::CxxFlags()")
RINSIDELIBS :=     $(shell $(R_HOME)/bin/Rscript -e "RInside:::LdFlags()")

# Variables
TARGET = VarDownload
OBJS := $(patsubst src/%.cpp, %.o, $(wildcard src/*.cpp))
CXX = g++
LIBS = $(RLDFLAGS) $(RBLAS) $(RLAPACK) $(RINSIDELIBS)
INCL = $(PROJECT_ROOT)/include $(RCPPFLAGS) $(RCPPINCL) $(RINSIDEINCL)
CXXFLAGS = -std=c++11 -I $(INCL) -L $(LIBS)

# Default rule
all:	env $(TARGET)

env:
	echo $(PROJECT_ROOT)

$(TARGET) :	$(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)

# Rules
%.o :	src/%.cpp
	$(CXX) $(CXXFLAGS) -c $<

.PHONY:	clean
clean:
	rm $(TARGET) $(OBJS)
