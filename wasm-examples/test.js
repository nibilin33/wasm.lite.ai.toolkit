const fs = require('fs');
const path = require('path');
const cv = require('opencv.js');
const jpeg = require('jpeg-js');
const createModule = require('./mnn_age_googlenet.js');
const wasmBinary = fs.readFileSync(path.resolve(__dirname, 'mnn_age_googlenet.wasm'));
const modelPath = path.resolve(__dirname, 'age_googlenet.mnn');
createModule({
    wasmBinary: wasmBinary,
}).then((Module) => {
    console.log("Module.onRuntimeInitialized");
    try {
        if (typeof Module.MNNAgeGoogleNet === 'undefined') {
            console.error('MNNAgeGoogleNet is undefined');
            return;
        }
        const modelData = fs.readFileSync(modelPath);
        const modelFilePath = '/age_googlenet.mnn';
        Module.FS_createDataFile('/', 'age_googlenet.mnn', modelData, true, true);
        // 创建 MNNAgeGoogleNet 实例
        const numThreads = 1;
        const ageGoogleNet = new Module.MNNAgeGoogleNet(modelFilePath, numThreads);
        // new Module.MNNAgeGoogleNet(modelPath, numThreads);
        const imagePath = './test_lite_age_googlenet.jpg';
        // const mat = cv.imread(imagePath);
        const jpegData = fs.readFileSync(imagePath);
        const rawData = jpeg.decode(jpegData);
        const arrayBuffer = rawData.data;
        const mat = Module.createMatFromArrayBuffer(rawData.height, rawData.width, cv.CV_8UC4, new Uint8Array(arrayBuffer.buffer));
        // 创建 Age 对象
        const age = new Module.AgeType();
        age.age = 0;
        age.age_interval = new Module.VectorUnsignedInt();
        age.age_interval.push_back(0);
        age.age_interval.push_back(0);
        age.interval_prob = 0;
        age.flag = false;
    
        // 调用 detect 方法
        ageGoogleNet.detect(mat, age);
    
        // 输出结果
        console.log("Predicted Age:", age.age);
        console.log("Age Interval:", age.age_interval.get(0), age.age_interval.get(1));
        console.log("Interval Probability:", age.interval_prob);
        console.log("Detection Flag:", age.flag);  
        // 释放内存
        Module.freeMat(mat);
    } catch (error) {
        console.log("Error:", error);
    }
});
