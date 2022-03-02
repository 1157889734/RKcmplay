TAR_FILE=libcmplayer.a
HEAD_FILE=cmplayer.h

RESUME_DIR_FILES += -path ./subobj -prune -o

SRC_FILES = $(shell find ./  $(RESUME_DIR_FILES) -name "*.cpp" -print)
OBJ_FILES=$(patsubst %.cpp,%.o,$(SRC_FILES))

AVDEC_LIB = ../lib/libavdec.a
RTSP_LIB = ../lib/librtsp.a
DEBUGOUT_LIB = ../lib/libdebugout.a


AVDEC=avdeclib
RTSP=rtsplib
DEBUGOUT=debugoutlib
CXXFLAGS+=-I../include
#-I./avdec -I./rtsp -I./debugout -I/usr/local/qt-4.8.1-arm/include  -I/usr/local/qt-4.8.1-arm/include/QtGui  -I/usr/local/qt-4.8.1-arm/include/QtCore

all:$(TAR_FILE)
$(TAR_FILE): $(RTSP) $(DEBUGOUT) $(OBJ_FILES)
	$(AR) -rc $(TAR_FILE) $(OBJ_FILES) subobj/*.o
	cp $(TAR_FILE) ../lib/
	cp $(HEAD_FILE) ../include

$(AVDEC):
	(cd ../avdec; make all)
	$(AR) x $(AVDEC_LIB)
	mv *.o subobj
	
$(RTSP):
	(cd ../rtsp; make all)
	$(AR) x $(RTSP_LIB)
	mv *.o subobj
	
$(DEBUGOUT):
	(cd ../debugout; make all)
	$(AR) x $(DEBUGOUT_LIB)
	mv *.o subobj	
	
install:
	cp $(TAR_FILE) ../lib/
	cp $(HEAD_FILE) ../include
clean:
	rm -rf  ./*.o
	rm -rf $(TAR_FILE)

