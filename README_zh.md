# MNN WebAssembly Integration

## 项目简介

本项目旨在利用 Emscripten 将 [MNN](https://github.com/alibaba/MNN) 部分转为 WebAssembly (WASM)，以便在 JavaScript 环境中调用。通过将 MNN 转为 WASM，可以在浏览器和 Node.js 环境中高效地运行深度学习模型。

## 原始项目介绍
本项目基于 [Lite.Ai.ToolKit](https://github.com/DefTruth/lite.ai.toolkit) 进行改造。Lite.Ai.ToolKit 是一个轻量级的 AI 工具包，旨在提供高效、易用的深度学习模型推理能力。该工具包支持多种深度学习框架和模型格式，适用于各种 AI 应用场景。
<div align='center'>
  <img src='https://github.com/DefTruth/lite.ai.toolkit/assets/31974251/5b28aed1-e207-4256-b3ea-3b52f9e68aed' height="80px" width="80px">
  <img src='https://github.com/DefTruth/lite.ai.toolkit/assets/31974251/28274741-8745-4665-abff-3a384b75f7fa' height="80px" width="80px">
  <img src='https://github.com/DefTruth/lite.ai.toolkit/assets/31974251/c802858c-6899-4246-8839-5721c43faffe' height="80px" width="80px">
  <img src='https://github.com/DefTruth/lite.ai.toolkit/assets/31974251/20a18d56-297c-4c72-8153-76d4380fc9ec' height="80px" width="80px">
  <img src='https://github.com/DefTruth/lite.ai.toolkit/assets/31974251/f4dd5263-8514-4bb0-a0dd-dbe532481aff' height="80px" width="80px">
  <img src='https://github.com/DefTruth/lite.ai.toolkit/assets/31974251/b6a431d2-225b-416b-8a1e-cf9617d79a63' height="80px" width="80px">
  <img src='https://github.com/DefTruth/lite.ai.toolkit/assets/31974251/84d3ed6a-b711-4c0a-8e92-a2da05a0d04e' height="80px" width="80px">
  <img src='https://github.com/DefTruth/lite.ai.toolkit/assets/31974251/157b9e11-fc92-445b-ae0d-0d859c8663ee' height="80px" width="80px">
  <img src='https://github.com/DefTruth/lite.ai.toolkit/assets/31974251/ef0eeabe-6dbe-4837-9aad-b806a8398697' height="80px" width="80px">  
</div> 

## 特性

- 将 MNN 部分 转为 WebAssembly，支持在浏览器和 Node.js 环境中调用。
- 提供示例代码，展示如何在 JavaScript 中使用转换后的 MNN 模型。
- 支持 OpenCV 与 MNN 的集成，便于图像处理和模型推理。

## 目录结构

```
.
├── CMakeLists.txt          # Emscripten 的 CMake 配置文件
├── README.md               # 项目简介
├── lite                    # MNN 本次处理的代码
│   ├── mnn
│   │   ├── core
│   │   │   └── mnn_handler.cpp
│   │   └── cv
│   │       └── mnn_age_googlenet.cpp
├── thirdparty              # 第三方库
│   ├── MNN
│   └── opencv
└── wasm-examples           # 转换后调用示例代码
    ├── mnn_age_googlenet.js
    └── mnn_age_googlenet.wasm
    ├── test.js             # Node.js 示例代码
│   └── test_lite_age_googlenet.jpg  # 示例图像
```

## 安装与配置

### 环境准备

1. 安装 [Emscripten](https://emscripten.org/docs/getting_started/downloads.html)。
2. 安装 [OpenCV](https://opencv.org/) 和 [MNN](https://github.com/alibaba/MNN)。

### 编译步骤

1. 克隆本项目：

```sh
git clone https://github.com/nibilin33/wasm.lite.ai.toolkit.git
cd wasm.lite.ai.toolkit
```

2. 编译 OpenCV wasm版：

```sh
git clone https://github.com/opencv/opencv.git
cd opencv
mkdir build
cd build
emcmake python platforms/js/build_js.py build  -DCMAKE_INSTALL_PREFIX=/xx/thirdparty/opencv  -DBUILD_TESTS=OFF \
  -DBUILD_PERF_TESTS=OFF -DBUILD_opencv_imgcodecs=ON
emmake make install
```

3. 编译 MNN wasm版：

```sh
git clone https://github.com/alibaba/MNN.git
cd MNN
mkdir build
cd build
emcmake cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=/xx/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake ..
emmake make
cp libMNN.dylib  /xx/thirdparty/MNN/lib/
cp -r ../include /xx/thirdparty/MNN/
```

4. 编译项目：

```sh
sh ./build-wasm.sh
```

## 使用示例

### Node.js 示例

1. 安装依赖：

```sh
cd wasm-examples
npm install opencv.js
```

2. 运行示例代码：

```sh
node test.js
```
```output result 
Predicted Age: 28.5
Age Interval: 25 32
Interval Probability: 0.27939242124557495
Detection Flag: true
```
示例代码将加载 MNN 模型，并使用 OpenCV 处理图像数据，进行年龄检测。


## 模型转换

```sh
pip install -U MNN
mnnconvert -f ONNX --modelFile age_googlenet.onnx --MNNModel age_googlenet.mnn  --bizCode age_googlenet
```