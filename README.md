# OpenGL With Refpages
A [GLEW](http://glew.sourceforge.net/) extension that includes the [Khronos OpenGL Reference Pages](https://www.khronos.org/registry/OpenGL-Refpages/gl4/).

## What is GLWR?
GLWR is an attempt to provide C/C++ header files for OpenGL functions that include relevant documentation about the functions of OpenGL. It uses GLEW to load the functions.

GLWR generates header files for each OpenGL Reference Page. It redefines the OpenGL functions with proper parameter names. For each function, it includes documentation in [Doxygen](https://www.doxygen.nl/index.html) format.

## Building GLWR headers

To build the GLWR headers and use them, you need:

- [GLEW](http://glew.sourceforge.net/) 2.1.0. If newer versions become available, GLWR will need to be tested for them.
- [CMake](https://cmake.org/) 3.13 or higher 
- [ctre](https://github.com/hanickadot/compile-time-regular-expressions)
- [RapidXml](http://rapidxml.sourceforge.net/)
- GCC 9+ with C++20 support

### Build steps
- Clone the Git repository
    - `git clone https://github.com/Rheel17/glwr.git`
    - `cd glwr`
- Run CMake
    - `mkdir build`
    - `cmake .. <options>`
- Build
    - `cmake --build .`

The headers can now be found in `/build/include/GL`. Find your `glew.h` header file and copy `glwr.h` and `func/*.h` to the same directory.

### Options
Several options can be given in the CMake build step.

#### Include/exclude documentation sections
Use the following options to include/exclude documentation sections:

| Option                       | Description                                           | Default |
|------------------------------|-------------------------------------------------------|:-------:|
| `-DLINK=<ON/OFF>`            | A link to the Khronos site                            |   `ON`  |
| `-DBRIEF=<ON/OFF>`           | A short description about the function                |   `ON`  |
| `-DVERSION=<ON/OFF>`         | The OpenGL version where this function first appeared |   `ON`  |
| `-DDESCRIPTION=<ON/OFF>`     | A long description about the function                 |  `OFF`  |
| `-DEXAMPLES=<ON/OFF>`        | Examples on how to use the function                   |  `OFF`  |
| `-DNOTES=<ON/OFF>`           | Notes about the function                              |  `OFF`  |
| `-DPARAMETERS=<ON/OFF>`      | Information about the parameters                      |   `ON`  |
| `-DERRORS=<ON/OFF>`          | Errors that the function can generate                 |   `ON`  |
| `-DASSOCIATED_GETS=<ON/OFF>` | Associated `get`s                                     |  `OFF`  |
| `-DSEE_ALSO=<ON/OFF>`        | A 'See Also' section                                  |  `OFF`  |
| `-DCOPYRIGHT=<ON/OFF>`       | A copyright notice                                    |   `ON`  |

Note that the documentation sections correspond to the sections in the [OpenGL Refpages](https://www.khronos.org/registry/OpenGL-Refpages/gl4/). Not all sections will be available for all functions. 

***Warning:** enabling some sections (in particular the 'description' section) will result in some very large (>100kB) header files. Use with caution!*  

#### Verbose output
Enable verbose output using `-DVERBOSE=ON`. If `VERBOSE` is turned on, the generator code will output the header file names as they are generated.

## Usage
Simply `#include <GL/glwr.h>` instead of `GL/glew.h`.

## License/Copyright
The GLWR header generator source code is Copyright (c) 2020 Levi van Rheenen. It falls under the MIT license. See `LICENSE.md` for more information.

The generated header files fall under the licenses of their respective documentation. For more information, go to the [OpenGL Refpages repository](https://github.com/KhronosGroup/OpenGL-Refpages#licenses) for more information.
