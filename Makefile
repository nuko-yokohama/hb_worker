# contrib/hb_worker/Makefile

MODULE_big = hb_worker
OBJS = hb.o hb_worker.o util.o

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
PG_CPPFLAGS = -I$(shell $(PG_CONFIG) --includedir)
PG_LIBS = -L$(shell $(PG_CONFIG) --libdir) -lpq
SHLIB_LINK = -L$(shell $(PG_CONFIG) --libdir) -lpq
include $(PGXS)
else
subdir = contrib/hb_worker
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif
