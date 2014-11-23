{
  'includes': [
    'build/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'protobuf-qml',
      'type': 'loadable_module',
      'product_dir': '<(PRODUCT_DIR)/qml/Protobuf',
      'dependencies': [
        '<(qpb_root)/build/protobuf.gyp:protobuf',
        '<(qpb_root)/build/qt5.gyp:qt5',
      ],
      'defines': [
        'QPB_EXPORT',
        'QT_PLUGIN',
      ],
      'include_dirs': [
        '<(qpb_root)/lib',
      ],
      'sources': [
        '<(qpb_root)/lib/qpb/common.h',
        '<(qpb_root)/lib/qpb/io.h',
        '<(qpb_root)/lib/qpb/io.cpp',
        '<(qpb_root)/lib/qpb/memory.h',
        '<(qpb_root)/lib/qpb/memory.cpp',
        '<(qpb_root)/lib/qpb/plugin.h',
        '<(qpb_root)/lib/qpb/plugin.cpp',
        '<(qpb_root)/lib/qpb/wire_format_lite.h',
        '<(qpb_root)/lib/qpb/wire_format_lite.cpp',
        '<(qpb_root)/lib/qpb/descriptor_database.h',
        '<(qpb_root)/lib/qpb/descriptor_database.cpp',
      ],
      'copies': [{
        'destination': '<(PRODUCT_DIR)/qml/Protobuf',
        'files': [
          '<(qpb_root)/lib/qmldir',
          '<(qpb_root)/lib/message.js',
        ],
      }],
    },
    {
      'target_name': 'protoc-gen-qml',
      'type': 'executable',
      'dependencies': [
        '<(qpb_root)/build/protobuf.gyp:protoc',
        '<(qpb_root)/build/protobuf.gyp:protobuf',
        '<(qpb_root)/build/qt5.gyp:qt5',
      ],
      'include_dirs': [
        '<(qpb_root)/compiler',
      ],
      'sources': [
        '<(qpb_root)/compiler/qpb/qml_generator.cpp',
        '<(qpb_root)/compiler/qpb/qml_generator.h',
        '<(qpb_root)/compiler/qpb/main.cpp',
      ],
    },
    {
      'variables': {
        'qml_test_dir': '<(PRODUCT_DIR)/qml_test',
      },
      'target_name': 'qpb-qml-test',
      'type': 'none',
      'sources': [
        '<(qpb_root)/test/QpbTest.proto',
        '<(qpb_root)/test/QpbTest2.proto',
      ],
      'rules': [{
        'rule_name': 'protoc-gen',
        'extension': '.proto',
        'action': [
          'python',
          '<(qpb_root)/build/protoc_wrapper.py',
          '-p', '<(PRODUCT_DIR)/protoc-gen-qml',
          '-c', '<(protoc_path)',
          '-o', '<(qml_test_dir)',
          '<(RULE_INPUT_PATH)',
        ],
        'inputs': [
          '<(qpb_root)/build/protoc_wrapper.py',
          '<(PRODUCT_DIR)/protoc-gen-qml',
          '<(protoc_path)',
        ],
        'outputs': [
          '<(qml_test_dir)/<(RULE_INPUT_ROOT).pb.js',
        ],
      }],
      'actions': [{
        'action_name': 'generate_runner',
        'action': [
          'python',
          '<(DEPTH)/build/generate_test_runner.py',
          '<(qt_bin_dir)/qmltestrunner',
          '<(DEPTH)',
          '<(PRODUCT_DIR)/qml',
          '<(PRODUCT_DIR)/qml_test',
          '<(PRODUCT_DIR)/run_tests.py',
          '-L',
          '<(protobuf_lib_dir)',
          '<(qt_lib_dir)',
        ],
        'inputs': [
          '<(DEPTH)/build/generate_test_runner.py',
          '<(qt_bin_dir)/qmltestrunner',
        ],
        'outputs': [
          '<(PRODUCT_DIR)/srun_tests.py',
        ],
      }],
      'copies': [{
        'destination': '<(qml_test_dir)',
        'files': [
          '<(qpb_root)/test/tst_qbs.qml',
        ],
      }],
    },
  ],
}
