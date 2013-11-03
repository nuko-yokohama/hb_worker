/* -------------------------------------------------------------------------
 *
 * hb_worker.c
 *
 * -------------------------------------------------------------------------
 */
#include "postgres.h"

/* These are always necessary for a bgworker */
#include "miscadmin.h"
#include "postmaster/bgworker.h"
#include "storage/ipc.h"
#include "storage/latch.h"
#include "storage/lwlock.h"
#include "storage/proc.h"
#include "storage/shmem.h"

/* these headers are used by this particular worker's code */
#include "access/xact.h"
#include "executor/spi.h"
#include "fmgr.h"
#include "lib/stringinfo.h"
#include "pgstat.h"
#include "utils/builtins.h"
#include "utils/snapmgr.h"
#include "tcop/utility.h"

#include "hb.h"

PG_MODULE_MAGIC;

/* Hit and Blow Server conninfo */
#define HB_DEFAULT_CONNINFO        "user=postgres"
char* hb_server_conninfo = NULL;

void		_PG_init(void);

/* flags set by signal handlers */
static volatile sig_atomic_t got_sighup = false;
static volatile sig_atomic_t got_sigterm = false;

/*
 * Signal handler for SIGTERM
 * Set a flag to let the main loop to terminate, and set our latch to wake
 * it up.
 */
static void
worker_spi_sigterm(SIGNAL_ARGS)
{
        int                     save_errno = errno;

        got_sigterm = true;
        if (MyProc)
                SetLatch(&MyProc->procLatch);

        errno = save_errno;
}


static void
hb_worker_main(Datum main_arg)
{
	char* conninfo = (char*) CStringGetDatum(main_arg);

	/* Establish signal handlers before unblocking signals. */
	pqsignal(SIGTERM, worker_spi_sigterm);

	/* We're now ready to receive signals */
	BackgroundWorkerUnblockSignals();

	elog(LOG, "hb_worker conninfo = %s", conninfo);

	/*
	 * Main loop: do this until the SIGTERM handler tells us to terminate
	 */
	while (!got_sigterm)
	{
		hb_main(conninfo);
	}

	proc_exit(0);
}

/*
 * Entrypoint of this module.
 *
 * We register more than one worker process here, to demonstrate how that can
 * be done.
 */
void
_PG_init(void)
{
	BackgroundWorker worker;

	/* get the configuration */
	DefineCustomStringVariable("hb_worker.conninfo",
							"Hit and Blow server connection informatioin.",
							NULL,
							&hb_server_conninfo,
							HB_DEFAULT_CONNINFO,
							PGC_USERSET,
							0,
							NULL,
							NULL,
							NULL);

	/* set up common data for all our workers */
	worker.bgw_flags = BGWORKER_SHMEM_ACCESS |
		BGWORKER_BACKEND_DATABASE_CONNECTION;
	worker.bgw_start_time = BgWorkerStart_RecoveryFinished;
	worker.bgw_restart_time = BGW_NEVER_RESTART;
	worker.bgw_main = hb_worker_main;
        worker.bgw_main_arg = CStringGetDatum(hb_server_conninfo);

        snprintf(worker.bgw_name, BGW_MAXLEN, "hb_worker");

	RegisterBackgroundWorker(&worker);
}
