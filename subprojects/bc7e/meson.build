project('bc7e', 'cpp')

ispc = find_program('ispc', required: false)
if not ispc.found()
    warning('ISPC not found, continuing without BC7 support!')
else
    source_dir = meson.current_source_dir()
    build_dir = meson.current_build_dir()
    include_dirs = include_directories()
    compile_args = []

    sources = []
    headgen = []
    targets = ['avx', 'avx2', 'sse2', 'sse4']
    objects = ['bc7e.o']
    headers = ['bc7e.h']
    foreach target : targets
        objects += ['bc7e_' + target + '.o']
        headers += ['bc7e_' + target + '.h']
    endforeach

    sources += custom_target(
        'bc7e.o',
        output: objects + headers,
        depend_files: ['bc7e_ispc/bc7e.ispc'],
        command: [
            ispc,
            '--PIC', '-g', '-O2', '--woff',
            '--target=' + ','.join(targets),
            '--opt=fast-math',
            '--opt=disable-assertions',
            source_dir / 'bc7e_ispc/bc7e.ispc',
            '-o', build_dir / objects[0],
            '-h', build_dir / headers[0],
        ],
    )

    foreach source : sources
        foreach source_output : source.to_list()
            if source_output.full_path().endswith('.h')
                headgen += [source_output]
            endif
        endforeach
    endforeach

    bc7e = static_library(
        'bc7e', sources,
        include_directories: include_dirs,
        cpp_args: compile_args,
    )

    bc7e_dep = declare_dependency(
        link_with: bc7e,
        sources: headgen,
        include_directories: include_dirs,
        compile_args: compile_args,
    )
endif
