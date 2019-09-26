# Some of the info here is taken from uyjulian findings 
meta:
  id: dir
  file-extension: dir
  endian: le


seq:
  - id: magic1
    contents: [0x4c, 0x42]
  - id: type
    type: u1
  - id: magic2
    contents: [0x44, 0x49, 0x52, 0x1a, 0x00]
  - id: file_count
    type: u4le
    
  - id: unknown1
    type: u4le
    
  - id: name
    type: entry
    repeat: expr
    repeat-expr: file_count
    
types:
  entry:
    seq:
      - id: filename #Filename is padded with ' ' (0x20) before extension
        size: 12
        type: str
        encoding: UTF-8
      - id: timestamp2
        type: u4le
      - id: compress_size
        type: u4le
      - id: uncompress_size
        type: u4le
      - id: unknown
        type: u4le
      - id: timestamp
        type: u4le
      - id: offset
        type: u4le