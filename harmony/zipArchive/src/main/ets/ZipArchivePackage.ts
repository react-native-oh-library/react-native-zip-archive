import { RNPackage, TurboModulesFactory } from 'rnoh/ts';
import type { TurboModule, TurboModuleContext } from 'rnoh/ts';
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

