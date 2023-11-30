# PagesManager

一个极简并基于Qt的多级页面管理器, 仅有一个`pages_manager.hpp`文件.

## Build && Install

```shell
$ git clone https://github.com/xxxxxx.git pyembed
$ cd pages_manager && mkdir build && cd build
$ cmake .. -DCMAKE_PREFIX_PATH:PATH="G:\local\libraries\qt\5.15.2\5.15.2\msvc2019"
  or
$ cmake -G "Visual Studio 16 2019" -A Win32 .. -DCMAKE_PREFIX_PATH:PATH="G:\local\libraries\qt\5.15.2\5.15.2\msvc2019"
$ cmake --build .
```