# MNN WebAssembly Integration

## Project Introduction

This project aims to use Emscripten to convert parts of [MNN](https://github.com/alibaba/MNN) to WebAssembly (WASM) for use in JavaScript environments. By converting MNN to WASM, deep learning models can be efficiently run in both browser and Node.js environments.

## Original Project Introduction

This project is based on [Lite.Ai.ToolKit](https://github.com/DefTruth/lite.ai.toolkit). Lite.Ai.ToolKit is a lightweight AI toolkit designed to provide efficient and easy-to-use deep learning model inference capabilities. The toolkit supports multiple deep learning frameworks and model formats, suitable for various AI application scenarios.
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

## Features

- Convert parts of MNN to WebAssembly, supporting calls in browser and Node.js environments.
- Provide sample code demonstrating how to use the converted MNN model in JavaScript.
- Support integration of OpenCV with MNN for image processing and model inference.

## Directory Structure

```
.
├── CMakeLists.txt          # CMake configuration file for Emscripten
├── README.md               # Project introduction
├── lite                    # Code processed for MNN
│   ├── mnn
│   │   ├── core
│   │   │   └── mnn_handler.cpp
│   │   └── cv
│   │       └── mnn_age_googlenet.cpp
├── thirdparty              # Third-party libraries
│   ├── MNN
│   └── opencv
└── wasm-examples           # Example code for calling the converted code
    ├── mnn_age_googlenet.js
    └── mnn_age_googlenet.wasm
    ├── test.js             # Node.js example code
│   └── test_lite_age_googlenet.jpg  # Example image
```

## Installation and Configuration

### Environment Preparation

1. Install [Emscripten](https://emscripten.org/docs/getting_started/downloads.html).
2. Install [OpenCV](https://opencv.org/) and [MNN](https://github.com/alibaba/MNN).

### Compilation Steps

1. Clone this project:

```sh
git clone https://github.com/nibilin33/wasm.lite.ai.toolkit.git
cd wasm.lite.ai.toolkit
```

2. Compile OpenCV wasm:

```sh
git clone https://github.com/opencv/opencv.git
cd opencv
mkdir build
cd build
emcmake python platforms/js/build_js.py build  -DCMAKE_INSTALL_PREFIX=/xx/thirdparty/opencv  -DBUILD_TESTS=OFF \
  -DBUILD_PERF_TESTS=OFF -DBUILD_opencv_imgcodecs=ON
emmake make install
```

3. Compile MNN wasm:

```sh
git clone https://github.com/alibaba/MNN.git
cd MNN
mkdir build
cd build
emcmake cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=/xx/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake ..
emmake make
cp libMNN.a  /xx/thirdparty/MNN/lib/
cp -r ../include /xx/thirdparty/MNN/
```

4. Compile the project:

```sh
sh ./build-wasm.sh
```

## Usage Example

### Node.js Example

1. Install dependencies:

```sh
cd wasm-examples
npm install opencv.js
```

2. Run the example code:

```sh
node test.js
```

```output result 
Predicted Age: 28.5
Age Interval: 25 32
Interval Probability: 0.27939242124557495
Detection Flag: true
```

The example code will load the MNN model and use OpenCV to process image data for age detection.

## Model Transfrom
[free onnx model](https://github.com/onnx/models/tree/main)
```sh
pip install -U MNN
mnnconvert -f ONNX --modelFile age_googlenet.onnx --MNNModel age_googlenet.mnn  --bizCode age_googlenet
```

## Emscripten bindings Learning   
[opencv](https://github.com/opencv/opencv/blob/4.x/modules/js/src/core_bindings.cpp)

