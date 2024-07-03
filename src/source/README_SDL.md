# SHARP X68000 Emulator SDL edition

#### Copyright(C) Common Source Code Project, Sasaji 2012-2024 All Rights Reserved.

## ファイル構成

    docs/ .................. ドキュメント
    source/
      include/ ............. インクルードファイル
      lib/ ................. ライブラリファイル
      locale/ .............. 翻訳用(gettext)
        list.xml ........... 言語一覧
        ja/
          LC_MESSAGES/
            x68000.po ...... 日本語翻訳ファイル
            x68000.mo ...... 日本語翻訳ファイル(変換済み)
      src/ ................. ソースファイル
        extra/ ............. その他ソース
        gui/ ............... GUI関連ソース
          agar/ ............ Agar関連ソース
          cocoa/ ........... Mac Cocoa関連ソース
          gtk_x11/ ......... Gtk+関連ソース
          windows/ ......... Windows GUI関連ソース
        osd/ ............... OS依存関連ソース
          gtk/ ............. Gtk+依存関連ソース
          SDL/ ............. SDL依存関連ソース
        res/ ............... リソースファイル
        video/ ............. 録画用関連ソース
          cocoa/ ........... Cocoa(mac用)関連ソース
          libpng/ .......... LibPNG関連ソース
          windows/ ......... windows関連ソース
        vm/ ................ VMメインプログラムソース
      patch/ ............... パッチファイル
        SDL-1.2.15-mac-keyboard.patch ...
                             SDL macでfnキーを使えるようにするパッチファイル
        SDL-2.0.8-mac-keyboard.patch ...
                             SDL2 macでfnキーを使えるようにするパッチファイル
      Eclipse/ ............. Eclipseプロジェクトファイル
        sdl_linux/ ......... SDL linux用
        sdl_win/ ........... SDL Pleiades(Eclipse日本語版)用
      VC++2010/
        x68000_sdl.vcxproj .. SDL版 Visual Studio プロジェクトファイル
      VC++2013/
        x68000_sdl.vcxproj .. SDL版 Visual Studio プロジェクトファイル
      VC++2015/
        x68000_sdl.vcxproj .. SDL版 Visual Studio プロジェクトファイル
      VC++2017/
        x68000_sdl.vcxproj .. SDL版 Visual Studio プロジェクトファイル
      VC++2019/
        x68000_sdl.vcxproj .. SDL版 Visual Studio プロジェクトファイル
      Xcode/ ............... Xcode用プロジェクトファイル
      Makefile.xxx ......... 各OSごとのmakeファイル
      README_SDL.md ........ このファイル


## コンパイル方法


----------------------------------------
### MacOSX版 ###

* 以下のバージョンがあります。

 + SDL2 + Cocoa版 -> Makefile.mac_cocoa2, Makefile.mac_cocoa2_dbgr


#### 1. 開発環境の構築

 * Xcode を インストールします。
   (Command Line Tools for Xcode は自動的にインストールされるようだ。)

 * コンパイルに必要なライブラリをインストールします。
   ターミナル上で行います。(Xcodeは使用しません。)

  + SDL2-2.28.5

    1. パッチを適用します。

           patch -p 1 < SDL2-2.28.5-mac-keyboard.patch

    2. ビルド＆インストール

           ./configure
           make
           make install

  + SDL2_ttf-2.20.2

        ./configure
        make
        make install


#### 2. コンパイル（コマンドラインを使用する場合）

 ターミナル上で行います。

 * sharedなバイナリを作成する場合

       make -f Makefile.xxx clean
       make -f Makefile.xxx install

  + installは、makeを実行したディレクトリの上にReleaseSHディレクトリを作成し、
  そこに必要なファイルをコピーします。
  (/usr/localにコピーはしません。)
  + インストール先を変更するには、Makefile中にある、SH_INSTALLDIRを変更
     してください。

 * staticなバイナリを作成する場合

       make -f Makefile.xxx st_clean
       make -f Makefile.xxx st_install

  + インストール先を変更するには、Makefile中にある、ST_INSTALLDIRを変更
    してください。


#### 3. コンパイル（Xcodeを使用する場合）

 * Xcodeフォルダにあるプロジェクトを開く。

 * ヘッダファイルやライブラリが/usr/local以外にあるときは
   BuildSettings -> Search Paths にある
   Header Search Paths と Library Search Paths を設定してください。


----------------------------------------
### Linux版 ###

* 以下のバージョンがあります。

 + GTK+3 + SDL2版 -> Makefile.linux_gtk_sdl2, Makefile.linux_gtk_sdl2_dbgr


