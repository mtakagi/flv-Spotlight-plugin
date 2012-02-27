# flv.mdimporter

[flv ファイル](http://ja.wikipedia.org/wiki/Flash_Video)用の Spotlight プラグインです。このプラグインをインストールすることで flv ファイルの情報(動画の縦横のサイズや再生時間など)が Finder などで確認できるようになります。(なるはず)  
![Screen shot](http://dl.dropbox.com/u/142785/screenshot/flv_finder_info.png "こんな感じ")  
現在のところ flv ファイルにある onMetaData の [AMF 形式](http://opensource.adobe.com/wiki/download/attachments/1114283/JP_amf3_spec_121207.pdf)のデータからメタデータを抽出しているだけのはずです。はずですになっているのはこのプラグインを作りかけで__半年くらい放置__していたため ~~flv ファイルのフォーマットの記憶も無ければ、ソースコードにコメントもないのであしからず~~。

## How to use

### 対応 OS とか
ぶっちゃけプロジェクトの設定が 10.7 の SDK を使用するようになっているので、そのままビルドした場合の対応 OS は Mac OS X 10.7 からになります。対応 CPU は設定が i386 と x86\_64 になっています。  
コードは基本的に [CoreFoundation](https://developer.apple.com/library/mac/#documentation/CoreFoundation/Reference/CoreFoundation_Collection/_index.html) の [CFReadStream](https://developer.apple.com/library/mac/#documentation/CoreFoundation/Reference/CFReadStreamRef/Reference/reference.html) を使用しているので、SDK さえあれば _10.4 からの対応_ にするのも _PPC に対応_ するのもそれほど難しくはないと思います。__がんばって!__

### Build
[プロジェクト](https://github.com/mtakagi/flv-Spotlight-plugin/downloads)をダウンロードするか `git clone` でリポジトリをローカルにクローンし、flv ディレクトリに移動し `rake` コマンドでビルドできます。あ、もちろん __[Developer Tools](http://itunes.apple.com/jp/app/xcode/id497799835?mt=12) のインストール__は必要ですよ!

### インストール
`~/Library/Spotlight/` ディレクトリにビルドしたファイルを移動するか、 `rake install` でビルド後にビルドしたファイルを移動できます。

### テスト
インストールしたプラグインはターミナルで

	mdimport -d 1 /path/to/flv
	Parse flv tags
	2012-02-25 23:46:13.217 mdimport[41020:707] Imported '/Volumes/Macintosh HD/Users/mtakagi/Desktop/第6回カーネル:VM勉強会 2.flv' of type 'com.adobe.flash.video' with plugIn /Users/mtakagi/Library/Spotlight/flv.mdimporter.

のように `mdimport` コマンドをデバッグレベルを指定して実行することで、ファイルからメタデータをインポートする際にデバッグ情報を出力しプラグインの挙動をチェックすることができます。  
`mdimport` の詳細はターミナルで `man` ページを参照してください。また ADC にも `mdimport` の `man` ページがあります。

* [mdimport(1) Mac OS X Manual Page](https://developer.apple.com/library/mac/#documentation/Darwin/Reference/ManPages/man1/mdimport.1.html)

### 免責事項とか
このソフトウェアの使用によって生じた損害に対して、一切責任負いかねますので予め御了承下さい。  
質問等は [@deprecated](http://twitter.com/deprecated) などにどうぞ。