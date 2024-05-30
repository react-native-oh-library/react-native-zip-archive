declare module 'react-native-zip-archive' {
  export function creteFile(filePath: string, fileContent: string): Promise<string>;
  export function pathParameters(): string;
  export function zip(source: string, target: string): Promise<string>;
  export function unzip(source: string, target: string, charset?: string): Promise<string>;
  export function zipWithPassword(source: string, target: string, password: string, encryptionMethod?: 'STANDARD' | 'AES-128' | 'AES-256' | ''): Promise<string>;
  export function unzipWithPassword(assetPath: string, target: string, password: string): Promise<string>;
  export function isPasswordProtected(source: string): Promise<boolean>;
  export function subscribe(callback: ({ progress, filePath }: { progress: number, filePath: string }) => void): void;
  export function unzipAssets(assetPath: string, target: string): Promise<string>;
  export function getUncompressedSize(source: string, charset?: string): Promise<number>;
}
