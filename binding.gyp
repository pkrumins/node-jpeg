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
                "src/module.cpp",
            ],
            "include_dirs" : [
                "<!(node -e \"require('nan')\")"
            ],
            "conditions" : [
                [
                    'OS=="linux"', {
                        "libraries" : [
                            '-ljpeg'
                        ],
                        'cflags!': [ '-fno-exceptions' ],
                        'cflags_cc!': [ '-fno-exceptions' ]
                    }
                ],
                [
                    'OS=="mac"', {
                        'xcode_settings': {
                            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES'
                        },
                        "libraries" : [
                            '-ljpeg'
                        ]
                    }
                ],
                [
                    'OS=="win"', {
                        "include_dirs" : [ "gyp/include" ],
                        "libraries" : [
                            '<(module_root_dir)/gyp/lib/libjpeg.lib'
                        ]
                    }
                ]
            ]
        },
        {
          "target_name": "action_after_build",
          "type": "none",
          "dependencies": [ "<(module_name)" ],
          "copies": [
            {
              "files": [ "<(PRODUCT_DIR)/<(module_name).node" ],
              "destination": "<(module_path)"
            }
          ]
        },
    ]
}
