{
  'includes': [
    '../build/common.gypi',
  ],
  'targets': [
    {
      'variables': {
        'qml_module_dir': '<(PRODUCT_DIR)/qml/Protobuf/Sandbox',
      },
      'target_name': 'qpb-sandbox',
      'type': 'loadable_module',
      'product_dir': '<(qml_module_dir)',
      'dependencies': [
        '<(qpb_root)/build/protobuf.gyp:protobuf',
        '<(qpb_root)/build/qt5.gyp:qt5',
      ],
      'defines': [
        'QT_PLUGIN',
      ],
      'include_dirs': [
        '<(qpb_root)/sandbox',
      ],
      'sources': [
        '<(qpb_root)/sandbox/qpb/sandbox.h',
        '<(qpb_root)/sandbox/qpb/sandbox.cpp',
        '<(qpb_root)/sandbox/qpb/sandbox_plugin.h',
        '<(qpb_root)/sandbox/qpb/sandbox_plugin.cpp',
      ],
      'copies': [{
        'destination': '<(qml_module_dir)',
        'files': [
          '<(qpb_root)/sandbox/qmldir',
        ],
      }],
    },
    {
      'target_name': 'qpb-qml-test',
      'type': 'none',
      'copies': [{
        'destination': '<(PRODUCT_DIR)/qml_sandbox',
        'files': [
          '<(qpb_root)/sandbox/tst_.qml',
        ],
      }],
    },
  ],
}
