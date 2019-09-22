# skytrails-replacer
Attempt on reverse engineering the Trails in the Sky 3D model files so they can be replaced with higher quality models


### xxViewer

Decoder and viewer for _X3 files from Second Chapter. First Chapter support will probably come soon

**Compile**
Run `cmake .` followed by `make`.

**Dependencies**

- png++ and libpng
- GLFW
- glm

**Usage*

`./trailsViewer INPUT_X3_FILE`

Or simply drag-and-drop a file onto the executable. A folder called `images` should exist in the working directory and contain the game images in PNG format. Use the option in `falcnvrt` to generate the PNG files.

### specifications

Contains kaitai-struct files for the formats. Can be used to generate parsers in different languages and to inspect files using the kaitai struct WebIDE.