import ReactNative from "react-native";
import RNZipArchive from "./src/NativeZipArchive"


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

// export const unzipAssets = (source, target) => {
//   if (!RNZipArchive.unzipAssets) {
//     throw new Error("unzipAssets not supported on this platform");
//   }

//   return RNZipArchive.unzipAssets(source, target);
// };

// export const subscribe = (callback) => {
//   return rnzaEmitter.addListener("zipArchiveProgressEvent", callback);
// };

// export const getUncompressedSize = (source, charset = "UTF-8") => {
//   return RNZipArchive.getUncompressedSize(source, charset);
// };

// export const isPasswordProtected = (source) => {
//   return RNZipArchive.isPasswordProtected(source).then(
//     (isEncrypted) => !!isEncrypted
//   );
// };