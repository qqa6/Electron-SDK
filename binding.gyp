{
    'target_defaults': {
        'default_configuration': 'Release'
    },
    'targets': [
        {
            'target_name': 'VideoSource',
            'cflags_cc!': ['-fno-rtti'],
            'type': 'executable',
            'include_dirs': [
                './source_code/common',
                './source_code/ipc',
                './source_code/process',
                './source_code/windowInfo',
                './source_code/raw_data/video_transporter',
                './source_code/video_source',
                "<!@(node -p \"require('node-addon-api').include\")"
            ],
            'sources': [
                "<!@(node -p \"var fs=require('fs'),path=require('path'),walk=function(r){let t,e=[],n=null;try{t=fs.readdirSync(r)}catch(r){n=r.toString()}if(n)return n;var a=0;return function n(){var i=t[a++];if(!i)return e;let u=path.resolve(r,i);i=r+'/'+i;let c=fs.statSync(u);if(c&&c.isDirectory()){let r=walk(i);return e=e.concat(r),n()}return e.push(i),n()}()};walk('./source_code/video_source/').join(' ');\")",
                "<!@(node -p \"var fs=require('fs'),path=require('path'),walk=function(r){let t,e=[],n=null;try{t=fs.readdirSync(r)}catch(r){n=r.toString()}if(n)return n;var a=0;return function n(){var i=t[a++];if(!i)return e;let u=path.resolve(r,i);i=r+'/'+i;let c=fs.statSync(u);if(c&&c.isDirectory()){let r=walk(i);return e=e.concat(r),n()}return e.push(i),n()}()};walk('./source_code/ipc/').join(' ');\")",
                "<!@(node -p \"var fs=require('fs'),path=require('path'),walk=function(r){let t,e=[],n=null;try{t=fs.readdirSync(r)}catch(r){n=r.toString()}if(n)return n;var a=0;return function n(){var i=t[a++];if(!i)return e;let u=path.resolve(r,i);i=r+'/'+i;let c=fs.statSync(u);if(c&&c.isDirectory()){let r=walk(i);return e=e.concat(r),n()}return e.push(i),n()}()};walk('./source_code/raw_data/video_transporter/').join(' ');\")",
                "<!@(node -p \"var fs=require('fs'),path=require('path'),walk=function(r){let t,e=[],n=null;try{t=fs.readdirSync(r)}catch(r){n=r.toString()}if(n)return n;var a=0;return function n(){var i=t[a++];if(!i)return e;let u=path.resolve(r,i);i=r+'/'+i;let c=fs.statSync(u);if(c&&c.isDirectory()){let r=walk(i);return e=e.concat(r),n()}return e.push(i),n()}()};walk('./source_code/common/').join(' ');\")",
            ],
            'conditions': [
                [
                    'OS=="win"',
                    {
                        'library_dirs': [
                            './sdk/lib/win32/library',
                        ],
                        'link_settings': {
                            'libraries': [
                                '-lagora_rtc_sdk.lib',
                                '-lws2_32.lib',
                                '-lRpcrt4.lib',
                                '-lgdiplus.lib',
                            ]
                        },
                        'link_settings!': [
                            '-liojs.lib',
                        ],
                        'sources': [
                            './source_code/process/node_process_win.cpp',
                        ],
                        'include_dirs': [
                            './sdk/include',
                        ],
                        'defines': [
                            '_USING_V110_SDK71_',
                            '_HAS_EXCEPTIONS=0',
                            'IRIS_EXPORT',
                            'IRIS_JSON_ARRAY',
                            'EXECUTABLE_OUTPUT_NAME="VideoSource"'
                        ],
                        'configurations': {
                            'Release': {
                                'msvs_settings': {
                                    'VCCLCompilerTool': {
                                        'ExceptionHandling': '0',
                                        'AdditionalOptions': [
                                            '/EHsc'
                                        ]
                                    },
                                    'VCManifestTool': {
                                        'EmbedManifest': 'true',
                                        'AdditionalManifestFiles': '../Resources/dpi_aware.manifest'
                                    }
                                }
                            },
                            'Debug': {
                                'msvs_settings': {
                                    'VCCLCompilerTool': {
                                        'ExceptionHandling': '0',
                                        'AdditionalOptions': [
                                            '/EHsc'
                                        ]
                                    },
                                    'VCManifestTool': {
                                        'EmbedManifest': 'true',
                                        'AdditionalManifestFiles': '../Resources/dpi_aware.manifest'
                                    }
                                }
                            }
                        }
                    }
                ], [
                    'OS=="mac"',
                    {
                        'mac_framework_dirs': [
                            '../sdk/lib/mac',
                            '../iris/MAC/Release',
                        ],
                        'link_settings': {
                            'libraries': [
                                'AgoraAIDenoiseExtension.framework',
                                'AgoraCore.framework',
                                'AgoraDav1dExtension.framework',
                                'Agorafdkaac.framework',
                                'Agoraffmpeg.framework',
                                'AgoraRtcKit.framework',
                                'AgoraSoundTouch.framework',
                                'AgoraRtcWrapper.framework',
                                'av1.framework',
                                'AppKit.framework',
                                'CoreGraphics.framework',
                            ]
                        },
                        'include_dirs':[
                            './sdk/lib/mac/AgoraRtcKit.framework/Headers/',
                            './iris/MAC/Release/AgoraRtcWrapper.framework/Headers/',
                        ],
                        'sources': [
                            './source_code/process/node_process_unix.cpp',
                        ],
                        'defines': [
                            'IRIS_JSON_ARRAY',
                            'EXECUTABLE_OUTPUT_NAME="VideoSource"'
                        ],
                        'OTHER_CFLAGS': [
                            '-std=c++11',
                            '-stdlib=libc++',
                            '-fexceptions'
                        ],
                        'xcode_settings': {
                            'MACOSX_DEPLOYMENT_TARGET': '10.11',
                            'GCC_ENABLE_CPP_RTTI': 'YES',
                            'FRAMEWORK_SEARCH_PATHS': [
                                './sdk/lib/mac',
                                './iris/MAC/Release'
                            ],
                            "DEBUG_INFORMATION_FORMAT": "dwarf-with-dsym",
                            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
                            'CLANG_CXX_LIBRARY': 'libc++'
                        },
                    }
                ]
            ]
        },
        {
            'target_name': 'agora_node_ext',
            'cflags_cc!': ['-fno-rtti'],
            'include_dirs': [
                './source_code/agora_node_ext',
                './source_code/common/',
                './source_code/ipc/',
                './source_code/process/',
                './source_code/window/',
                './source_code/raw_data/video_transporter/',
                './source_code/windowInfo/',
                "<!@(node -p \"require('node-addon-api').include\")"
            ],
            'sources': [
                "<!@(node -p \"var fs=require('fs'),path=require('path'),walk=function(r){let t,e=[],n=null;try{t=fs.readdirSync(r)}catch(r){n=r.toString()}if(n)return n;var a=0;return function n(){var i=t[a++];if(!i)return e;let u=path.resolve(r,i);i=r+'/'+i;let c=fs.statSync(u);if(c&&c.isDirectory()){let r=walk(i);return e=e.concat(r),n()}return e.push(i),n()}()};walk('./source_code/agora_node_ext').join(' ');\")",
                "<!@(node -p \"var fs=require('fs'),path=require('path'),walk=function(r){let t,e=[],n=null;try{t=fs.readdirSync(r)}catch(r){n=r.toString()}if(n)return n;var a=0;return function n(){var i=t[a++];if(!i)return e;let u=path.resolve(r,i);i=r+'/'+i;let c=fs.statSync(u);if(c&&c.isDirectory()){let r=walk(i);return e=e.concat(r),n()}return e.push(i),n()}()};walk('./source_code/ipc').join(' ');\")",
                "<!@(node -p \"var fs=require('fs'),path=require('path'),walk=function(r){let t,e=[],n=null;try{t=fs.readdirSync(r)}catch(r){n=r.toString()}if(n)return n;var a=0;return function n(){var i=t[a++];if(!i)return e;let u=path.resolve(r,i);i=r+'/'+i;let c=fs.statSync(u);if(c&&c.isDirectory()){let r=walk(i);return e=e.concat(r),n()}return e.push(i),n()}()};walk('./source_code/raw_data/video_transporter/').join(' ');\")",
                "<!@(node -p \"var fs=require('fs'),path=require('path'),walk=function(r){let t,e=[],n=null;try{t=fs.readdirSync(r)}catch(r){n=r.toString()}if(n)return n;var a=0;return function n(){var i=t[a++];if(!i)return e;let u=path.resolve(r,i);i=r+'/'+i;let c=fs.statSync(u);if(c&&c.isDirectory()){let r=walk(i);return e=e.concat(r),n()}return e.push(i),n()}()};walk('./source_code/common/').join(' ');\")",
            ],
            'conditions': [
                [
                    'OS=="win"',
                    {
                        'copies': [{
                            'destination': '<(PRODUCT_DIR)',
                            'files': [
                                './sdk/lib/win32/library/agora_rtc_sdk.dll',
                                './sdk/lib/win32/library/libagora-fdkaac.dll',
                                './sdk/lib/win32/library/libagora-ffmpeg.dll',
                                './sdk/lib/win32/library/libagora-mpg123.dll',
                                './sdk/lib/win32/library/libagora-soundtouch.dll',
                                './sdk/lib/win32/library/libhwcodec.dll',
                                './sdk/lib/win32/library/av1.dll',
                                './sdk/lib/win32/library/libagora_ai_denoise_extension.dll',
                                './sdk/lib/win32/library/libagora_dav1d_extension.dll',
                                './sdk/lib/win32/library/libagora-core.dll'
                            ]
                        }],
                        'link_settings': {
                            'libraries': [
                                '-lws2_32.lib',
                                '-lRpcrt4.lib',
                                '-lgdiplus.lib',
                                '-lagora_rtc_sdk.lib'
                            ]
                        },
                        'library_dirs': [
                            './sdk/lib/win32/library',
                        ],
                        'defines': [
                            '_USING_V110_SDK71_',
                            '_HAS_EXCEPTIONS=0',
                            'IRIS_JSON_ARRAY',
                            'IRIS_EXPORT',
                            'EXECUTABLE_OUTPUT_NAME="agora_node_ext"'
                        ],
                        'sources': [
                            './source_code/process/node_process_win.cpp',
                            './source_code/windowInfo/node_screen_window_info_win.cpp',
                            './source_code/windowInfo/node_screen_window_info.h'
                        ],
                        'include_dirs': [
                            './sdk/include',
                            './extra/internal'
                        ],
                        'configurations': {
                            'Release': {
                                'msvs_settings': {
                                    'VCCLCompilerTool': {
                                        'ExceptionHandling': '1',
                                        'AdditionalOptions': [
                                            '/EHsc'
                                        ]
                                    }
                                }
                            },
                            'Debug': {
                                'msvs_settings': {
                                    'VCCLCompilerTool': {
                                        'ExceptionHandling': '1',
                                        'AdditionalOptions': [
                                            '/EHsc'
                                        ]
                                    }
                                }
                            }
                        }

                    }
                ],
                [
                    'OS=="mac"',
                    {
                        'mac_framework_dirs': [
                            '../sdk/lib/mac',
                            '../iris/MAC/Release',
                        ],
                        'copies': [{
                            'destination': '<(PRODUCT_DIR)',
                            'files': [
                                './sdk/lib/mac/AgoraAIDenoiseExtension.framework',
                                './sdk/lib/mac/AgoraCore.framework',
                                './sdk/lib/mac/AgoraDav1dExtension.framework',
                                './sdk/lib/mac/Agorafdkaac.framework',
                                './sdk/lib/mac/Agoraffmpeg.framework',
                                './sdk/lib/mac/AgoraRtcKit.framework',
                                './sdk/lib/mac/AgoraSoundTouch.framework',
                                './sdk/lib/mac/av1.framework',
                                './iris/MAC/Release/AgoraRtcWrapper.framework',
                                './iris/MAC/Release/AgoraRtcScreenSharing',
                            ]
                        }],
                        'link_settings': {
                            'libraries': [
                                'AgoraAIDenoiseExtension.framework',
                                'AgoraCore.framework',
                                'AgoraDav1dExtension.framework',
                                'Agorafdkaac.framework',
                                'Agoraffmpeg.framework',
                                'AgoraRtcKit.framework',
                                'AgoraSoundTouch.framework',
                                'AgoraRtcWrapper.framework',
                                'av1.framework',
                                'AppKit.framework',
                                'CoreGraphics.framework',
                            ]
                        },
                        'sources': [
                            './source_code/windowInfo/node_screen_window_info_mac.cpp',
                            './source_code/windowInfo/node_screen_window_info.h',
                            './source_code/process/node_process_unix.cpp',
                        ],
                        'include_dirs': [
                            './sdk/lib/mac/AgoraRtcKit.framework/Headers',
                            './iris/MAC/Release/AgoraRtcWrapper.framework/Headers',
                        ],
                        'defines': [
                            # '_NOEXCEPT',
                            # '-std=c++11',
                            'IRIS_JSON_ARRAY',
                            'EXECUTABLE_OUTPUT_NAME="agora_node_ext"'
                        ],
                        'OTHER_CFLAGS': [
                            '-std=c++11',
                            '-stdlib=libc++',
                            '-fexceptions'
                        ],
                        'xcode_settings': {
                            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
                            'GCC_ENABLE_CPP_RTTI': 'YES',
                            'MACOSX_DEPLOYMENT_TARGET': '10.11',
                            'EXECUTABLE_EXTENSION': 'node',
                            'FRAMEWORK_SEARCH_PATHS': [
                                './sdk/lib/mac',
                                './iris/MAC/Release'
                            ],
                            "DEBUG_INFORMATION_FORMAT": "dwarf-with-dsym",
                        },
                    }
                ]
            ]
        },
    ]
}
