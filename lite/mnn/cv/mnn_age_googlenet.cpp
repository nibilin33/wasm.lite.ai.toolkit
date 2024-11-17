//
// Created by DefTruth on 2021/11/27.
//

#include "mnn_age_googlenet.h"
#include "lite/utils.h"
#include <emscripten/bind.h>
#include <vector>
#include <iostream>
#include <cstring>

using mnncv::MNNAgeGoogleNet;
using namespace emscripten;

MNNAgeGoogleNet::MNNAgeGoogleNet(const std::string &_mnn_path, unsigned int _num_threads)
    : BasicMNNHandler(_mnn_path, _num_threads)
{
  initialize_pretreat();
}

inline void MNNAgeGoogleNet::initialize_pretreat()
{
  pretreat = std::shared_ptr<MNN::CV::ImageProcess>(
      MNN::CV::ImageProcess::create(
          MNN::CV::BGR,
          MNN::CV::RGB,
          mean_vals, 3,
          norm_vals, 3));
}

void MNNAgeGoogleNet::transform(const cv::Mat &mat)
{
  cv::Mat canvas;
  // (1,3,224,224)
  cv::resize(mat, canvas, cv::Size(input_width, input_height));
  pretreat->convert(canvas.data, input_width, input_height, canvas.step[0], input_tensor);
}

void MNNAgeGoogleNet::detect(const cv::Mat &mat, types::Age &age)
{
  if (mat.empty())
  {
    std::cout << "Input image is empty." << std::endl;
    return;
  }
  try
  {
    // 1. make input tensor
    std::cout << "Transforming input image..." << std::endl;
    this->transform(mat);
    // 2. inference
    std::cout << "Running inference..." << std::endl;
    mnn_interpreter->runSession(mnn_session);
    auto output_tensors = mnn_interpreter->getSessionOutputAll(mnn_session);
       // 打印 output_tensors 的内容
    std::cout << "Output tensors:" << std::endl;
    for (const auto& tensor_pair : output_tensors) {
        std::cout << "Key: " << tensor_pair.first << ", Shape: ";
        auto tensor = tensor_pair.second;
        for (int i = 0; i < tensor->dimensions(); ++i) {
            std::cout << tensor->length(i) << " ";
        }
        std::cout << std::endl;
    }
    // 3. fetch age.
    std::cout << "Fetching age..." << std::endl;
    auto device_age_logits_ptr = output_tensors.at("loss3/loss3_Y"); // (1,8)
    std::cout << "Age logits shape: " << std::endl;
    MNN::Tensor host_age_logits_tensor(device_age_logits_ptr, device_age_logits_ptr->getDimensionType());
    device_age_logits_ptr->copyToHostTensor(&host_age_logits_tensor);
    // 4. post-process
    std::cout << "Post-processing..." << std::endl;
    auto age_dims = host_age_logits_tensor.shape();
    unsigned int interval = 0;
    const unsigned int num_intervals = age_dims.at(1); // 8
    const float *pred_logits_ptr = host_age_logits_tensor.host<float>();

    auto softmax_probs = lite::utils::math::softmax<float>(pred_logits_ptr, num_intervals, interval);
    const float pred_age = static_cast<float>(age_intervals[interval][0] + age_intervals[interval][1]) / 2.0f;

    age.age = pred_age;
    age.age_interval[0] = age_intervals[interval][0];
    age.age_interval[1] = age_intervals[interval][1];
    age.interval_prob = softmax_probs[interval];
    age.flag = true;
    // 打印结果
    std::cout << "Predicted Age: " << age.age << std::endl;
    std::cout << "Age Interval: [" << age.age_interval[0] << ", " << age.age_interval[1] << "]" << std::endl;
    std::cout << "Interval Probability: " << age.interval_prob << std::endl;
    std::cout << "Detection Flag: " << (age.flag ? "true" : "false") << std::endl;
  }
  catch (const std::exception &e)
  {
    std::cerr << "Exception caught: " << e.what() << std::endl;
  }
}
// 辅助函数
int getRows(const cv::Mat &mat)
{
  return mat.rows;
}

int getCols(const cv::Mat &mat)
{
  return mat.cols;
}

// 创建 cv::Mat 对象的辅助函数
cv::Mat createMat(int rows, int cols, int type, uintptr_t buffer)
{
  void *dataPtr = reinterpret_cast<void *>(buffer);
  return cv::Mat(rows, cols, type, dataPtr);
}

// Updated `createMatFromArrayBuffer` to accept ArrayBuffer and allocate WebAssembly memory
cv::Mat createMatFromArrayBuffer(int rows, int cols, int type, val jsBuffer)
{
  size_t dataSize = jsBuffer["byteLength"].as<size_t>();
  uint8_t *dataPtr = reinterpret_cast<uint8_t *>(malloc(dataSize)); // Allocate memory
  if (!dataPtr)
  {
    throw std::runtime_error("Memory allocation failed in createMatFromArrayBuffer.");
  }

  // Copy data from JavaScript ArrayBuffer to WebAssembly memory
  val memoryView = val::global("Uint8Array").new_(val::module_property("HEAPU8"), reinterpret_cast<uintptr_t>(dataPtr), dataSize);
  memoryView.call<void>("set", jsBuffer);

  // Create cv::Mat using the allocated memory
  return cv::Mat(rows, cols, type, dataPtr);
}

// Free allocated memory for a cv::Mat
void freeMat(cv::Mat &mat)
{
  if (mat.data)
  {
    free(mat.data); // Free memory allocated in WebAssembly
    mat.data = nullptr;
  }
}

std::vector<unsigned int> getAgeInterval(const lite::types::AgeType &age)
{
  return std::vector<unsigned int>(age.age_interval, age.age_interval + 2);
}

void setAgeInterval(lite::types::AgeType &age, const std::vector<unsigned int> &interval)
{
  std::copy(interval.begin(), interval.end(), age.age_interval);
}
// 导出函数
EMSCRIPTEN_BINDINGS(mnn_age_googlenet)
{
  class_<MNNAgeGoogleNet>("MNNAgeGoogleNet")
      .constructor<const std::string &, unsigned int>()
      .function("detect", &MNNAgeGoogleNet::detect);

  function("createMat", &createMat, allow_raw_pointers());
  function("createMatFromArrayBuffer", &createMatFromArrayBuffer);
  function("freeMat", &freeMat);

  function("malloc", optional_override([](size_t size) -> uintptr_t
                                       { return reinterpret_cast<uintptr_t>(malloc(size)); }),
           allow_raw_pointers());
  function("free", optional_override([](uintptr_t ptr)
                                     { free(reinterpret_cast<void *>(ptr)); }),
           allow_raw_pointers());
}

EMSCRIPTEN_BINDINGS(cv)
{
  class_<cv::Mat>("Mat")
      .constructor<>()
      .function("rows", &getRows)
      .function("cols", &getCols);
}

EMSCRIPTEN_BINDINGS(lite_types)
{
  register_vector<unsigned int>("VectorUnsignedInt");
  class_<lite::types::AgeType>("AgeType")
      .constructor<>()
      .property("age", &lite::types::AgeType::age)
      .property("age_interval", &getAgeInterval, &setAgeInterval)
      .property("interval_prob", &lite::types::AgeType::interval_prob)
      .property("flag", &lite::types::AgeType::flag);
}