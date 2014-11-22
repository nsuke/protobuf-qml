{
  'includes': ['common.gypi'],
  'targets': [
    {
      'target_name': 'qt5',
      'type': 'none',
      'direct_dependent_settings': {
        'variables': {
          'qt_moc%': 1,
        },
        'defines': [
          'QT_QUICK_LIB',
          'QT_QML_LIB',
          'QT_NETWORK_LIB',
          'QT_GUI_LIB',
          'QT_CORE_LIB',
        ],
        'include_dirs+': [
          '<(qt_include_dir)',
          '<(qt_include_dir)/QtCore',
          '<(qt_include_dir)/QtQml',
        ],
        'link_settings': {
          'libraries': [
            '<(qt_lib_dir)/<(SHARED_LIB_PREFIX)Qt5Core<(SHARED_LIB_SUFFIX)',
            '<(qt_lib_dir)/<(SHARED_LIB_PREFIX)Qt5Gui<(SHARED_LIB_SUFFIX)',
            '<(qt_lib_dir)/<(SHARED_LIB_PREFIX)Qt5Qml<(SHARED_LIB_SUFFIX)',
            '<(qt_lib_dir)/<(SHARED_LIB_PREFIX)Qt5Quick<(SHARED_LIB_SUFFIX)',
          ],
        },
      },
    },
  ],
}
