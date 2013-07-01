{
  "targets": [
    {
      "target_name": "jpeg",
      "sources": [
        "src/common.cpp",
        "src/jpeg_encoder.cpp",
        "src/jpeg.cpp",
        "src/fixed_jpeg_stack.cpp",
        "src/dynamic_jpeg_stack.cpp",
        "src/module.cpp"],
      "libraries": ["-lJPEG"],
      'cflags!': [ '-fno-exceptions' ],
      'cflags_cc!': [ '-fno-exceptions' ],
      'conditions': [
        ['OS=="mac"', {
          'xcode_settings': {
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES'
          }
        }]
      ]
    }
  ]
}
