#!/bin/bash

# 创建构建目录
rm -rf build
mkdir -p build
cd build

# 配置和编译项目
emcmake cmake ..
emmake make

# 检查构建是否成功
if [ $? -ne 0 ]; then
    echo "Build failed"
    exit 1
fi

# 拷贝生成的 .js 和 .wasm 文件到 wasm-examples 目录
cp mnn_age_googlenet.js ../wasm-examples/
cp mnn_age_googlenet.wasm ../wasm-examples/

echo "Build and copy completed successfully"