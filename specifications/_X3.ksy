meta:
  id: x3
  file-extension: _X3
  endian: le
  
seq:
  - id: type
    type: u1
  - id: version
    type: u1
  - id: header
    type: header
    if: version >= 2
    
  - id: frame_count
    type: u2le
    
  - id: submodel
    type: submodel
    repeat: expr
    repeat-expr: frame_count
    
  # Probably of all meshes
  - id: min_pos
    type: vec3
  - id: max_pos
    type: vec3
  - id: avg_pos # Just the average of the min and max values apparently
    type: vec3
  
  - id: unknown
    type: f4le
    
types:
  vec3:
    seq:
      - id: x
        type: f4le
      - id: y
        type: f4le
      - id: z
        type: f4le  
        
  header:
    seq:
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
        
  
  texture_v1:
    seq:
      - id: edge_start #An entire group of 3 values
        type: u4le
      - id: edge_count
        type: u4le
      - id: unknown
        size: 176
      - id: name
        type: str
        size: 256
        encoding: UTF-8
      #TODO: Check for the other maps  
      - id:  unknown2
        size: 104
  texture:
    seq:
      - id: unknown1 #Appears to be a single bit
        type: u4le
      - id: unknown_floats
        type: f4le
        repeat: expr
        repeat-expr: 16
      - id: unknown3
        type: u4le #Unknown format, sometimes 52.1 float
      - id: name
        type: str
        size: 204 # This is not confirmed, but due to the random data hard to verify
        encoding: UTF-8
      - id: bump
        type: str
        size: 204
        encoding: UTF-8
      - id: reflection
        type: str
        size: 204
        encoding: UTF-8
      - id:  unknown2
        size: 168
        
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
      - id: pos
        type: vec3
      - id: normal
        type: vec3
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
  bonedata1:
    seq:
      - id: content
        type: f4le
        repeat: expr
        repeat-expr: 16
  
  bonedata:
    seq:
      - id: count1
        type: u4le
      - id: count2
        type: u4le
      - id: count3
        type: u4le
        
      - id: unknown1
        type: bonedata1
        repeat: expr
        repeat-expr: count3
        
      - id: names
        type: str
        size: 256
        encoding: UTF-8
        repeat: expr
        repeat-expr: count3
    
        
  mesh:
    seq:
      - id: name
        type: str
        size: 256
        encoding: UTF-8
        
      - id: magic
        size: 4
        #contents: [0xD2, 0x01, 0x00, 0x00] #Always 466 / 0x000001D2
        
      - id: vertice_size
        type: u4le
        
      - id: texture_ref_count
        type: u4le
        
      - id: textures
        type: texture_v1
        if: _root.version == 1
        repeat: expr
        repeat-expr: texture_ref_count
        
      - id: texture_refferences
        type: texture_ref
        if: _root.version >= 2
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
        
      - id: bones
        type: bonedata
        if: vertice_size == 48

      - id: min_pos
        type: vec3
      - id: max_pos
        type: vec3
      - id: avg_pos # Just the average of the min and max values apparently
        type: vec3
        
      - id: unknown6
        type: f4le
        
      - id: unknown7
        type: u4le
        
      - id: unknown_data
        type: f4le
        repeat: expr
        repeat-expr: unknown7 * 26
       
        
  unknown_type2:
    seq:
      - id: step
        type: u4le
        
      - id: unknown2
        type: f4le
        
      - id: unknown3
        type: f4le
        
      - id: unknown4
        type: f4le
        
  unknown_type3:
    seq:
      - id: step
        type: u4le
        
      - id: unknown2
        type: f4le
        
      - id: unknown3
        type: f4le
        
      - id: unknown4
        type: f4le
        
      - id: unknown5
        type: f4le
        
  submodel:
    seq:
      - id: name
        type: str
        size: 260
        encoding: UTF-8
        
      - id: matrix
        type: f4le
        repeat: expr
        repeat-expr: 16
        
      - id: texture_count
        type: u2le
        if: _root.version >= 2
        
      - id: textures
        type: texture
        if: _root.version >= 2
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
        
      - id: start_a
        type: u4le
      - id: end_a
        type: u4le
      - id: start_b
        type: u4le
      - id: end_b
        type: u4le
      - id: start_c
        type: u4le
      - id: end_c
        type: u4le
        
      - id: unknown6
        type: u4le
      - id: unknown7
        type: u4le
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
        
      - id: unknown9
        type: u4le
        
      - id: count4
        type: u2le
        
      - id: children
        type: submodel
        repeat: expr
        repeat-expr: count4
        