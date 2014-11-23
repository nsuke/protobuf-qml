{
  'variables': {
    'variables': {
      'protobuf_static_link%': 0,
      'protobuf_include_dir%': '>!(pkg-config --variable=includedir protobuf)',
      'protobuf_lib_dir%': '>!(pkg-config --variable=libdir protobuf)',
      'protoc_path%': '>!(pkg-config --variable=exec_prefix protobuf)/bin/protoc',

      'qt_include_dir%': '>!(pkg-config --variable=includedir Qt5Core)',
      'qt_lib_dir%': '>!(pkg-config --variable=libdir Qt5Core)',
      'qt_bin_dir%': '>!(pkg-config --variable=host_bins Qt5Core)',
    },
    'clang%': 1,
    'protobuf_static_link%': '<(protobuf_static_link)',
    'protobuf_include_dir%': '<(protobuf_include_dir)',
    'protobuf_lib_dir%': '<(protobuf_lib_dir)',
    'protoc_path%': '<(protoc_path)',

    'qt_include_dir%': '<(qt_include_dir)',
    'qt_lib_dir%': '<(qt_lib_dir)',
    'qt_bin_dir%': '<(qt_bin_dir)',

    'qpb_root%': '<(DEPTH)',
  },
  'conditions': [
    ['clang!=0', {
      'make_global_settings': [
        ['CC', '<!(which clang)'],
        ['CXX', '<!(which clang++)'],
        ['CC.host', '$(CC)'],
        ['CXX.host', '$(CXX)'],
      ],
      'target_defaults': {
        'cflags': [
          '-fdiagnostics-color',
        ],
      },
    }],
  ],
  'target_defaults': {
    'defines': [
      '_REENTRANT',
    ],
    'cflags': [
      '-Wall',
      '-Wextra',
      '-Wno-unused-parameter',
      '-fPIC',
      '-fstack-protector-strong',
      '--param=ssp-buffer-size=4',
      '-pipe',
    ],
    'cflags_cc': [
      '-std=c++11',
      '-frtti',
    ],
    'ldflags': [
      '-Wl,--as-needed',
      '-Wl,-z,relro',
      '-pthread',
    ],
    'variables': {
      'qt_moc%': 0,
      'qt_moc_out%': '>(INTERMEDIATE_DIR)/moc_out',
    },
    'target_conditions': [
      ['qt_moc!=0', {
        'rules': [{
          'rule_name': 'apply_moc',
          'extension': '.h',
          'action': [
            'python',
            '<(qpb_root)/build/moc_wrapper.py',
            '--moc', '<(qt_bin_dir)/moc',
            '--includes', '>(_include_dirs)',
            '--defines', '>(_defines)',
            '--out', '>(qt_moc_out)/moc_<(RULE_INPUT_ROOT).cpp',
            '<(RULE_INPUT_PATH)',
          ],
          'inputs': [
            '<(qt_bin_dir)/moc',
          ],
          'outputs': [
            '>(qt_moc_out)/moc_<(RULE_INPUT_ROOT).cpp',
          ],
          'process_outputs_as_sources': 1,
        }],
      }],
    ],
    'configurations': {
      'Debug': {
        'defines': [
          '_DEBUG',
        ],
        'cflags': [
          '-ggdb3',
          '-O0',
        ],
      },
      'Release': {
        'defines': [
          'NDEBUG',
          'QT_NO_DEBUG',
        ],
        'cflags': [
          '-g0',
          '-O2',
        ],
        'ldflags': [
          '-Wl,-O2',
        ],
      },
    },
    'default_configuration': 'Debug',
  },
}
