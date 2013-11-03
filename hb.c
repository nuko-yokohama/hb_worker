/*
 * hb.c
 *       Hit & Blow server core.
 */

#ifdef WIN32
#include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>

#include "postgres.h"
#include "libpq/pqformat.h"
#include "libpq-fe.h"
#include "utils/elog.h"

#include "util.h"
#include "hb.h"

static void
exit_nicely(PGconn *conn)
{
	PQfinish(conn);
	exit(1);
}

int
hb_main(const char* conninfo)
{
	PGconn	   *conn;
	PGresult   *res;
	PGnotify   *notify;
	int			nnotifies;
	int checked;
	int result;

	char number[5];
	char notify_buf[1024];

	srand((unsigned)time(NULL));

	/* Make a connection to the database */
	conn = PQconnectdb(conninfo);

	/* Check to see that the backend connection was successfully made */
	if (PQstatus(conn) != CONNECTION_OK)
	{
		elog(WARNING, "Connection to database failed: %s",
				PQerrorMessage(conn));
		exit_nicely(conn);
	}

	/*
	 * Issue LISTEN command to enable notifications from the rule's NOTIFY.
	 */
	res = PQexec(conn, "LISTEN HB_SV");
	if (PQresultStatus(res) != PGRES_COMMAND_OK)
	{
		elog(WARNING, "LISTEN command failed: %s", PQerrorMessage(conn));
		PQclear(res);
		exit_nicely(conn);
	}

	/*
	 * should PQclear PGresult whenever it is no longer needed to avoid memory
	 * leaks
	 */
	PQclear(res);

	/* Set Secret Number */
	memset(number, 0x00, 5);
	create_random_number(number);
	elog(LOG , "number=%s\n", number);

	/* Quit after four notifies are received. */
	nnotifies = 0;
	while (1)
	{
		/*
		 * Sleep until something happens on the connection.  We use select(2)
		 * to wait for input, but you could also use poll() or similar
		 * facilities.
		 */
		int			sock;
		fd_set		input_mask;

		sock = PQsocket(conn);

		if (sock < 0)
			break;				/* shouldn't happen */

		FD_ZERO(&input_mask);
		FD_SET(sock, &input_mask);

		if (select(sock + 1, &input_mask, NULL, NULL, NULL) < 0)
		{
			elog(WARNING, "select() failed: %s\n", strerror(errno));
			exit_nicely(conn);
		}

		/* Now check for input */
		PQconsumeInput(conn);
		while ((notify = PQnotifies(conn)) != NULL)
		{
			checked = check_number(notify->extra);
			switch (checked) {
				case NUMBER_COMMAND:
					result = compare_numbers(number, notify->extra);
					if (GET_HITS(result) == 4) {
						// Notify Game Clear, and Set new number.
						elog(LOG, "hb_worker: NOTIFY HB_CL,'4 Hit! Conguratulatoins!, next new game.'\n");
						strcpy(notify_buf, "NOTIFY HB_CL,'4 Hit! Conguratulatoins!, next new game.'");
						PQexec(conn, notify_buf);
						create_random_number(number);
						elog(LOG, "hb_worker: set secret number=%s\n", number);

					} else {
						// Notify Hit&blow
						elog(LOG, "NOTIFY HB_CL,'%d Hit / %d Blow.'",
							GET_HITS(result), GET_BLOWS(result));
						sprintf(notify_buf, "NOTIFY HB_CL,'%d Hit / %d Blow.'",
							GET_HITS(result), GET_BLOWS(result));
						PQexec(conn, notify_buf);
					}
					break;
				case START_COMMAND:
					// Set New number.
					elog(LOG, "hb_worker: Set New number.");
					create_random_number(number);
					break;
				case QUIT_COMMAND:
					// nop
					break;
				case INVALID_COMMAND:
				default:
					// NOTIFY error status
					sprintf(notify_buf, "hb_worker: NOTIFY HB_CL,'Invalid data.(%s)'", notify->extra);
					PQexec(conn, notify_buf);
					break;
			}
			

			PQfreemem(notify);
			nnotifies++;
		}
	}

	elog(LOG, "Done.\n");

	/* close the connection to the database and cleanup */
	PQfinish(conn);

	return 0;
}
