VERSION = @LIB3DS_VERSION@

SUBDIRS = source examples

bin_SCRIPTS = lib3ds-config

EXTRA_DIST = \
  lib3ds.m4 \
  lib3ds-config.1 \
  lib3ds.sln

m4datadir = $(datadir)/aclocal
m4data_DATA = lib3ds.m4

.PHONY: release snapshot

release:
	rm -rf .deps */.deps
	$(MAKE) distcheck

snapshot:
	$(MAKE) dist-zip distdir=$(PACKAGE)-`date +"%Y%m%d"`

