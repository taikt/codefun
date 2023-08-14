# Makefile.am - make executable for service layer process
bin_PROGRAMS = stuckdetector

stuckdetector_SOURCES :=\
    StuckDetector_main.cpp \
	EventNotifier.cpp \
	AcceptorHandler.cpp \
	ReaderHandler.cpp \
	Reactor.cpp \
	SocketException.cpp \
	StuckController.cpp \
	StuckDetectorManager.cpp \
	Dispatcher.cpp \
	JobQueue.cpp \
	WorkerThread.cpp \
	TraceDataManager.cpp \
	connectionManager.cpp

stuckdetector_CPPFLAGS := \
	-I$(top_srcdir)/include \
	-Wall \
	-std=c++11 \
	-Wno-unused-parameter \
	-fpermissive \
	-Wreturn-type \
	-Werror=char-subscripts -Werror=nonnull -Werror=parentheses -Werror=sequence-point \
	-Werror=return-type -Werror=shift-count-negative -Werror=shift-count-overflow -Werror=uninitialized \
	-Werror=maybe-uninitialized -Werror=array-bounds -Werror=bool-compare -Werror=div-by-zero \
	-Werror=free-nonheap-object -Werror=sizeof-pointer-memaccess -Werror=sizeof-array-argument \
	-Werror=memset-transposed-args -Werror=address -Werror=overflow \
	-Wformat -Wformat-security -Werror=format-security \
	-D_BUILD_MODULE_ \
	$(AM_CPPFLAGS) \
	$(USE_SERVICE_BIN_CFLAGS) \
	$(USE_SECURE_BIN_CFLAGS)

stuckdetector_LDFLAGS = \
    -liapplication \
    -lihealth \
    -liremotediag \
    -lipower \
    -licommunication \
    -lidiag \
    -llog \
    -ldlt \
    -lservicelayerlog \
    -lpthread \
    -lsystemd \
    -lboost_thread \
    -lboost_system \
	$(USE_SERVICE_BIN_LDFLAGS) \
	$(USE_DEBUG_LDFLAGS) \
	$(USE_EXTERNAL_LDFLAGS) \
	$(USE_SECURE_BIN_LDFLAGS)
