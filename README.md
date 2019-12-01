# cmake_embed
CMake function to embed a file statically into a binary

# Usage
```cmake
include(cmake_embed/cmake_embed.cmake)
...
...
add_executable(foo foo.c)
embed_file(foo resource/logo.png)
# embed_file takes optional LIBNAME and SYMNAME vars.
# LIBNAME controls the name of the target and base name for header file.
# SYMNAME controls the name of the symbol in the header file.
```

```c
#include <logo.h>
render_logo(logo_png, logo_png_size);
```
