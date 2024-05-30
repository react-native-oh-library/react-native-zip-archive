export const create : () => minizipNapi;

export class minizipNapi {
  compress(data:object,cb: (a: number) => void):number;
  decompress(data:object, cb: (a: number) => void):number;
  compressWithPsd(data:object,cb: (a: number) => void):number;
  decompressWithPsd(data:object,cb: (a: number) => void):number;
  isPasswordProtected(data:object):boolean;
  getUnCompressedSize(data:object):number;
}