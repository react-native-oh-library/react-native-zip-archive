import { TurboModule } from "@rnoh/react-native-openharmony/ts";
import common from '@ohos.app.ability.common';
import promptAction from '@ohos.promptAction'
import minizip from "libnativi_minizip.so";
import {TM} from '@rnoh/react-native-openharmony/generated/ts'

//获取应用文件路径
let context = getContext(this) as common.UIAbilityContext;
let filesDir = context.filesDir;

export class ZipArchiveTurboModule extends TurboModule implements TM.RNZipArchive.Spec {

  // 压缩
  compress(): void {
    try {
      let result = minizip.create().compress({
        path: filesDir + ".zip",
        operate: "compress",
        option: { append: 1, compress_level: 9 },
        files: [filesDir]
      })
      promptAction.showToast({
        message: 'Compression success',
        duration: 2000
      });
      console.info(`compress result: ${result}`)
    } catch (error) {
      promptAction.showToast({
        message: 'Compression failed',
        duration: 2000
      });
      console.info(`compress err: ${error}}`)
    }
  }

  // 解压
  decompress(): void {
    try {
      let result = minizip.create().decompress({
        path: filesDir + ".zip",
        operate: "decompress",
        option: { overwrite: 1, compress_level: 9 },
        directory: filesDir + "out"
      })
      promptAction.showToast({
        message: 'decompression success',
        duration: 2000
      });
      console.info(`decompress result: ${result}`)
    } catch (error) {
      promptAction.showToast({
        message: 'decompression failed',
        duration: 2000
      });
      console.info(`decompress err: ${error}}`)
    }
  }

  // 加密压缩
  compressWithPsd(): void {
    try {
      let result = minizip.create().compressWithPsd({
        path: filesDir + ".zip",
        password: "test",
        operate: "compress",
        option: { overwrite: 1, compress_level: 9 },
        files: [filesDir]
      })
      promptAction.showToast({
        message: 'Encryption add compression success',
        duration: 2000
      });
      console.info(`compressWithPsd result: ${result}`)
    } catch (error) {
      promptAction.showToast({
        message: 'Encryption add compression failed',
        duration: 2000
      });
      console.info(`compressWithPsd err: ${error}}`)
    }
  }

  // 加密解压
  decompressWithPsd(): void {
    try {
      let result = minizip.create().decompressWithPsd({
        path: filesDir + ".zip",
        password: "test",
        operate: "decompress",
        option: { overwrite: 1, compress_level: 9 },
        directory: filesDir + "out"
      })
      promptAction.showToast({
        message: 'Encryption and decompression success',
        duration: 2000
      });
      console.info(`decompressWithPsd result: ${result}`)
    } catch (error) {
      promptAction.showToast({
        message: 'Encryption and decompression failed',
        duration: 2000
      });
      console.info(`decompressWithPsd err: ${error}}`)
    }
  }
}
