Groove - Distributed storage for Zuri programs
==============================================

Copyright (c) 2024, Michael Frank

Overview
--------

Groove is a collection of software implementing distributed storage, data
synchronization, and structured query execution for Zuri programs.

Prerequisites
-------------

Groove requires the following dependencies to be installed on the system:
```
CMake version 3.27 or greater:      https://cmake.org
Conan version 2:                    https://conan.io
Tempo:                              https://github.com/msfrank/tempo
Lyric:                              https://github.com/msfrank/lyric
Zuri:                               https://github.com/msfrank/zuri
```

Groove also depends on the package recipes from Timbre and expects that the recipes
are exported into the conan2 package cache.

Quick Start
-----------

1. Navigate to the repository root.
1. Build and install Groove using Conan:
  ```
conan create . --build=missing
  ```

Licensing
---------

The software libraries in this package are licensed under the terms of the BSD
3-clause license. The text of the license is contained in the file `LICENSE.txt`.

The software programs in this package are licensed separately under the terms of the
Affero GNU Public License version 3.0 (AGPL-3.0) or later. The text of the license
is contained in the file `bin/LICENSE.txt`.