/*
 * Copyright (c) 2024 Huawei Device Co., Ltd. All rights reserved
 * Use of this source code is governed by a MIT license that can be
 * found in the LICENSE file.
 */

import type { TurboModule } from "react-native/Libraries/TurboModule/RCTExport";
import { TurboModuleRegistry } from "react-native";
import { Int32 } from "react-native/Libraries/Types/CodegenTypes";

export interface Spec extends TurboModule {
  creteFile(filePath: string, fileContent: string): Promise<string>;
  pathParameters(): string;
  zip(source: string, target: string): Promise<string>;
  unzip(source: string, target: string, charset?: string): Promise<string>;
  zipWithPassword(source: string, target: string, password: string, encryptionMethod?: 'STANDARD' | 'AES-128' | 'AES-256' | ''): Promise<string>;
  unzipWithPassword(assetPath: string, target: string, password: string): Promise<string>;
  isPasswordProtected(source: string): Promise<boolean>;
  subscribe(callback: ({ progress, filePath }: { progress: Int32, filePath: string }) => void): void;
  unzipAssets(assetPath: string, target: string): Promise<string>;
  getUncompressedSize(source: string, charset?: string): Promise<number>;
}

export default TurboModuleRegistry.get<Spec>("RNZipArchive") as Spec | null;