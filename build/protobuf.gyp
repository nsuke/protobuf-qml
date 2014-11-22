{
  'includes': [
    'common.gypi',
  ],
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
