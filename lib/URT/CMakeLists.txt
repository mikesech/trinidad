option(BUILD_CONTRIB "Build contrib classes" ON)
option(USE_LMSENSORS "USE LMSensors" ON)
option(STATIC_BOOST "Link boost libraries statically" ON)

if(BUILD_CONTRIB)
	set(CONTRIB_SOURCES
		contrib/Ax3500.cpp
		contrib/SerLCD.cpp)
	
	if(USE_LMSENSORS)
		set(CONTRIB_SOURCES
			${CONTRIB_SOURCES}
			contrib/LMSensors.cpp)
		set(CONTRIB_LIBRARIES
			sensors)
	endif(USE_LMSENSORS)
endif(BUILD_CONTRIB)

if(STATIC_BOOST)
	set(BOOST_LIBRARIES
		libboost_signals-mt.a
		libboost_filesystem-mt.a
		libboost_system-mt.a
		libboost_regex-mt.a)
else(STATIC_BOOST)
	set(BOOST_LIBRARIES
		boost_signals-mt
		boost_filesystem-mt
		boost_system-mt
		boost_regex-mt)
endif(STATIC_BOOST)

add_library(URT
	ArdPort.cpp
	DeviceManager.cpp
	EventLoop.cpp
	ExternalProgram.cpp
	FDEvtSource.cpp
	HotDeviceManager.cpp
	Log.cpp
	SerialPort.cpp
	SlottedTimer.cpp
	Socket.cpp
	SocketServer.cpp
	State.cpp
	StateDevice.cpp
	StateSocket.cpp
	Timer.cpp
	Watchdog.cpp
	${CONTRIB_SOURCES})
target_link_libraries(URT rt ${BOOST_LIBRARIES} ${CONTRIB_LIBRARIES})
