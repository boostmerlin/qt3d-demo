## 快速开始
1. 获取pMaxwell=>logs目录下的dmp文件,路径为dmp_file
2. 生成符号文件，windows下通过pMaxwell.pdb生成符号文件，linux下通过pMaxwell生成符号文件
    ```bash
        mkdir dbg-syms
        bin/breakpad/make_symbols.sh dbg-syms cmake-build-release/pMaxwell pMaxwell.pdb
    ```
    生成pdb对应的符号文件，路径为dbg-syms
3. 使用dmp文件和符号文件合成crash日志
```bash
    bin/breakpad/stackwalk.sh dmp_file dbg-syms/ > crash.log
```

## make_symbols.sh
本地生成需要保证环境和release机器上的一致

支持的格式：  
`make_symbols.sh dbg-syms cmake-build-release/pMaxwell pMaxwell.pdb`

`make_symbols.sh dbg-syms cmake-build-release/pMaxwell *.pdb`

`make_symbols.sh dbg-syms cmake-build-release/pMaxwell pMaxwell.pdb JKQT*.pdb`

## replace_symbols
将符号文件放到指定目录下,使得stackwalk.sh能够找到符号文件
```bash
    bin/breakpad/replace_symbols dbg-syms pMaxwell.sym
```

## dump_syms.exe
windows上依赖msdia140.dll，需要注册先执行init.bat