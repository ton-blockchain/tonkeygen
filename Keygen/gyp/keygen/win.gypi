# This file is part of TON Key Generator,
# a desktop application for the TON Blockchain project.
#
# For license and copyright information please follow this link:
# https://github.com/ton-blockchain/tonkeygen/blob/master/LEGAL

{
  'conditions': [[ 'build_win', {
    'libraries': [
      '-lzlibstat',
      '-lUxTheme',
      '-lDwmapi',
      '-lDbgHelp',
    ],
    'msvs_settings': {
      'VCManifestTool': {
        'AdditionalManifestFiles': '<(res_loc)/win/Keygen.manifest',
      }
    },
    'configurations': {
      'Debug': {
        'library_dirs': [
          '<(libs_loc)/ton/build-debug',
          '<(libs_loc)/zlib/contrib/vstudio/vc14/x86/ZlibStatDebug',
        ],
        'msvs_settings': {
          'VCLinkerTool': {
            'AdditionalDependencies': [
              'tdutils/Debug/tdutils.lib',
              'tdactor/Debug/tdactor.lib',
              'adnl/Debug/adnllite.lib',
              'tl/Debug/tl_api.lib',
              'crypto/Debug/ton_crypto.lib',
              'crypto/Debug/ton_block.lib',
              'tl/Debug/tl_tonlib_api.lib',
              'tonlib/Debug/tonlib.lib',
              'tdnet/Debug/tdnet.lib',
              'keys/Debug/keys.lib',
              'tl-utils/Debug/tl-utils.lib',
              'tl-utils/Debug/tl-lite-utils.lib',
              'tl/Debug/tl_lite_api.lib',
              'third-party/crc32c/Debug/crc32c.lib',
            ],
          },
        },
      },
      'Release': {
        'library_dirs': [
          '<(libs_loc)/ton/build',
          '<(libs_loc)/zlib/contrib/vstudio/vc14/x86/ZlibStatReleaseWithoutAsm',
        ],
        'msvs_settings': {
          'VCLinkerTool': {
            'AdditionalDependencies': [
              'tdutils/Release/tdutils.lib',
              'tdactor/Release/tdactor.lib',
              'adnl/Release/adnllite.lib',
              'tl/Release/tl_api.lib',
              'crypto/Release/ton_crypto.lib',
              'crypto/Release/ton_block.lib',
              'tl/Release/tl_tonlib_api.lib',
              'tonlib/Release/tonlib.lib',
              'tdnet/Release/tdnet.lib',
              'keys/Release/keys.lib',
              'tl-utils/Release/tl-utils.lib',
              'tl-utils/Release/tl-lite-utils.lib',
              'tl/Release/tl_lite_api.lib',
              'third-party/crc32c/Release/crc32c.lib',
            ],
          },
        },
      },
    },
  }]],
}
