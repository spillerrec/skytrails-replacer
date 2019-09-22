meta:
  id: x3
  file-extension: _X3
  endian: le
  
seq:
  - id: header
    type: header
    
  - id: submodel
    type: submodel
    repeat: expr
    repeat-expr: header.unknown12
    
  
  - id: file_end_test
    type: f4le
    repeat: expr
    repeat-expr: 10
    
    
types:
  bone:
    seq:
      - id: name
        type: str
        size: 260
        encoding: UTF-8
        
      - id: matrix
        type: f4le
        repeat: expr
        repeat-expr: 16
        
      - id: unknown1
        type: u4le
        
      - id: unknown2
        type: u1
        
      - id: unknown3
        type: u4le
        repeat: expr
        repeat-expr: 8
        
      - id: count1
        type: u4le
        
      - id: block1
        type: bone_type1
        repeat: expr
        repeat-expr: count1
        
      - id: count2
        type: u4le
        
      - id: block2
        type: bone_type2
        repeat: expr
        repeat-expr: count2
        
      - id: count3
        type: u4le
        
      - id: block3
        type: bone_type1
        repeat: expr
        repeat-expr: count1
        
      - id: unknown4
        type: u4le
        
      - id: unknown5
        type: u2le
        
  bone_type1:
    seq:
      - id: unknown1
        type: u4le
      - id: unknown2
        type: f4le
      - id: unknown3
        type: f4le
      - id: unknown4
        type: f4le
        
  bone_type2:
    seq:
      - id: unknown1
        type: u4le
      - id: unknown2
        type: f4le
      - id: unknown3
        type: f4le
      - id: unknown4
        type: f4le
      - id: unknown5
        type: f4le
  header:
    seq:
      - id: type
        type: u1
      - id: count
        type: u1
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
      - id: edge_start
        type: u4le
      - id: edge_count
        type: u4le
      - id: start2
        type: u4le
      - id: count2
        type: u4le
      - id: unknown
        type: u4le
        repeat: expr
        repeat-expr: 24
      - id: order
        type: u4le
      - id: unknown_format
        size: 4
        
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
        
  vertex2:
    seq:
      - id: base
        type: vertex
        
      - id: unknown1
        type: f4le
        
      - id: unknown2
        type: f4le
    
        
  mesh:
    seq:
      - id: name
        type: str
        size: 256
        encoding: UTF-8
        
      - id: unknown1
        type: u4le
        
      - id: vertice_size
        type: u4le
        
      - id: texture_ref_count
        type: u4le
        
      - id: texture_refferences
        type: texture_ref
        repeat: expr
        repeat-expr: texture_ref_count
        
      - id: vertices_count
        type: u4le
        
      - id: vertices40
        type: vertex
        if: vertice_size == 40
        repeat: expr
        repeat-expr: vertices_count
        
      - id: vertices48
        type: vertex2
        if: vertice_size == 48
        repeat: expr
        repeat-expr: vertices_count
        
      - id: edge_count
        type: u4le
        
      - id: edges
        type: u2le
        repeat: expr
        repeat-expr: edge_count
        
      - id: unknown2
        type: f4le
      - id: unknown3
        type: f4le
      - id: unknown4
        type: f4le
        
      - id: unknown5
        type: f4le
        repeat: expr
        repeat-expr: 6
        
      - id: unknown6
        type: f4le
        
      - id: unknown7
        type: f4le
       
      # This doesn't seem to be correct 
      #- id: block
      #  size: 104
      #  repeat: expr
      #  repeat-expr: unknown6
  unknown_type_test:
    seq:
      - id: unknown1
        type: f4le
        repeat: expr
        repeat-expr: 26
        
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
        size: 32
        
      - id: count
        type: u4le
        
      - id: unknown2
        size: 12
        
      - id: unknown_block1
        type: unknown_type2
        repeat: expr
        repeat-expr: count
        
      #- id: count2
      #  type: u4le
        
      - id: unknown_block2
        type: unknown_type3
        repeat: expr
        repeat-expr: count
        
      #- id: count3
      #  type: u4le
        
      - id: unknown_block3
        type: unknown_type2
        repeat: expr
        repeat-expr: count
        
  submodel:
    seq:
      - id: skipper
        type: u1
        repeat: until
        repeat-until: _ == 70
      - id: name
        type: str
        size: 259
        encoding: UTF-8
        
      - id: matrix
        type: f4le
        repeat: expr
        repeat-expr: 16
        
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
        
      - id: unknown_block1
        type: unknown_type
        if: unknown1 == 1
        
      - id: padding
        size: 48
        if: mesh_count == 0 and unknown1 == 0
        
      - id: count4
        type: u2le
        
      - id: children
        type: submodel
        repeat: expr
        repeat-expr: count4
        