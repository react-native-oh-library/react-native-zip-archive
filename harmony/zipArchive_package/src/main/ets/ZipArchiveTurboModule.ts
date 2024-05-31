import { TurboModule } from "@rnoh/react-native-openharmony/ts";
import common from '@ohos.app.ability.common';
import promptAction from '@ohos.promptAction'
import minizip from "libnativi_minizip.so";
import { TM } from '@rnoh/react-native-openharmony/generated/ts'
import { RNOHContext } from '@rnoh/react-native-openharmony/ts'
import fs, { Filter, ListFileOptions } from '@ohos.file.fs';
import { buffer, JSON } from '@kit.ArkTS';
import { BusinessError } from '@kit.BasicServicesKit';

interface NativeEventSubscription {
  remove(): void;
}

interface FilePathResult {
  result?: FilePath[];
}

interface FilePath {
  path?: string;
}

//获取应用文件路径
let context = getContext(this) as common.UIAbilityContext;

export class ZipArchiveTurboModule extends TurboModule implements TM.RNZipArchive.Spec {
  ctx: RNOHContext;

  // 沙箱路径
  pathParameters(): string {
    let newFilesDir: string = context.filesDir;
    return newFilesDir;
  }

  // 创建文件
  creteFile(filePath: string, fileContent: string): Promise<string> {
    let file = fs.openSync(filePath, fs.OpenMode.READ_WRITE | fs.OpenMode.CREATE);
    let str: string = fileContent; //写入内容
    fs.write(file.fd, str).then((writeLen: number) => {
      console.info(`write data to file succeed and size is:${writeLen}----file: ${file}---str:${str}`);
    }).catch((err: BusinessError) => {
      console.error("write data to file failed with error message: " + err.message + ", error code: " + err.code);
    }).finally(() => {
      fs.closeSync(file);
    });
    return Promise.resolve('文件创建成功');
  }

  // 查看文件列表
  isEmptyFolder(path: string): boolean {
    let isFile = fs.statSync(path).isFile();
    if (isFile) {
      return false;
    }
    let files = fs.listFileSync(context.filesDir);
    console.info('isEmptyFolder:' + files)
    return files.length <= 0;
  }

  // react-native-zip-archive接口
  zip(source: string, target: string): Promise<string> {
    try {
      this.compress(source, target);
      console.info('zip---2')
      return Promise.resolve('压缩成功');
    } catch (err) {
      console.info(`zip error: ${err}`);
      return Promise.resolve('压缩失败');
    }
  }

  unzip(source: string, target: string, charset: string = "UTF-8"): Promise<string> {
    try {
      this.decompress(source, target)
      return Promise.resolve('解压成功');
    } catch (err) {
      console.info(`unzip error: ${err}`);
      return Promise.resolve('解压失败');
    }
  }

  zipWithPassword(source: string, target: string, password: string,
    encryptionMethod?: 'STANDARD' | 'AES-128' | 'AES-256' | ''): Promise<string> {
    try {
      this.compressWithPsd(source, target, password);
      return Promise.resolve('加密压缩成功');
    } catch (err) {
      console.info(`zipWithPassword error: ${err}`);
      return Promise.resolve('加密压缩失败');
    }
  }

  unzipWithPassword(source: string, target: string, password: string): Promise<string> {
    try {
      this.decompressWithPsd(source, target, password)
      return Promise.resolve('加密解压成功');
    } catch (err) {
      console.info(`unzipWithPassword error: ${err}`);
      return Promise.resolve('加密解压失败');
    }
  }

  isPasswordProtected(source: string): Promise<boolean> {
    return new Promise(resolve => {
      let result = false;
      try {
        result = minizip.create().isPasswordProtected({
          path: source
        });
        console.info(`isPasswordProtected result: ${result}----${source}`)
        resolve(result);
      } catch (err) {
        console.info(`isPasswordProtected err: ${err}`)
      }
    })
  }

  subscribe(callback: ({ progress, filePath }: {
    progress: number,
    filePath: string
  }) => void): void {
    let eventEmitter =
      this.ctx.rnInstance.emitDeviceEvent('zipArchiveProgressEvent', callback({ progress: 0, filePath: '' }))
    console.log(`zipArchiveProgressEvent:${eventEmitter}`);
  }

  unzipAssets(assetPath: string, target: string): Promise<string> {
    this.decompress(assetPath, target);
    return Promise.resolve('解压成功');
  }

  getUncompressedSize(source: string, charset?: string): Promise<number> {
    return new Promise(resole => {
      let result: number;
      try {
        result = minizip.create().getUnCompressedSize({
          path: source
        });
        console.info(`getUncompressedSize result: ${result}----${source}`)
        resole(result)
      } catch (err) {
        console.info(`getUncompressedSize err: ${err}`)
      }
    })
  }

  // minizip接口
  compress(source: string, target: string) {
    try {
      let result = minizip.create().compress({
        path: target,
        operate: "compress",
        option: { append: 1, compress_level: 9 },
        files: [source]
      }, (progress: number) => {
        this.ctx.rnInstance.emitDeviceEvent('zipArchiveProgressEvent', Math.floor(progress));
        console.info(`test-0514 progress---compress: ${Math.floor(progress)}`)
      })
      console.info(`compress result: ${result}`)
    } catch (error) {
      console.info(`compress err: ${error}`)
    }
  }

  decompress(source: string, target: string): void {
    try {
      let result = minizip.create().decompress({
        path: source,
        operate: "decompress",
        option: { overwrite: 1, compress_level: 9 },
        directory: target
      }, (progress: number) => {
        this.ctx.rnInstance.emitDeviceEvent('zipArchiveProgressEvent', Math.floor(progress));
        console.info(`test-0514 progress---: ${Math.floor(progress)}`)
      })
      console.info(`decompress result: ${result}`)
    } catch (error) {
      console.info(`decompress err: ${error}`)
    }
  }

  compressWithPsd(source: string, target: string, password: string): void {
    try {
      let result = minizip.create().compressWithPsd({
        path: target,
        password: password,
        operate: "compress",
        option: { overwrite: 1, compress_level: 9 },
        files: [source]
      }, (progress: number) => {
        this.ctx.rnInstance.emitDeviceEvent('zipArchiveProgressEvent', Math.floor(progress));
        console.info(`test-0514 progress---compressWithPsd: ${Math.floor(progress)}`)
      })
      console.info(`compressWithPsd result: ${result}`)
    } catch (error) {
      console.info(`compressWithPsd err: ${error}`)
    }
  }

  decompressWithPsd(source: string, target: string, password: string): void {
    try {
      let result = minizip.create().decompressWithPsd({
        path: source,
        password: password,
        operate: "decompress",
        option: { overwrite: 1, compress_level: 9 },
        directory: target
      }, (progress: number) => {
        this.ctx.rnInstance.emitDeviceEvent('zipArchiveProgressEvent', Math.floor(progress));
        console.info(`test-0514 progress---decompressWithPsd: ${Math.trunc(progress)}`)
      })
      console.info(`decompressWithPsd result: ${result}`)
    } catch (error) {
      console.info(`decompressWithPsd err: ${error}`)
    }
  }
}