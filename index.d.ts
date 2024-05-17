declare module 'react-native-zip-archive' {
  import { NativeEventSubscription } from 'react-native';
  export function pathParameters(): string;
  export function zip(source: string, target: string): Promise<string>;
  export function unzip(source: string, target: string, charset?: string): Promise<string>;
  export function zipWithPassword(source: string, target: string, password: string, encryptionMethod?: 'STANDARD' | 'AES-128' | 'AES-256' | ''): Promise<string>;
  export function unzipWithPassword(assetPath: string, target: string, password: string): Promise<string>;
  // export function unzipAssets(assetPath: string, target: string): Promise<string>;
  // export function subscribe(callback: ({ progress, filePath }: { progress: number, filePath: string }) => void): NativeEventSubscription;
  // export function getUncompressedSize(source: string, charset?: string): Promise<number>;
  // export function isPasswordProtected(source: string): Promise<boolean>;
}
