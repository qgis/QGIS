# C++ Wrapper for ODBC

odbc-cpp-wrapper is an object-oriented C++-wrapper of the ODBC API. It takes
care of
 - managing the lifetime of ODBC resources,
 - allocating and managing resources needed for ODBC operations and
 - converting ODBC errors to exceptions and throwing them.

The odbc-cpp-wrapper API attempts to make usage of ODBC as simple as possible.
The API was designed to make wrong usage almost impossible and to ensure proper
object lifetime management.

odbc-cpp-wrapper was originally developed for exchanging spatial data with
databases. It focuses on batch operations of variable-sized data, which is not
very well supported by other ODBC wrappers.

## Requirements

To build odbc-cpp-wrapper you need
 - A C++-11-standard-compliant compiler
 - [The Git command line client](https://git-scm.com/)
 - [CMake 3.12 or newer](https://cmake.org/)

On Linux platforms you additionally need
 - [unixODBC](http://www.unixodbc.org/)

To generate the API's documentation, you need
 - [Doxygen 1.8.0 or later](http://www.doxygen.nl/)


## Building and Installation

### Linux

- Clone the repository:
    ```
    git clone https://github.com/SAP/odbc-cpp-wrapper.git
    ```

- Create a build directory and change to it:
    ```
    mkdir odbc-cpp-wrapper/build && cd odbc-cpp-wrapper/build
    ```

- Create the makefiles with CMake:
    ```
    cmake ..
    ```

- Build the library:
    ```
    make -j <number of parallel build jobs>
    ```

    The build will create a shared library `libodbccpp.so` and a static library `libodbccpp_static.a`.

- To build the documentation (optional):
    ```
    make doc
    ```

    The mainpage of the documentation can be found at `doc/html/index.html`.

- Install the library:
    ```
    sudo make install
    ```

    This will install the library and header files. CMake will install them to `usr/local/lib` and `usr/local/include` by default. If you prefer different locations, you can set CMake's install prefix to a different path. See
https://cmake.org/cmake/help/latest/variable/CMAKE_INSTALL_PREFIX.html for details.


### Windows

- Clone the repository:
    ```
    git clone https://github.com/SAP/odbc-cpp-wrapper.git
    ```

- Create a build directory and change to it:
    ```
    mkdir odbc-cpp-wrapper\build && cd odbc-cpp-wrapper\build
    ```

#### Visual Studio 2015 and later

- Generate a Visual Studio solution
    ```
    cmake ..
    ```

    You can then open the `odbccpp.sln` file and build the desired targets in Visual Studio.

#### MSBuild (nmake)

- Start the Visual Studio Native Tools Command Prompt for the desired target and change the directory to the build directory. Create the makefiles for nmake:
    ```
    cmake -G "NMake Makefiles" ..
    ```

    > Optionally you can use CMAKE_BUILD_TYPE to define if you'd like to build a Debug or Release build. See
https://cmake.org/cmake/help/latest/variable/CMAKE_BUILD_TYPE.html for details.

- Build the library:
    ```
    nmake
    ```

    The build will create a dynamic link library `odbccpp.dll` and a static library `odbccpp_static.lib`.

- Build the documentation (optional):
    ```
    nmake doc
    ```

    The mainpage of the documentation can be found at `doc\html\index.html`.

- Install the library (optional):
    ```
    nmake install
    ```

    This will install the library and header files. CMake will install them to `C:\Program Files\odbccpp` by default. If you prefer a different location, you can set CMake's install prefix to a different path. See
https://cmake.org/cmake/help/latest/variable/CMAKE_INSTALL_PREFIX.html for details.


## Using the library

You can just link against the shared/dynamic or the static library. If you are linking against the static library, you have to additionally define `ODBC_STATIC` when compiling.

Usage of the library should be pretty straight-forward if you are familiar with ODBC and/or other database connectors.

### Example

The following code gives an example how working with odbc-cpp-wrapper looks like. It connects to a database, batch inserts two rows and executes a query.

```cpp
#include <iostream>
#include <odbc/Connection.h>
#include <odbc/Environment.h>
#include <odbc/Exception.h>
#include <odbc/PreparedStatement.h>
#include <odbc/ResultSet.h>

int main()
{
    try
    {
        odbc::EnvironmentRef env = odbc::Environment::create();

        odbc::ConnectionRef conn = env->createConnection();
        conn->connect("DSN", "user", "pass");
        conn->setAutoCommit(false);

        odbc::PreparedStatementRef psInsert =
            conn->prepareStatement("INSERT INTO TAB (ID, DATA) VALUES (?, ?)");
        psInsert->setInt(1, 101);
        psInsert->setCString(2, "One hundred one");
        psInsert->addBatch();
        psInsert->setInt(1, 102);
        psInsert->setCString(2, "One hundred two");
        psInsert->addBatch();
        psInsert->executeBatch();
        conn->commit();

        odbc::PreparedStatementRef psSelect =
            conn->prepareStatement("SELECT ID, DATA FROM TAB WHERE ID > ?");
        psSelect->setInt(1, 100);
        odbc::ResultSetRef rs = psSelect->executeQuery();
        while (rs->next())
        {
            std::cout << rs->getInt(1) << ", " << rs->getString(2) << std::endl;
        }
    }
    catch (const odbc::Exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}
```

## How to obtain support

If you experience issues with using the library, please file a report in the GitHub bug tracking system.


## License

Copyright (c) 2019 SAP SE or an SAP affiliate company. All rights reserved.

This file is licensed under the Apache Software License 2.0 except as noted otherwise in the [LICENSE](LICENSE) file.
