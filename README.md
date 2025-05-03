Test compiler
```
clang++ -Wextra -Werror -std=c++20 nuo.cc -o build/nuo && ./build/nuo --test
```

Compile & run playground code
```
./build/nuo --compile playground/main.nuo && \
clang -Wextra -Werror -std=c99 playground/main.nuo.c -o build/main && \
./build/main
```