#### 1. 開発環境の構築

 * ディストリビューションに付属する開発用ライブラリをインストール。

  + DebianやUbuntu: Synapticパッケージマネージャもしくはaptでインストール
  + RedhatやFedora: Yumなどでインストール

  + 必要なライブラリ
   - コンパイラ: gcc, g++, make
   - 画面系: gtk-3.0-dev, libsdl2-dev, libsdl2-ttf-dev
   - サウンド系: libalsa-dev

 * 上記ライブラリがない場合はソースコードからインストールします。

  + SDL2-2.28.5

          ./configure
          make
          make install

  + SDL2_ttf-2.20.2

          ./configure
          make
          make install


#### 2. コンパイル

 ターミナル(端末)上で行います。

 * sharedなバイナリを作成する場合
  - ライブラリをパッケージからインストールしている場合はこちらでビルド。

         make -f Makefile.xxx clean
         make -f Makefile.xxx install

  * installは、makeを実行したディレクトリの上にReleaseディレクトリを作成し、
    そこに必要なファイルをコピーします。
  (/usr/localにコピーはしません。)
  * インストール先を変更するには、Makefile中にある、SH_INSTALLDIRを変更
    してください。

 * staticなバイナリを作成する場合

         make -f Makefile.xxx st_clean
         make -f Makefile.xxx st_install

  * インストール先を変更するには、Makefile中にある、ST_INSTALLDIRを変更
    してください。


----------------------------------------
### MinGW + MSYS (Windows)版 ###

* 以下のバージョンがあります。

 + SDL2 + Win GUI版 -> Makefile.win_gui2, Makefile.win_gui2_dbgr

#### 1. 開発環境の構築

 * MinGWをインストール

  + インストーラに従ってインストールします。

  + C Compiler, C++ Compiler, MSYS Basic SYSTEM, MSYS Developer Toolkit
  をチェックしてインストール。
    インターネットから必要なモジュールがダウンロードされる。

 * MinGW Shellを起動してコンパイルに必要なライブラリをインストールします。

  + SDL2-2.28.5

   - Development Librariesかソースからインストール

   - ソースからインストールする場合

          ./configure
          make
          make install

  + SDL2_ttf-2.20.2

   - ソースからインストール

          ./configure
          make
          make install


#### 2. コンパイル

 MinGW Shell上で行います。

 * 必要に応じてMakefile.xxxを変更します。

  * MinGWのバージョンが異なる場合、GCCLIBDIRを修正。
  * SDLLIBSのパスを修正。
    SDL, SDL_ttfなどをバイナリパッケージでインストールした場合、
    インストール先が/usr/local/cross-tools/i386-mingw32/配下になるため

 * sharedなバイナリを作成する場合

       make -f Makefile.xxx clean
       make -f Makefile.xxx install

  * installは、makeを実行したディレクトリの上にReleaseディレクトリを作成し、
  そこに必要なファイルをコピーします。
  (/usr/localにコピーはしません。)
  * インストール先を変更するには、Makefile中にある、SH_INSTALLDIRを変更
     してください。

 * staticなバイナリを作成する場合

       make -f Makefile.xxx st_clean
       make -f Makefile.xxx st_install

  * インストール先を変更するには、Makefile中にある、ST_INSTALLDIRを変更
     してください。


----------------------------------------
### VC++(Windows)版 ###

#### 1. 開発環境の構築

* 必要なライブラリをVC++でビルドします。

 + SDL2-2.28.5

    ソースからインストール
      VisualCフォルダにあるSDL.slnを使用してビルド。
      出来たdll,libはlib/Release/x86にコピーしておく。

 + SDL2_ttf-2.20.2

    ソースからインストール
      VisualCフォルダにあるSDL_ttf.slnを使用してビルド。


#### 2. コンパイル

 * プロジェクトファイル(*_sdl.vcxproj)を使用してビルド。
  + 表示→プロパティマネージャを開き、Release下を開く。
  + ユーザーマクロに設定しているパスを変更する。


----------------------------------------
## 免責事項

* このソフトはフリーウェアです。ただし、著作権は放棄しておりません。
  実行モジュールについては作者Sasajiにあります。
  ソースコードについてはそれぞれの作者にあります。
* このソフトによって発生したいかなる損害についても著作権者は一切責任を負いません。
  このソフトを使用するにあたってはすべて自己責任で行ってください。
* 雑誌やネットなどに転載される場合、不特定多数の方に再配布を行う場合でも
  承諾の必要はありませんが、転載の旨をご連絡いただけたら幸いです。

==============================================================================

連絡先：Sasaji (sasaji@s-sasaji.ddo.jp)
 * My WebPage: http://s-sasaji.ddo.jp/bml3mk5/
 * GitHub:     https://github.com/bml3mk5/eCZ-600
 * X(Twitter): https://x.com/bml3mk5

==============================================================================

