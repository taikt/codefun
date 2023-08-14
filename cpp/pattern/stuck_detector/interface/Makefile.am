# Makefile.am - libistuckdetector.a

libistuckdetector_sources:= \
    $(top_srcdir)/StuckDetectorProxy.cpp

# Target name
lib_LIBRARIES := libistuckdetector.a

libistuckdetector_includedir = $(includedir)/services/stuckdetector

libistuckdetector_include_HEADERS = $(srcdir)/include/services/stuckdetector/*

libistuckdetector_a_SOURCES := \
    $(libistuckdetector_sources)

libistuckdetector_a_CPPFLAGS := \
    -I$(top_srcdir)/include\
    -I./include \
    -Wall \
    -Wno-unused-parameter \
    $(AM_CPPFLAGS) \
    $(USE_INTERFACE_LIB_CFLAGS) \
    $(USE_SECURE_LIB_CFLAGS)

libistuckdetector_a_LDFLAGS := \
	$(USE_DEBUG_LDFLAGS) \
	$(USE_INTERFACE_LIB_LDFLAGS) \
	$(USE_SECURE_LIB_LDFLAGS)

#Build a DSO too
lib_LTLIBRARIES := libistuckdetector.la
libistuckdetector_la_SOURCES := $(libistuckdetector_a_SOURCES)
libistuckdetector_la_CPPFLAGS := $(libistuckdetector_a_CPPFLAGS)
libistuckdetector_la_LDFLAGS := $(libistuckdetector_a_LDFLAGS)
