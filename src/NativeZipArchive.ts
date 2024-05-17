import type { TurboModule } from "react-native/Libraries/TurboModule/RCTExport";
import { TurboModuleRegistry } from "react-native";

export interface Spec extends TurboModule {
  pathParameters():string;
  zip(source: string, target: string): Promise<string>;
  unzip(source: string, target: string, charset?: string): Promise<string>;
  zipWithPassword(source: string, target: string, password: string, encryptionMethod?: 'STANDARD' | 'AES-128' | 'AES-256' | ''): Promise<string>;
  unzipWithPassword(assetPath: string, target: string, password: string): Promise<string>;
  // unzipAssets(assetPath: string, target: string): Promise<string>;
  // getUncompressedSize(source: string, charset?: string): Promise<number>;
  // isPasswordProtected(source: string): Promise<boolean>;
}

export default TurboModuleRegistry.get<Spec>("RNZipArchive") as Spec | null;