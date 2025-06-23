project(
    'F95Checker', 'c',
    version: '12.0.0',
    license: 'GPL-3.0',
    meson_version: '>=1.2.0',
    default_options: {
        'backend': 'ninja',
        'default_library': 'static',
        'default_both_libraries': 'static',
        'prefer_static': true,
        'werror': true,
        'b_lto': false,  # FIXME: enable in release builds?
        'b_ndebug': 'if-release',
        'c_std': 'gnu23',
        'cpp_std': 'c++23',
    },
)
cmake = import('cmake')
fs = import('fs')
python = import('python')

source_dir = meson.current_source_dir()
build_dir = meson.current_build_dir()
include_dirs = include_directories('src')
compile_args = [
    '-I' + (source_dir / 'subprojects'),
    '-I' + (build_dir / 'subprojects'),
    '-DF95CHECKER_VERSION="' + meson.project_version() + '"',
    '-Wall', '-Wextra',
    '-Wstrict-prototypes',
    '-Wredundant-decls',
    '-Wundef',
    '-Wunused',
]
linking_args = [
    '-static-libgcc',
    '-static-libstdc++',
]
dependencies = []

add_global_arguments(
    '-Wno-unused',
    '-fdiagnostics-color=always',
    language: ['c', 'cpp']
)
add_global_link_arguments(
    '-fdiagnostics-color=always',
    language: ['c', 'cpp']
)

assert(
    meson.get_compiler('c').get_id() == 'clang',
    'This project only support the Clang compiler!',
)

# Python (needed for codegen)
python_exe = python.find_installation('python3').full_path()
python_requirements_files = [
    'subprojects/icons/requirements.txt',
    'subprojects/dcimgui/dear_bindings/requirements.txt',
]
if get_option('python_venv')
    if not fs.is_dir(build_dir / 'python_venv')
        message('Preparing python venv')
        run_command(
            python_exe,
            '-m', 'venv',
            build_dir / 'python_venv',
            check: true,
        )
    endif
    python_exe = build_dir / 'python_venv/bin/python'
    foreach python_requirements_file : python_requirements_files
        message('Preparing python requirements ' + python_requirements_file)
        run_command(
            python_exe,
            '-m', 'pip',
            '--disable-pip-version-check',
            'install', '-U',
            '-r', source_dir / python_requirements_file,
            check: true,
        )
    endforeach
endif
meson.override_find_program('python3-maybe-venv', files(python_exe))

# Helper for finding source files by globbing at configure time
glob = build_dir / 'meson-glob.py'
glob_script = (
    '#!/usr/bin/env python3\n'+
    'import pathlib, sys;'+
    'root = pathlib.Path(sys.argv[1]);'+
    'glob = root.glob(sys.argv[2]);'+
    'print("\\n".join(str(p.absolute()) for p in glob), end="");'
)
run_command(
    python_exe, '-c',
    'import pathlib, stat, sys;'+
    'script = pathlib.Path(sys.argv[1]);'+
    'script.write_text(sys.argv[2]);'+
    'script.chmod(script.stat().st_mode | stat.S_IEXEC);',
    glob, glob_script,
    check: true,
)
meson.override_find_program('meson-glob', files(glob))

# Setup subprojects
cmake_opts = {
    'BUILD_EXAMPLES': false,
    'BUILD_SHARED_LIBS': false,
    'BUILD_STATIC_LIBS': true,
    'BUILD_TESTING': false,
    'CMAKE_SUPPRESS_DEVELOPER_WARNINGS': true,
}
dummy_dep = dependency('', required: false)

# bc7e
bc7e_proj = subproject('bc7e')
bc7e = bc7e_proj.get_variable('bc7e_dep', dummy_dep)
dependencies += [bc7e]
compile_args += [
    '-DBC7E_AVAILABLE=' + bc7e.found().to_int().to_string()
]

# c7zip (basic C wrapper for 7-Zip)
c7zip_proj = subproject('c7zip')
c7zip = c7zip_proj.get_variable('c7zip_dep')
dependencies += [c7zip]

