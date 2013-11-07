hb_worker
=========

hb_worker - Number guessing for background worker process

[ Parameters ]
hb_worker.conn connection string ( default : "postgres")

[ Start-up ]
- Add the following entry to the postgresql.conf.

shared_preload_libraries = 'hb_woker'
hb_worker.conn = connection string

- Connection string to specify the connection string for libpq .

[ How to use]

- Connect to the database that you specified in the connection string in any client .
  (The following shows an example of executing in psql)

- listen for HB_CL channel LISTEN command .

$ LISTEN HB_CL;

- send the number of 4-digit NOTIFY command to HB_SV channel .

$ NOTIFY HB_SV, '1234 ';

- receive the judgment number of messages that you just supplied with the command after the next .

- If you have made ??4 Hit, displays a message congraturations ,
  I generates a new secret number of the background process.

(ex.)
postgres=# NOTIFY HB_SV,'6315';
NOTIFY
Asynchronous notification "hb_cl" with payload "4 Hit! Conguratulatoins!, next
new game." received from server process with PID 17453.

