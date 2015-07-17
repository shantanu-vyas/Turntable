CC		:= ccache g++
SRCDIR		 := src
BUILDSRCDIR	 := build

TESTDIR		 := test
BUILDTESTDIR := build/test
BINTESTDIR	 := bin/test

INCLUDEDIR	 := include
HEADEREXT	 := h
SRCEXT		 := cpp
OBJEXT		 := o
EXEEXT		 :=

SOURCES	  := $(shell find $(SRCDIR)		 -type f -name *.$(SRCEXT))
HEADERS	  := $(shell find $(INCLUDEDIR)	 -type f -name *.$(HEADEREXT))
TESTS	  := $(shell find $(TESTDIR)	  -type f -name *.$(SRCEXT))
OBJECTS	  := $(addprefix $(BUILDSRCDIR)/, $(notdir $(SOURCES:.$(SRCEXT)=.$(OBJEXT))))
TOBJECTS  := $(addprefix $(BUILDTESTDIR)/, $(notdir $(TESTS:.$(SRCEXT)=.$(OBJEXT))))
TPROGRAMS := $(addprefix $(BINTESTDIR)/, $(notdir $(TESTS:.$(SRCEXT)=)))

CFLAGS := -std=c++11 -g -O3 -fPIC -I/usr/include/x86_64-linux-gnu -pthread 
LIB= -lARgsub -lARvideo -lARMulti -lAR -lARICP -lAR -lglut -lGLU -lGL -lX11 -lm -lpthread -ljpeg -lgstreamer-0.10 -lgobject-2.0 -lgmodule-2.0 -pthread -lgthread-2.0 -pthread -lglib-2.0 -lxml2 -L /usr/local/lib `pkg-config --libs opencv` -lopencv_core -lopencv_imgproc -lopencv_highgui
INC	   := -I $(INCLUDEDIR) -I/usr/include/gstreamer-0.10 -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include -I/usr/include/libxml2 -I/usr/local/include/opencv/ -I/usr/local/include
FLAGS	:=$(INC)
LDFLAG=-L/usr/lib/x86_64-linux-gnu -L../../lib/linux-x86_64 -L$(LIB_DIR)

test: $(OBJECTS) $(TPROGRAMS) $(TOBJECTS)

$(BINTESTDIR)/% : $(BUILDTESTDIR)/%.$(OBJEXT) $(OBJECTS)
		$(CC) $(FLAGS) $(CFLAGS) -o $@ $^ $(LDFLAG) $(LIB) 

$(BUILDTESTDIR)/%.o : $(TESTDIR)/%.$(SRCEXT) $(HEADERS)
		$(CC) $(FLAGS) $(CFLAGS) -c -o $@ $<

$(BUILDSRCDIR)/%.o : $(SRCDIR)/%.$(SRCEXT) $(HEADERS)
		$(CC) $(FLAGS) $(CFLAGS) -c -o $@ $<

clean:
		@echo "Cleaning build directory...";
		rm	-rf $(BUILDSRCDIR)/*.$(OBJEXT)
		rm	-rf $(BUILDTESTDIR)/*.$(OBJEXT)

