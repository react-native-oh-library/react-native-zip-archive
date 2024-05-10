import type { TurboModule } from "react-native/Libraries/TurboModule/RCTExport";
import { TurboModuleRegistry } from "react-native";

export interface Spec extends TurboModule {
  compress():void;
  decompress():void;
  compressWithPsd():void;
  decompressWithPsd():void;
}

export default TurboModuleRegistry.get<Spec>("RNZipArchive") as Spec | null;