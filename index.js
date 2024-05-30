import ReactNative, { DeviceEventEmitter } from "react-native";
import RNZipArchive from "./src/NativeZipArchive"
const { NativeEventEmitter, NativeModules } = ReactNative;

// const RNZipArchive = NativeModules.RNZipArchive;

const rnzaEmitter = new NativeEventEmitter(RNZipArchive);

export const creteFile = (filePath, fileContent) => {
  return RNZipArchive.creteFile(filePath, fileContent)
}

export const pathParameters = () => {
  return RNZipArchive.pathParameters();
}

export const zip = (source, target) => {
  return RNZipArchive.zip(source, target);
};

export const unzip = (source, target, charset = "UTF-8") => {
  return RNZipArchive.unzip(source, target, charset);
};

export const zipWithPassword = (source, target, password, encryptionMethod = "") => {
  return RNZipArchive.zipWithPassword(source, target, password, encryptionMethod)
};

export const unzipWithPassword = (source, target, password) => {
  return RNZipArchive.unzipWithPassword(source, target, password);
};

export const isPasswordProtected = (source) => {
  return RNZipArchive.isPasswordProtected(source);
};

export const subscribe = (callback) => {
  const onZipArchiveProgressEvent = DeviceEventEmitter.addListener('zipArchiveProgressEvent', (progress) => {
    callback({ progress });
    if (progress === 100) {
      onZipArchiveProgressEvent.remove();
    }
  })
};

export const unzipAssets = (source, target) => {
  if (!RNZipArchive.unzipAssets) {
    throw new Error("unzipAssets not supported on this platform");
  }

  return RNZipArchive.unzipAssets(source, target);
};

export const getUncompressedSize = (source, charset = "UTF-8") => {
  return RNZipArchive.getUncompressedSize(source, charset);
};

