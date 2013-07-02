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
        "src/module.cpp"
      ],
      "libraries": ["-lJPEG"],
      "cflags!": [ "-fno-exceptions", "-D_FILE_OFFSET_BITS=64", "-D_LARGEFILE_SOURCE" ],
      "cflags_cc!": [ "-fno-exceptions", "-D_FILE_OFFSET_BITS=64", "-D_LARGEFILE_SOURCE" ],
      "conditions": [
        ["OS=='mac'", {
          "xcode_settings": {
            "GCC_ENABLE_CPP_EXCEPTIONS": "YES"
          }
        }]
      ]
    }
  ]
}
