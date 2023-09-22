##
## Process this file with automake to produce Makefile.in
##

AM_CPPFLAGS = -pedantic -Wall -pipe -W -Wfloat-equal -Wcast-align -Wsign-compare -Wpointer-arith -Wswitch -Wunknown-pragmas -Wconversion -Wundef -Wcast-qual

noinst_PROGRAMS = reference_test

reference_test_SOURCES = reference_test.cpp
reference_test_LDADD = ../../reference.o ../../tag.o ../../gloox.o \
                    ../../util.o
reference_test_CFLAGS = $(CPPFLAGS)
