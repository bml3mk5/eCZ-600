# SHARP X68000 Emulator Windows(VC++) Edition

#### Copyright(C) Common Source Code Project, Sasaji 2011-2025 All Rights Reserved.

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
        gui/ ............... GUI関連ソース
          windows/ ......... Windows GUI関連ソース
        osd/ ............... OS依存関連ソース
          windows/ ......... Windows依存関連ソース
        res/ ............... リソースファイル
        video/ ............. 録画用関連ソース
          windows/ ......... windows関連ソース
        vm/ ................ VMメインプログラムソース
      VC++2010/
        x68000.vcxproj ..... Visual Studio プロジェクトファイル
      VC++2013/
        x68000.vcxproj ..... Visual Studio プロジェクトファイル
      VC++2015/
        x68000.vcxproj ..... Visual Studio プロジェクトファイル
      VC++2017/
        x68000.vcxproj ..... Visual Studio プロジェクトファイル
      VC++2019/
        x68000.vcxproj ..... Visual Studio プロジェクトファイル
      README_WIN.md ........ このファイル

## コンパイル方法

  Visual Studioからvcxprojファイルを開いてビルドしてください。


## 免責事項

* このソフトはフリーウェアです。ただし、著作権は放棄しておりません。
  実行モジュールについては作者Sasajiにあります。
  ソースコードについてはそれぞれの作者にあります。
* このソフトによって発生したいかなる損害についても著作権者は一切責任を負いません。
  このソフトを使用するにあたってはすべて自己責任で行ってください。
* 雑誌やネットなどに転載される場合、不特定多数の方に再配布を行う場合でも
  承諾の必要はありませんが、転載の旨をご連絡いただけたら幸いです。


------------------------------------------------------------------------------

連絡先：Sasaji (sasaji@s-sasaji.ddo.jp)
 * My WebPage: http://s-sasaji.ddo.jp/bml3mk5/
 * GitHub:     https://github.com/bml3mk5/eCZ-600
 * X(Twitter): https://x.com/bml3mk5

------------------------------------------------------------------------------

