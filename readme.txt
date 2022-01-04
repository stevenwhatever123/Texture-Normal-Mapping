# README for code distributed for use in COMP 5821M 2021-22
#
#

To compile, execute the following on feng-linux:
module add qt/5.13.0
qmake -project QT+=opengl
qmake
make


To run the program use the following command:
./Assignment_2 <model>
e.g.
./Assignment_2 ./models/bumpysphere.obj


The generated texture and normal map will be in the output folder.
The generated files will be named <object name>_texture.ppm and <object name>_normal.ppm
