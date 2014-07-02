require 'rake'
require 'rake/clean'

CC          = "clang"
PKGS        = "alsa libspotify ncurses"
CFLAGS      = "-std=gnu99 -ggdb -Wall"
LDFLAGS     = `pkg-config --libs #{PKGS}`.strip << " -lpthread"

TARGET      = "spoticli"
SOURCE_DIR  = "src"
OBJECT_DIR  = "build"

SOURCE_FILES = FileList.new("#{SOURCE_DIR}/**/*.c")

directory OBJECT_DIR

task :default => "build:target"

namespace :build do
    task :deps do

    end

    task :objects do
        SOURCE_FILES.each do |source|
            # replace source dir with object dir
            object = source.gsub(/^#{SOURCE_DIR}/, "#{OBJECT_DIR}")
            object = object.sub(/\.c$/, '.o')

            # create directory
            mkdir_p object.pathmap("%d").strip

            # compile source
            sh "#{CC} #{CFLAGS} -I./#{SOURCE_DIR} -c -o #{object} #{source}"
        end
    end
    CLEAN.include('**/*.o', 'build')

    task :target => :objects do
        # find all object files in build
        object_files = FileList["#{OBJECT_DIR}/**/*.o"].join(' ')

        # link
        sh "#{CC} #{object_files} #{LDFLAGS} -o #{TARGET}"
    end
    CLOBBER.include("#{TARGET}")
end
