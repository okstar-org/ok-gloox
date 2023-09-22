##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual

noinst_PROGRAMS = hint_test

hint_test_SOURCES = hint_test.cpp
hint_test_LDADD = ../../hint.o ../../tag.o ../../gloox.o \
                    ../../util.o
hint_test_CFLAGS = $(CPPFLAGS)
