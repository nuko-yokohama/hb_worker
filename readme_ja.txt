数当てゲーム用バックグラウンドワーカプロセス

【パラメータ】
hb_worker.conn 接続文字列(デフォルト： "postgres")

【起動】
・postgresql.confに以下のエントリを追加。

  shared_preload_libraries = 'hb_woker'
  hb_worker.conn = 接続文字列

  接続文字列はlibpq用の接続文字列を指定する。

【使い方】
・任意のクライアントで接続文字列で指定したデータベースに接続する。
　(以下はpsqlでの実行例を示す）

・LISTENコマンドで HB_CL チャネルを待ち受ける。

$ LISTEN HB_CL;

・HB_SV チャネルにNOTIFYコマンドで4桁の数を送信する。

$ NOTIFY HB_SV,'1234';

・次のコマンド実行後に先ほど指定した数の判定メッセージを受け取る。
  (ex.)
postgres=# NOTIFY HB_SV,'6314';
NOTIFY
Asynchronous notification "hb_cl" with payload "3 Hit / 0 Blow." received from
server process with PID 17453.

・4 Hit になった場合、congraturationsメッセージを表示し、
　バックグラウンドプロセス内で新たな秘密数を生成する。
  (ex.)
postgres=# NOTIFY HB_SV,'6315';
NOTIFY
Asynchronous notification "hb_cl" with payload "4 Hit! Conguratulatoins!, next
new game." received from server process with PID 17453.

