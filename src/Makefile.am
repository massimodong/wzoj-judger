## Process this file with automake to produce Makefile.in

## Created by Anjuta

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir). \
	$(LIBCURL_CFLAGS). \
	$(jsoncpp_CFLAGS). \
	$(jsoncpp_CFLAGS)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\"

AM_CFLAGS =\
	 -Wall\
	 -g

bin_PROGRAMS = wzoj_judger

wzoj_judger_SOURCES = \
	main.cc \
	wzoj-judger.h \
	version.cc \
	daemon.cc \
	http.cc \
	jsoncpp.cpp \
	json/json-forwards.h \
	json/json.h \
	judger.cc \
	okcalls64.h \
	okcalls32.h \
	sim.cc \
	udp-listen.cc

wzoj_judger_CXXFLAGS = -std=c++11 

wzoj_judger_LDFLAGS = 

wzoj_judger_LDADD = $(LIBCURL_LIBS) 


