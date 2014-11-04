API
=================================
概要
---------------------------------
lpopupは `Lua <http://www.lua.org>`_ でアプリケーションを作成可能です。 ``lpopup`` モジュールはアプリケーション開発用のライブラリです。

文字コード
---------------------------------
lpopupは内部的にutf8で処理を行っています。そのためAPIでやりとりされる文字列は基本的にutf8が用いられます。ただし、 :lua:func:`lpopup.call` や :lua:func:`lpopup.capture` はコマンドが出力した文字コードのまま結果を返します。また、アプリ内に組み込まれている `LuaFileSystem <http://keplerproject.github.io/luafilesystem/>`_ の関数へ渡す文字列はOSネイティブの文字コードである必要があります。

ひな形
---------------------------------
以下にlpopupアプリケーションのひな形を示します。

    .. code-block:: lua

        local lpopup = require("lpopup")

        function lpopup_execute(argv)
          local root_menu = lpopup.root_menu()
          -- root_menu.add_item()
          root_menu.show(lpopup.SHOW_MOUSE, 0, 0)
        end

``lpopup.lua`` 内の ``lpopup.apps.ext`` や ``lpopup.apps.launcher`` も参考にしてください。

定数
---------------------------------

.. lua:attribute:: lpopup.SHOW_*

   :lua:func:`lpopup.Menu:show` で使用できる定数群です。

   - SHOW_MOUSE : マウスカーソルの位置
   - SHOW_COORD:  指定座標
   - SHOW_WINDOW: アクティブウインドウの中央
   - SHOW_CENTER: ウインドウの中央

.. lua:attribute:: lpopup.OS

   OSタイプを示す文字列です。

.. lua:attribute:: lpopup.PATHSEP

   パス区切り文字です。

関数
---------------------------------

外部コマンド
~~~~~~~~~~~~~~
.. lua:function:: lpopup.call(cmd [, options])

   外部コマンドを起動します。

   :param string cmd: 実行するコマンドと引数
   :param table  options: 実行オプション。以下のオプションが存在します。

       - ``workdir:string`` 実行ディレクトリ、デフォルトはコマンドと同じディレクトリ
       - ``async:bool`` 非同期実行するかどうか。非同期実行の場合、結果コードは受け取れません。デフォルト ``false``
       - ``hide:bool`` コマンドの画面を非表示Niするか。デフォルト ``false``
   :returns: {code:int, stdout:string, stderr:string, errmsg:string} 

       - ``code`` 終了コード
       - ``stdout`` 標準出力
       - ``stdout`` 標準エラー出力
       - ``errmsg`` エラーメッセージ

.. lua:function:: lpopup.shell_execute(cmd [, options])

   外部コマンドを起動します。 :lua:func:`lpopup.call` と異なりOSのシェル経由で実行されるため実行ファイル以外のファイルも起動可能です。

   :param string cmd: 実行するコマンドと引数
   :param table  options: 実行オプション。以下のオプションが存在します。
       - ``workdir:string`` 実行ディレクトリ、デフォルトはコマンドと同じディレクトリ
   :returns: なし

.. lua:function:: lpopup.capture(cmd [, options])

   :lua:func:`lpopup.call` を外部CUIアプリケーションの出力を取得する場合用にオプションを調整したショートカットです。

   :param string cmd: 実行するコマンドと引数
   :param table  options: :lua:func:`lpopup.call` と同じ

.. lua:function:: lpopup.launch(cmd [, options])

   :lua:func:`lpopup.call` を外部GUIアプリケーションを起動する場合用にオプションを調整したショートカットです。

   :param string cmd: 実行するコマンドと引数
   :param table  options: :lua:func:`lpopup.call` と同じ

文字コード/文字列
~~~~~~~~~~~~~~~~~~~~~
.. lua:function:: lpopup.utf82local(str)

   UTF8からOSネイティブの文字コードに変換します。

   :param string str: 変換対象文字列
   :returns: string:変換後文字列

.. lua:function:: lpopup.local2utf8(str)

   OSネイティブの文字コードからUTF8に変換します。

   :param string str: 変換対象文字列
   :returns: string:変換後文字列

.. lua:function:: lpopup.utf8iter(str)

   UTF8文字単位で文字列を返すイテレータを返します。

   :param string str: 対象文字列
   :returns: iterator:イテレータ

.. lua:function:: lpopup.tokenize(str)

   文字列を一般的なシェル規則でトークン化します。

   :param string str: 対象文字列
   :returns: table:トークンリスト

その他
~~~~~~~~~~~~~~

.. lua:function:: lpopup.root_menu()

   lpopupのルートメニューを返します。

   :returns: lpopup.Menu:ルートメニュー

.. lua:function:: lpopup.add_shortcutkey(menu, label)

   ``menu`` 内で ``label`` を使用する場合に、ショートカットキーを自動で設定します。

   :param lpopup.Menu menu: 対象メニュー
   :param string label: ラベル

クラス
---------------------------------

.. lua:class:: lpopup.Menu.new()

   新しいメニューを生成します。

.. lua:function:: lpopup.Menu:id()

   このメニューのIDを返します。

   :returns: int: ID

.. lua:function:: lpopup.Menu:add_item(label, callback [, options])

   このメニューにアイテムを追加します。

   :param string label: 表示名
   :param function callback: コールバック関数
   :param table options: オプション
   
       - ``icon:string`` アイコン文字列、デフォルトなし

.. lua:function:: lpopup.Menu:add_file_context(label, path [, options])

   このメニューにOSネイティブのコンテキストメニューを追加します。

   :param string label: 表示名
   :param string path:  対象ファイルのパス
   :param table options: オプション
   
       - ``icon:string`` アイコン文字列、デフォルトなし

.. lua:function:: lpopup.Menu:add_submenu(label, submenu [, options])

   このメニューにサブメニューを追加します。

   :param string label: 表示名
   :param lpopup.Menu submenu: サブメニュー
   :param table options: オプション
   
       - ``icon:string`` アイコン文字列、デフォルトなし

.. lua:function:: lpopup.Menu:add_hseparator()

   このメニューに水平セパレータを追加します。

.. lua:function:: lpopup.Menu:add_vseparator()

   このメニューに垂直セパレータを追加します。

.. lua:function:: lpopup.Menu:item_count()

   このメニューのアイテム数を返します。

   :returns: int:アイテム数

.. lua:function:: lpopup.Menu:show(type [,x ,y])

   メニューを表示します。

   :param int type: :lua:func:`lpopup.SHOW_*` の値
   :param int x: x座標
   :param int y: y座標

