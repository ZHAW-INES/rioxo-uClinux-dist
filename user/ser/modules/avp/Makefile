# $Id: Makefile,v 1.3 2004/11/01 13:37:24 janakj Exp $
#
# print example module makefile
#
# 
# WARNING: do not run this directly, it should be run by the master Makefile

include ../../Makefile.defs
auto_gen=
NAME=avp.so

ifeq ($(MAKELEVEL), 0)
# make called directly in the module dir!

all::
	cd ../..; make all

else

include ../../Makefile.modules

endif
