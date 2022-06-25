//
// Created by DefTruth on 2022/6/19.
//

#include "mnn_portrait_seg_extremec3net.h"
#include "lite/utils.h"

using mnncv::MNNPortraitSegExtremeC3Net;

MNNPortraitSegExtremeC3Net::MNNPortraitSegExtremeC3Net(
    const std::string &_mnn_path, unsigned int _num_threads)
    : BasicMNNHandler(_mnn_path, _num_threads)
{ initialize_pretreat(); }

void MNNPortraitSegExtremeC3Net::initialize_pretreat()
{
  pretreat = std::shared_ptr<MNN::CV::ImageProcess>(
      MNN::CV::ImageProcess::create(
          MNN::CV::BGR,
          MNN::CV::BGR,
          mean_vals, 3,
          norm_vals, 3
      )
  );
}

void MNNPortraitSegExtremeC3Net::transform(const cv::Mat &mat_rs)
{
  pretreat->convert(mat_rs.data, input_width, input_height,
                    mat_rs.step[0], input_tensor);
}

void MNNPortraitSegExtremeC3Net::resize_unscale(const cv::Mat &mat, cv::Mat &mat_rs,
                                                int target_height, int target_width,
                                                PortraitSegExtremeC3NetScaleParams &scale_params)
{
  if (mat.empty()) return;
  int img_height = static_cast<int>(mat.rows);
  int img_width = static_cast<int>(mat.cols);

  mat_rs = cv::Mat(target_height, target_width, CV_8UC3,
                   cv::Scalar(0, 0, 0));
  // scale ratio (new / old) new_shape(h,w)
  float w_r = (float) target_width / (float) img_width;
  float h_r = (float) target_height / (float) img_height;
  float r = std::min(w_r, h_r);
  // compute padding
  int new_unpad_w = static_cast<int>((float) img_width * r); // floor
  int new_unpad_h = static_cast<int>((float) img_height * r); // floor
  int pad_w = target_width - new_unpad_w; // >=0
  int pad_h = target_height - new_unpad_h; // >=0

  int dw = pad_w / 2;
  int dh = pad_h / 2;

  // resize with unscaling
  cv::Mat new_unpad_mat = mat.clone(); // TODO: may not need clone.
  cv::resize(new_unpad_mat, new_unpad_mat, cv::Size(new_unpad_w, new_unpad_h));
  new_unpad_mat.copyTo(mat_rs(cv::Rect(dw, dh, new_unpad_w, new_unpad_h)));

  // record scale params.
  scale_params.r = r;
  scale_params.dw = dw;
  scale_params.dh = dh;
  scale_params.new_unpad_w = new_unpad_w;
  scale_params.new_unpad_h = new_unpad_h;
  scale_params.flag = true;
}

void MNNPortraitSegExtremeC3Net::detect(const cv::Mat &mat, types::PortraitSegContent &content,
                                        float score_threshold, bool remove_noise)
{
  if (mat.empty()) return;

  // resize & unscale
  cv::Mat mat_rs;
  PortraitSegExtremeC3NetScaleParams scale_params;
  this->resize_unscale(mat, mat_rs, input_height, input_width, scale_params);

  // 1. make input tensor
  this->transform(mat_rs);
  // 2. inference
  mnn_interpreter->runSession(mnn_session);
  auto output_tensors = mnn_interpreter->getSessionOutputAll(mnn_session);
  // 3. generate mask
  this->generate_mask(scale_params, output_tensors, mat, content, score_threshold, remove_noise);
}

static inline void __decode_and_zero_if_small_inplace(float *mutable_ptr, float &score)
{
  // ref: https://github.com/clovaai/ext_portrait_segmentation/blob/master/etc/lovasz_losses.py#L143
  const float sign = (1.f / (*mutable_ptr)) <= score ? -1.f : 1.f;
  *mutable_ptr = (sign + 1.f) / 2.f; // 0. or 1.
}

void MNNPortraitSegExtremeC3Net::generate_mask(const PortraitSegExtremeC3NetScaleParams &scale_params,
                                               const std::map<std::string, MNN::Tensor *> &output_tensors,
                                               const cv::Mat &mat, types::PortraitSegContent &content,
                                               float score_threshold, bool remove_noise)
{
  auto device_output_ptr = output_tensors.at("output"); // e.g (1,2,224,224)
  MNN::Tensor host_output_tensor(device_output_ptr, device_output_ptr->getDimensionType());
  device_output_ptr->copyToHostTensor(&host_output_tensor);
  const unsigned int h = mat.rows;
  const unsigned int w = mat.cols;
  auto output_dims = host_output_tensor.shape();
  const unsigned int out_h = output_dims.at(2); // e.g 224
  const unsigned int out_w = output_dims.at(3); // e.g 224
  const unsigned int element_size = out_h * out_w;

  float *output_ptr = host_output_tensor.host<float>();

  // remove small values
  for (unsigned int i = 0; i < element_size; ++i)
    __decode_and_zero_if_small_inplace(output_ptr + i, score_threshold);

  // fetch foreground score
  const int dw = scale_params.dw;
  const int dh = scale_params.dh;
  const int nw = scale_params.new_unpad_w;
  const int nh = scale_params.new_unpad_h;

  cv::Mat alpha_pred(out_h, out_w, CV_32FC1, output_ptr);
  cv::Mat mask = alpha_pred(cv::Rect(dw, dh, nw, nh)); // 0. ~ 1.
  if (remove_noise) lite::utils::remove_small_connected_area(mask, 0.05f);
  if (nh != h || nw != w) cv::resize(mask, mask, cv::Size(w, h));

  content.mask = mask;
  content.flag = true;
}