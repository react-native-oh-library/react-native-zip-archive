import { TurboModule } from "@rnoh/react-native-openharmony/ts";
import common from '@ohos.app.ability.common';
import promptAction from '@ohos.promptAction'
import minizip from "libnativi_minizip.so";
import { TM } from '@rnoh/react-native-openharmony/generated/ts'


enum encryptionMethods {
  'STANDARD',
  'AES-128',
  'AES-256'
}

export class ZipArchiveTurboModule extends TurboModule implements TM.RNZipArchive.Spec {

  pathParameters(): string {
    //获取应用文件路径
    let context = getContext(this) as common.UIAbilityContext;
    let newFilesDir: string = context.filesDir;
    return newFilesDir;
  }

  // 原接口压缩
  zip(sourcePath: string, zipPath: string): Promise<string> {
    try {
      this.compress(sourcePath, zipPath);
      return Promise.resolve('压缩成功');
    } catch (err) {
      console.info(`zip error: ${err}`);
      return Promise.resolve('压缩失败');
    }
  }

  unzip(sourcePath: string, zipPath: string, charset: string = "UTF-8"): Promise<string> {
    try {
      this.decompress(sourcePath, zipPath)
      return Promise.resolve('解压成功');
    } catch (err) {
      console.info(`unzip error: ${err}`);
      return Promise.resolve('解压失败');
    }
  }

  zipWithPassword(sourcePath: string, zipPath: string, password: string,
    encryptionMethod?:  'STANDARD' | 'AES-128' | 'AES-256' | ''): Promise<string> {
    try {
      this.compressWithPsd(sourcePath, zipPath, password);
      return Promise.resolve('加密压缩成功');
    } catch (err) {
      console.info(`zipWithPassword error: ${err}`);
      return Promise.resolve('加密压缩失败');
    }
  }

  unzipWithPassword(sourcePath: string, zipPath: string, password: string): Promise<string> {
    try {
      this.decompressWithPsd(sourcePath, zipPath, password)
      return Promise.resolve('加密解压成功');
    } catch (err) {
      console.info(`unzipWithPassword error: ${err}`);
      return Promise.resolve('加密解压失败');
    }
  }

  compress(sourcePath: string, zipPath: string) {
    try {
      let result = minizip.create().compress({
        path: zipPath,
        operate: "compress",
        option: { append: 1, compress_level: 9 },
        files: [sourcePath]
      })
      console.info(`compress result: ${result}`)
    } catch (error) {
      console.info(`compress err: ${error}}`)
    }
  }

  decompress(sourcePath: string, zipPath: string): void {
    try {
      let result = minizip.create().decompress({
        path: zipPath,
        operate: "decompress",
        option: { overwrite: 1, compress_level: 9 },
        directory: sourcePath
      })
      console.info(`decompress result: ${result}`)
    } catch (error) {
      console.info(`decompress err: ${error}}`)
    }
  }

  // 加密压缩
  compressWithPsd(sourcePath: string, zipPath: string, password: string): void {
    try {
      let result = minizip.create().compressWithPsd({
        path: zipPath,
        password: password,
        operate: "compress",
        option: { overwrite: 1, compress_level: 9 },
        files: [sourcePath]
      })
      console.info(`compressWithPsd result: ${result}`)
    } catch (error) {
      console.info(`compressWithPsd err: ${error}}`)
    }
  }

  // 加密解压
  decompressWithPsd(sourcePath: string, zipPath: string, password: string): void {
    try {
      let result = minizip.create().decompressWithPsd({
        path: zipPath,
        password: password,
        operate: "decompress",
        option: { overwrite: 1, compress_level: 9 },
        directory: sourcePath
      })
      console.info(`decompressWithPsd result: ${result}`)
    } catch (error) {
      console.info(`decompressWithPsd err: ${error}}`)
    }
  }
}
