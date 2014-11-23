{
  'includes': [
    'common.gypi',
  ],
  'variables': {
    'conditions': [
      ['protobuf_static_link == 1', {
        'protobuf_lib%': '<(protobuf_lib_dir)/<(STATIC_LIB_PREFIX)protobuf<(STATIC_LIB_SUFFIX)',
        'protoc_lib%': '<(protobuf_lib_dir)/<(STATIC_LIB_PREFIX)protoc<(STATIC_LIB_SUFFIX)',
      }, {
        'protobuf_lib%': '<(protobuf_lib_dir)/<(SHARED_LIB_PREFIX)protobuf<(SHARED_LIB_SUFFIX)',
        'protoc_lib%': '<(protobuf_lib_dir)/<(SHARED_LIB_PREFIX)protoc<(SHARED_LIB_SUFFIX)',
      }],
    ],
  },
  'targets': [
    {
      'target_name': 'protobuf',
      'type': 'none',
      'direct_dependent_settings': {
        'include_dirs': ['<(protobuf_include_dir)'],
        'link_settings': {
          'libraries': ['<(protobuf_lib)'],
        },
      },
    },
    {
      'target_name': 'protoc',
      'type': 'none',
      'direct_dependent_settings': {
        'include_dirs': ['<(protobuf_include_dir)'],
        'link_settings': {
          'libraries': ['<(protoc_lib)'],
        },
      },
    },
  ],
}
