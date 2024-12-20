/*
 * Copyright (c) 2024 Huawei Device Co., Ltd. All rights reserved
 * Use of this source code is governed by a MIT license that can be
 * found in the LICENSE file.
 */

import { RNPackage, TurboModulesFactory } from '@rnoh/react-native-openharmony/ts';
import type { TurboModule, TurboModuleContext } from '@rnoh/react-native-openharmony/ts';
import { ZipArchiveTurboModule } from './ZipArchiveTurboModule';

class ZipArchiveTurboModulesFactory extends TurboModulesFactory {
  createTurboModule(name: string): TurboModule | null {
    if (name === 'RNZipArchive') {
      return new ZipArchiveTurboModule(this.ctx)
    }
    return null;
  }

  hasTurboModule(name: string): boolean {
    return name === 'RNZipArchive';
  }
}

export class ZipArchivePackage extends RNPackage {
  createTurboModulesFactory(ctx: TurboModuleContext): TurboModulesFactory {
    return new ZipArchiveTurboModulesFactory(ctx);
  }
}