# cURL
if get_option('vendored_curl')
    curl_opts = cmake.subproject_options()
    curl_opts.add_cmake_defines(cmake_opts + {
        'BUILD_CURL_EXE': false,
        'BUILD_LIBCURL_DOCS': false,
        'BUILD_MISC_DOCS': false,
        'BUILD_STATIC_CURL': true,
        'CURL_BROTLI': false,
        'CURL_DISABLE_INSTALL': true,
        'CURL_USE_LIBPSL': false,
        'CURL_USE_LIBSSH2': false,
        'CURL_ZLIB': false,
        'CURL_ZSTD': false,
        'ENABLE_CURL_MANUAL': false,
        'HTTP_ONLY': true,
        'PERL_EXECUTABLE': false,
        'USE_LIBIDN2': false,
        'USE_NGHTTP2': false,
    })
    if get_option('vendored_wolfssl')
        curl_opts.add_cmake_defines({
            'CURL_USE_OPENSSL': false,
            'CURL_USE_WOLFSSL': true,
        })
        curl_opts.append_compile_args('c', '-I' + (build_dir / 'subprojects/wolfssl/__CMake_build'))
    elif build_machine.system() == 'darwin'
         wolfssl_opts.add_cmake_defines({
            'CURL_USE_SECTRANSP': true,
        })
    elif build_machine.system() == 'windows'
         wolfssl_opts.add_cmake_defines({
            'CURL_USE_SCHANNEL': true,
        })
    endif
    curl_proj = cmake.subproject('curl', options: curl_opts)
    alias_target('curl', curl_proj.target('libcurl_static'))
    curl = curl_proj.dependency('libcurl_static')
else
    curl = meson.get_compiler('c').find_library('curl')
endif
dependencies += [curl]

# cwalk
cwalk_proj = subproject('cwalk')
cwalk = cwalk_proj.get_variable('cwalk_dep')
dependencies += [cwalk]

# dcimgui (ImGui via dear_bindings)
dcimgui_proj = subproject('dcimgui')
dcimgui = dcimgui_proj.get_variable('dcimgui_dep')
dependencies += [dcimgui]

# Fonts (generate embedded C sources)
fonts_proj = subproject('fonts')
fonts = fonts_proj.get_variable('fonts_dep')
dependencies += [fonts]

# Icons (generate embedded C sources)
icons_proj = subproject('icons')
icons = icons_proj.get_variable('icons_dep')
dependencies += [icons]

# json-c
jsonc_opts = cmake.subproject_options()
jsonc_opts.add_cmake_defines(cmake_opts + {
    'BUILD_APPS': false,
})
jsonc_proj = cmake.subproject('json-c', options: jsonc_opts)
alias_target('jsonc', jsonc_proj.target('json-c'))
jsonc = jsonc_proj.dependency('json-c')
dependencies += [jsonc]

# M*LIB
compile_args += [
    '-DM_USE_SMALL_NAME=0',  # Disable legacy type/function names
]

# SDL3
if get_option('vendored_sdl3')
    sdl3_opts = cmake.subproject_options()
    sdl3_opts.add_cmake_defines(cmake_opts + {
        'SDL_AUDIO': false,
        'SDL_CAMERA': false,
        'SDL_DIALOG': false,
        'SDL_EXAMPLES': false,
        'SDL_GPU': false,
        'SDL_HAPTIC': false,
        'SDL_INSTALL': false,
        'SDL_POWER': false,
        'SDL_RENDER': true,
        'SDL_SENSOR': false,
        'SDL_STATIC': true,
        'SDL_TEST_LIBRARY': false,
        'SDL_TESTS': false,
        'SDL_UNINSTALL': false,
    })
    sdl3_opts.append_compile_args('c', '-Wno-documentation-unknown-command')
    sdl3_proj = cmake.subproject('sdl3', options: sdl3_opts)
    alias_target('sdl3', sdl3_proj.target('SDL3-static'))
    sdl3 = sdl3_proj.dependency('SDL3-static')
else
    sdl3 = meson.get_compiler('c').find_library('SDL3')
endif
dependencies += [sdl3]

# shlex (port from Python to C++ and wrapped to C)
shlex_proj = subproject('shlex')
shlex = shlex_proj.get_variable('shlex_dep')
dependencies += [shlex]

# SQLite3
if get_option('vendored_sqlite3')
    sqlite3_proj = subproject('sqlite3')
    sqlite3 = sqlite3_proj.get_variable('sqlite3_dep')
else
    sqlite3 = meson.get_compiler('c').find_library('sqlite3')
endif
dependencies += [sqlite3]

# wolfSSL
if get_option('vendored_wolfssl')
    wolfssl_opts = cmake.subproject_options()
    wolfssl_opts.add_cmake_defines(cmake_opts + {
        'WOLFSSL_CRYPT_TESTS': false,
        'WOLFSSL_CURL': true,
        'WOLFSSL_EXAMPLES': false,
    })
    wolfssl_proj = cmake.subproject('wolfssl', options: wolfssl_opts)
    wolfssl = wolfssl_proj.dependency('wolfssl')
else
    wolfssl = meson.get_compiler('c').find_library('wolfssl')
endif
dependencies += [wolfssl]

sources = run_command(glob, source_dir, 'src/**/*.[ch]', check: true).stdout().split('\n')

f95checker = executable(
    'F95Checker', sources,
    include_directories: include_dirs,
    dependencies: dependencies,
    c_args: compile_args,
    link_args: linking_args,
)

custom_target(
    'run',
    output: 'run',
    command: [
        f95checker,
    ],
    console: true,
)
