project('c7zip', 'cpp', 'c')
fs = import('fs')

glob = find_program('meson-glob')

source_dir = meson.current_source_dir()
build_dir = meson.current_build_dir()
include_dirs = include_directories('.')
compile_args = [
    '-I' + (source_dir / '..'),  # For <mlib/m-string.h>
    '-D_REENTRANT',
    '-D_FILE_OFFSET_BITS=64',
    '-D_LARGEFILE_SOURCE',
]

sources = ['c7zip.cpp', 'c7zip.h']
headgen = []
sevenzip_sources = []

sevenzip_dir = source_dir / '7zip'
sevenzip_glob = []
sevenzip_glob += run_command(glob, sevenzip_dir, 'C/*.c', check: true).stdout().split('\n')
sevenzip_glob += run_command(glob, sevenzip_dir, 'CPP/7zip/Archive/*/*.cpp', check: true).stdout().split('\n')
sevenzip_glob += run_command(glob, sevenzip_dir, 'CPP/7zip/Archive/*.cpp', check: true).stdout().split('\n')
sevenzip_glob += run_command(glob, sevenzip_dir, 'CPP/7zip/Common/*.cpp', check: true).stdout().split('\n')
sevenzip_glob += run_command(glob, sevenzip_dir, 'CPP/7zip/Compress/*.cpp', check: true).stdout().split('\n')
sevenzip_glob += run_command(glob, sevenzip_dir, 'CPP/7zip/Crypto/*.cpp', check: true).stdout().split('\n')
sevenzip_glob += run_command(glob, sevenzip_dir, 'CPP/Common/*.cpp', check: true).stdout().split('\n')
sevenzip_glob += [
    sevenzip_dir / 'CPP/Windows/ErrorMsg.cpp',
    sevenzip_dir / 'CPP/Windows/FileDir.cpp',
    sevenzip_dir / 'CPP/Windows/FileFind.cpp',
    sevenzip_dir / 'CPP/Windows/FileIO.cpp',
    sevenzip_dir / 'CPP/Windows/FileLink.cpp',
    sevenzip_dir / 'CPP/Windows/FileName.cpp',
    sevenzip_dir / 'CPP/Windows/PropVariant.cpp',
    sevenzip_dir / 'CPP/Windows/PropVariantConv.cpp',
    sevenzip_dir / 'CPP/Windows/PropVariantUtils.cpp',
    sevenzip_dir / 'CPP/Windows/Synchronization.cpp',
    sevenzip_dir / 'CPP/Windows/System.cpp',
    sevenzip_dir / 'CPP/Windows/SystemInfo.cpp',
    sevenzip_dir / 'CPP/Windows/TimeUtils.cpp',
]
sevenzip_glob += [
    sevenzip_dir / 'CPP/7zip/UI/Common/ArchiveExtractCallback.cpp',
    sevenzip_dir / 'CPP/7zip/UI/Common/ArchiveOpenCallback.cpp',
    sevenzip_dir / 'CPP/7zip/UI/Common/DefaultName.cpp',
    sevenzip_dir / 'CPP/7zip/UI/Common/EnumDirItems.cpp',
    sevenzip_dir / 'CPP/7zip/UI/Common/Extract.cpp',
    sevenzip_dir / 'CPP/7zip/UI/Common/ExtractingFilePath.cpp',
    sevenzip_dir / 'CPP/7zip/UI/Common/HashCalc.cpp',
    sevenzip_dir / 'CPP/7zip/UI/Common/LoadCodecs.cpp',
    sevenzip_dir / 'CPP/7zip/UI/Common/OpenArchive.cpp',
    sevenzip_dir / 'CPP/7zip/UI/Common/PropIDUtils.cpp',
    sevenzip_dir / 'CPP/7zip/UI/Common/SetProperties.cpp',
    sevenzip_dir / 'CPP/7zip/UI/Common/SortUtils.cpp',
    sevenzip_dir / 'CPP/7zip/UI/Console/ConsoleClose.cpp',
    sevenzip_dir / 'CPP/7zip/UI/Console/ExtractCallbackConsole.cpp',
    sevenzip_dir / 'CPP/7zip/UI/Console/List.cpp',
    sevenzip_dir / 'CPP/7zip/UI/Console/OpenCallbackConsole.cpp',
    sevenzip_dir / 'CPP/7zip/UI/Console/PercentPrinter.cpp',
    sevenzip_dir / 'CPP/7zip/UI/Console/UserInputUtils.cpp',
]
foreach sevenzip_src : sevenzip_glob
    if fs.name(sevenzip_src).contains('Exports')
        continue
    endif
    sevenzip_sources += [sevenzip_src]
endforeach

c7zip = static_library(
    'c7zip', sources + sevenzip_sources,
    include_directories: include_dirs,
    cpp_args: compile_args,
    c_args: compile_args,
)

c7zip_dep = declare_dependency(
    link_whole: c7zip,  # Not link_with, needs to include all plugin object files
    sources: headgen,
    include_directories: include_dirs,
    compile_args: compile_args,
)
