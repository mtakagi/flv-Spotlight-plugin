require 'rake/clean'
require 'rake/packagetask'

# バージョンナンバー
PLUGIN_VERSION = "0.0.1" #IO.readlines("Info.plist")[0].match(/(\d|\.)+/)

# rake clean で build ディレクトリ以下を削除
CLEAN.include("build")
CLEAN.include("zip")

# rake package で zip ファイルを作る
package = Rake::PackageTask.new("flv", PLUGIN_VERSION) do |p|
	p.package_dir = "./zip"
	p.package_files.include("flv.mdimporter")
	p.package_files.exclude(".DS_Store")
	p.need_zip = true
end

desc "Install plugin"

task :install do
	system('xcodebuild -configuration Release -target "Copy plugin"')
end

desc "Build Spotlight plugin"

task :default do
	system('xcodebuild -configuration Release -target "flv"')
end

# Copy dat.qlgenerator
file "flv.mdimporter" do
	if !File.exist?("build/Release/flv.mdimporter") # dat.qlgenerator がない場合はビルドする
		puts "File not exist. Build Start."
		Rake::Task[:default].invoke()
	end
	
	sh "mkdir -p #{package.package_dir_path}"
	sh "cp -R build/Release/flv.mdimporter #{package.package_dir_path}"
#	sh "cp *.txt #{package.package_dir_path}"
end