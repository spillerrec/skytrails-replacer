# skytrails-replacer
Attempt on reverse engineering the Trails in the Sky 3D model files so they can be replaced with higher quality models

### xxViewer

![Example screenshot](/preview.jpg?raw=true)

Decoder and viewer for `X` files from First Chapter and `_X3` files from Second Chapter.

**Compile**
Run `cmake .` followed by `make`.

**Dependencies**

- png++ and libpng
- GLFW
- glm

**Usage**

`./trailsViewer INPUT_X3_FILE`

Or simply drag-and-drop a file onto the executable. A folder called `images` should exist in the working directory and contain the game images in PNG format. Use the option in `falcnvrt` to generate the PNG files.
To move around in the scene, hold CTRL or SHIFT while moving the mouse to rotate/zoom.

### Progress / Support

Files from First and Second chapter works.

This is done without searching for markers in the files which is the approch used in all other avaiable code. The structure is fully parsed though a lot about the format is still not understood. Some of the conditions might be incorrect.

- First chapter: 949 of 952 files parses (99.7%)
- Second chapter: 1598 of 1598 files parses (100%)
- Third chapter: Unknown

### specifications

Contains kaitai-struct files for the formats. Can be used to generate parsers in different languages and to inspect files using the kaitai struct WebIDE.
