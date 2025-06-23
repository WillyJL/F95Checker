project('dcimgui', 'cpp')

glob = find_program('meson-glob')
python_exe = find_program('python3-maybe-venv')

source_dir = meson.current_source_dir()
build_dir = meson.current_build_dir()
include_dirs = include_directories('.', 'imgui', 'imgui/backends')
compile_args = [
    '-DIMGUI_USER_CONFIG="dcimconfig.h"',
]

backends = ['sdl3', 'sdlrenderer3']
sources = []
headgen = []
imgui_sources = []

dear_bindings_cmd = [
    # No way to silence output with custom_target(), so wrap it with python
    python_exe, '-c',
    'import subprocess, sys;'+
    'subprocess.run(sys.argv[1:], stdout=subprocess.DEVNULL);',
    # Actual command here
    python_exe,
    source_dir / 'dear_bindings/dear_bindings.py',
    '--replace-prefix', 'cIm=Im',
    '--replace-prefix', 'CIM=IM',
    '--imconfig-path', source_dir / 'dcimconfig.h',
    '--emit-combined-json-metadata',
]

sources += custom_target(
    'dcimgui.cpp',
    output: ['dcimgui.cpp', 'dcimgui.h'],
    depend_files: ['imgui/imgui.h'],
    command: dear_bindings_cmd + [
        source_dir / 'imgui/imgui.h',
        '-o', build_dir / 'dcimgui',
    ],
)
sources += custom_target(
    'dcimgui_internal.cpp',
    output: ['dcimgui_internal.cpp', 'dcimgui_internal.h'],
    depend_files: ['imgui/imgui_internal.h', 'imgui/imgui.h'],
    command: dear_bindings_cmd + [
        '--include', source_dir / 'imgui/imgui.h',
        source_dir / 'imgui/imgui_internal.h',
        '-o', build_dir / 'dcimgui_internal',
    ],
)
imgui_sources += run_command(glob, source_dir, 'imgui/*.cpp', check: true).stdout().split('\n')

foreach backend : backends
    sources += custom_target(
        'dcimgui_impl_' + backend + '.cpp',
        output: ['dcimgui_impl_' + backend + '.cpp', 'dcimgui_impl_' + backend + '.h'],
        depends: [sources[0]],
        depend_files: ['imgui/backends/imgui_impl_' + backend + '.h', 'imgui/imgui.h'],
        command: dear_bindings_cmd + [
            '--backend',
            source_dir / 'imgui/backends/imgui_impl_' + backend + '.h',
            '-o', build_dir / 'dcimgui_impl_' + backend,
        ],
    )
    imgui_sources += ['imgui/backends/imgui_impl_' + backend + '.cpp']
endforeach

foreach source : sources
    foreach source_output : source.to_list()
        if source_output.full_path().endswith('.h')
            headgen += [source_output]
        endif
    endforeach
endforeach

dcimgui = static_library(
    'dcimgui', sources + imgui_sources,
    include_directories: include_dirs,
    cpp_args: compile_args,
)

dcimgui_dep = declare_dependency(
    link_with: dcimgui,
    sources: headgen,
    include_directories: include_dirs,
    compile_args: compile_args,
)

dcimgui_font_gen = executable('dcimgui_font_gen', 'imgui/misc/fonts/binary_to_compressed_c.cpp')
meson.override_find_program('dcimgui_font_gen', dcimgui_font_gen)
