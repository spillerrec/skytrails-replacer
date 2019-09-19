meta:
  id: x3
  file-extension: _X3
  endian: le
  
seq:
  - id: file
    type: main
  - id: file2
    type: main
  #- id: header2
  #  type: main
  #- id: header3
  #  type: submodel
  
  #- id: mesh_thing_count
  #  type: u2le
    
  #- id: mesh_thing
  #  type: mesh
  #  repeat: expr
  #  repeat-expr: 1000
  
  - id: file_end_test
    type: f4le
    repeat: expr
    repeat-expr: 10
    
    
types:
  main:
    seq:
      - id: header
        type: header
        
      - id: submodel
        type: submodel
        if: header.unknown12 != 0
        #repeat: expr
        #repeat-expr: header.unknown12

  header:
    seq:
      - id: magic #Err, probably not?
        type: u2le
      - id: unknown1
        type: f4le
      - id: unknown2
        type: f4le
      - id: unknown3
        type: f4le
      - id: unknown4
        type: f4le
      - id: unknown5
        type: f4le
      - id: unknown6
        type: f4le
      - id: unknown7
        type: f4le
      - id: unknown8
        type: f4le
      - id: unknown9
        type: f4le
      - id: unknown10
        type: f4le
      - id: unknown11
        type: f4le
      - id: unknown12
        type: u2le

  matrix:
    seq:
      - id: values
        type: f4le
        repeat: expr
        repeat-expr: 16
        
  texture:
    seq:
      - id: unknown
        size: 72
      - id: name
        type: str
        size: 256
        encoding: UTF-8
      - id:  unknown2
        size: 524
        
  texture_ref:
    seq:
      - id: start1
        type: u4le
      - id: count1
        type: u4le
      - id: start2
        type: u4le
      - id: count2
        type: u4le
      - id: unknown
        type: u4le
        repeat: expr
        repeat-expr: 26
        
  vertex:
    seq:
      - id: x
        type: f4le
      - id: y
        type: f4le
      - id: z
        type: f4le
      - id: normal_x
        type: f4le
      - id: normal_y
        type: f4le
      - id: normal_z
        type: f4le
      - id: unknown1
        type: f4le
      - id: unknown2
        type: f4le
      - id: u
        type: f4le
      - id: v
        type: f4le
        
  mesh:
    seq:
      - id: name
        type: str
        size: 256
        encoding: UTF-8
        
      - id: unknown1
        type: u4le
        
      - id: unknown2
        type: u4le
        
      - id: texture_ref_count
        type: u4le
        
      - id: texture_refferences
        type: texture_ref
        repeat: expr
        repeat-expr: texture_ref_count
        
      - id: vertices_count
        type: u4le
        
      - id: vertices
        type: vertex
        repeat: expr
        repeat-expr: vertices_count
        
      - id: edge_count
        type: u4le
        
      - id: edges
        type: u2le
        repeat: expr
        repeat-expr: edge_count
        
      - id: unknown3
        type: f4le
        repeat: expr
        repeat-expr: 10
        
      - id: unknown4
        type: u4le
        
      - id: block
        size: 104
        repeat: expr
        repeat-expr: unknown4
        
  unknown_type2:
    seq:
      - id: unknown1
        type: f4le
        
      - id: unknown2
        type: f4le
        
      - id: unknown3
        type: f4le
        
      - id: unknown4
        type: f4le
        
  unknown_type3:
    seq:
      - id: unknown1
        type: u4le
        
      - id: unknown2
        type: u4le
        
      - id: unknown3
        type: u4le
        
      - id: unknown4
        type: u4le
        
      - id: unknown5
        type: f4le
        
  unknown_type:
    seq:
      - id: unknown1
        size: 30
        
      - id: count
        type: u4le
        
      - id: unknown_block1
        type: unknown_type2
        repeat: expr
        repeat-expr: count
        
      - id: count2
        type: u4le
        
      - id: unknown_block2
        type: unknown_type3
        repeat: expr
        repeat-expr: count2
        
      - id: count3
        type: u4le
        
      - id: unknown_block3
        type: unknown_type2
        repeat: expr
        repeat-expr: count3
        
      - id: unknown2
        type: u4le
        
      - id: count4
        type: u2le
        
      - id: extra_model #Probably not the correct way to do it
        type: submodel
        if: count4 > 0
        #repeat: expr
        #repeat-expr: count4
        
  submodel:
    seq:
      - id: name
        type: str
        size: 260
        encoding: UTF-8
        
      - id: matrix
        type: matrix
        
      - id: texture_count
        type: u2le
        
      - id: textures
        type: texture
        repeat: expr
        repeat-expr: texture_count
        
      - id: mesh_count
        type: u2le
        
      - id: meshes
        type: mesh
        repeat: expr
        repeat-expr: mesh_count
      
      - id: unknown1
        type: u1
      
      - id: unknown2
        type: u2le
        
      - id: unknown_block1
        type: unknown_type
        if: unknown1 == 1
        