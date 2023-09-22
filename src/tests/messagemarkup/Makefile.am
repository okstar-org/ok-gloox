##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual 

noinst_PROGRAMS = messagemarkup_test

messagemarkup_test_SOURCES = messagemarkup_test.cpp
messagemarkup_test_LDADD = ../../messagemarkup.o ../../tag.o ../../gloox.o \
                    ../../util.o
messagemarkup_test_CFLAGS = $(CPPFLAGS)
