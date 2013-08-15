CXXFLAGS += -O3 -g -I/usr/include/opencv -DBOOST_FILESYSTEM_VERSION=2
LDFLAGS += -Wl,-Bstatic -lsensors -lrt -lboost_signals-mt -lboost_filesystem-mt -lboost_regex-mt -lboost_system-mt -lboost_program_options-mt -lm -Wl,-Bdynamic -lcxcore -lcv -lhighgui -lncurses -pthread -lraw1394

URT_OBJECTS = URT/ArdPort.o URT/EventLoop.o URT/ExternalProgram.o URT/FDEvtSource.o URT/SerialPort.o URT/Socket.o URT/SocketServer.o URT/State.o URT/StateDevice.o URT/StateSocket.o URT/Watchdog.o URT/HotDeviceManager.o URT/DeviceManager.o URT/contrib/Ax3500.o URT/contrib/LMSensors.o
OBJECTS = main.o globals.o Parameters.o AutoPilot.o utilities.o screen.o camera.o

all: trinidad2
trinidad2: $(URT_OBJECTS) $(OBJECTS)
	$(CXX) -o trinidad2 $(URT_OBJECTS) $(OBJECTS) $(LDFLAGS)

.PHONY: all clean clean_all help

help:
	@echo 	Trinidad is the next-generation robot control system for the
	@echo	Robotics Club at UCLA\'s RoboMagellan project \(GLaDOS\). It
	@echo	utilizes the in-house URT library.
	@echo
	@echo	Targets: 
	@echo -e \\t	all:		compile and link trinidad
	@echo -e \\t	trinidad2:	compile and link trinidad
	@echo -e \\t	help:		this message 
	@echo -e \\t	clean:		delete non-URT object files and executable 
	@echo -e \\t	clean_all:	delete all object files and executable
clean:
	rm -f $(OBJECTS) trinidad2
clean_all: clean
	rm -f $(URT_OBJECTS)
