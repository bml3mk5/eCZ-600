# SHARP X68000 Emulator 'eCZ-600'

#### Copyright(C) Common Source Code Project, Sasaji 2011-2023 All Rights Reserved.

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
          windows/ ......... Windows依存関連ソース
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
      tools/ ............... ツールファイル
      Eclipse/ ............. Eclipseプロジェクトファイル
        sdl_linux/ ......... SDL linux用
        sdl_win/ ........... SDL Pleiades(Eclipse日本語版)用
      VC++2010/
        x68000_sdl.vcxproj .. SDL版 VC++2010用プロジェクトファイル
        x68000.vcxproj ..... VC++2010用プロジェクトファイル
      VC++2019/
        x68000_sdl.vcxproj .. SDL版 VC++2019用プロジェクトファイル
        x68000.vcxproj ..... VC++2019用プロジェクトファイル
      Xcode/ ............... Xcode用プロジェクトファイル
      Makefile.xxx ......... 各OSごとのmakeファイル
      README_SDL.md ........ SDL版説明ファイル
      README_WIN.md ........ VC++版説明ファイル


## コンパイル方法

 * [Windows(VC++)版](src/source/README_WIN.md)

 * [SDL版](src/source/README_SDL.md)


## 免責事項

* このソフトはフリーウェアです。ただし、著作権は放棄しておりません。
  実行モジュールについては作者Sasajiにあります。
  ソースコードについてはそれぞれの作者にあります。
* このソフトによって発生したいかなる損害についても著作権者は一切責任を負いません。
  このソフトを使用するにあたってはすべて自己責任で行ってください。
* 雑誌やネットなどに転載される場合、不特定多数の方に再配布を行う場合でも
  承諾の必要はありませんが、転載の旨をご連絡いただけたら幸いです。


==============================================================================

連絡先：
  Sasaji (sasaji@s-sasaji.ddo.jp)
  http://s-sasaji.ddo.jp/bml3mk5/
  (Twitter: http://twitter.com/bml3mk5)

==============================================================================

