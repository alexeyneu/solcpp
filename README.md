# solcpp - A fast Solana and Mango Markets C++ SDK.
A fast C++ SDK to interact with Solana and Mango Markets. The SDK also includes a collection of examples to send transactions to Solana and trade on Mango Markets.

## Building
  

The project uses [conan.io](https://conan.io/) to manage dependencies. Install Conan [here](https://conan.io/downloads.html).
```sh
$ git clone https://github.com/mschneider/solcpp.git
# Create a default profile or copy over the example for linux / macos
$ conan profile detect
$ cd solcpp && mkdir build && cd build
$ conan install .. --build=missing -s build_type=Release
$ cmake .. -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_BUILD_TYPE=Release
$ cmake --build .
$ ./bin/tests # Run tests
```

## Dependencies  
- C++17
- cmake
- conan io
## Notes
- _If you have issues building libcurl on gcc-9, try clang. See
  [issue](https://github.com/curl/curl/issues/4821)._

- _In addition, some dependencies were directly included and slightly modified to
  work well with the rest of the code base._
    - [bitcoin/libbase58](https://github.com/bitcoin/libbase58/tree/b1dd03fa8d1be4be076bb6152325c6b5cf64f678) [MIT]
    - [base64](https://stackoverflow.com/a/37109258/18072933) [CC BY-SA 4.0]
    - [cpp-utilities/fixed](https://github.com/eteran/cpp-utilities/blob/master/fixed/include/cpp-utilities/fixed.h)
      [MIT]

## Documentation
Mango Markets documentation can be found [here](https://docs.mango.markets/development-resources/client-libraries).
  
to make `id.json` use `f.py` from examples  
