export const create : () => minizipNapi;

export class minizipNapi {
  compress(data:object):number;
  decompress(data:object):number;
  compressWithPsd(data:object):number;
  decompressWithPsd(data:object):number;
  compressWithEncryption(data:object):number;
  decompressWithEncryption(data:object):number;
}

