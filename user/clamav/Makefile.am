#
#  Copyright (C) 2002 - 2004 Tomasz Kojm <tkojm@clamav.net>
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#  MA 02110-1301, USA.

ACLOCAL_AMFLAGS=-I m4
SUBDIRS = libclamunrar libclamunrar_iface libclamav clamscan clamd clamdscan freshclam sigtool clamconf database docs etc clamav-milter test
EXTRA_DIST = FAQ examples BUGS shared libclamav.pc.in UPGRADE COPYING.bzip2 COPYING.lzma COPYING.unrar COPYING.LGPL COPYING.file COPYING.zlib COPYING.BSD contrib

bin_SCRIPTS=clamav-config

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libclamav.pc
