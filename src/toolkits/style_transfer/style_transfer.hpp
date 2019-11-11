/* Copyright © 2019 Apple Inc. All rights reserved.
 *
 * Use of this source code is governed by a BSD-3-clause license that can
 * be found in the LICENSE.txt file or at
 * https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef __TOOLKITS_STYLE_TRANSFER_H_
#define __TOOLKITS_STYLE_TRANSFER_H_

#include <memory>

#include <core/data/sframe/gl_sarray.hpp>
#include <core/data/sframe/gl_sframe.hpp>
#include <core/logging/table_printer/table_printer.hpp>
#include <ml/neural_net/compute_context.hpp>
#include <ml/neural_net/model_backend.hpp>
#include <ml/neural_net/model_spec.hpp>
#include <model_server/lib/extensions/ml_model.hpp>
#include <toolkits/coreml_export/mlmodel_wrapper.hpp>
#include <toolkits/coreml_export/neural_net_models_exporter.hpp>
#include <toolkits/style_transfer/style_transfer_data_iterator.hpp>

namespace turi {
namespace style_transfer {

class EXPORT style_transfer : public ml_model_base {
 public:
  void init_options(const std::map<std::string, flexible_type>& opts) override;
  size_t get_version() const override;
  void save_impl(oarchive& oarc) const override;
  void load_version(iarchive& iarc, size_t version) override;

  std::shared_ptr<coreml::MLModelWrapper> export_to_coreml(
      std::string filename, std::map<std::string, flexible_type> opts);

  void train(gl_sarray style, gl_sarray content,
             std::map<std::string, flexible_type> opts);

  virtual void init_train(gl_sarray style, gl_sarray content,
                          std::map<std::string, flexible_type> opts);

  virtual void iterate_training();
  virtual void finalize_training();

  gl_sframe predict(variant_type data,
                    std::map<std::string, flexible_type> opts);

  void import_from_custom_model(variant_map_type model_data, size_t version);

  BEGIN_CLASS_MEMBER_REGISTRATION("style_transfer")
  IMPORT_BASE_CLASS_REGISTRATION(ml_model_base);

  REGISTER_CLASS_MEMBER_FUNCTION(style_transfer::train, "style", "content",
                                 "opts");

  REGISTER_CLASS_MEMBER_DOCSTRING(
      style_transfer::train,
      "\n"
      "Options\n"
      "-------\n"
      "resnet_mlmodel_path : string\n"
      "    Path to the Resnet CoreML specification with the pre-trained model\n"
      "    parameters.\n"
      "vgg_mlmodel_path: string\n"
      "    Path to the VGG16 CoreML specification with the pre-trained model\n"
      "    parameters.\n"
      "num_styles: int\n"
      "    The defined number of styles for the style transfer model\n"
      "batch_size : int\n"
      "    The number of images per training iteration. If 0, then it will be\n"
      "    automatically determined based on resource availability.\n"
      "max_iterations : int\n"
      "    The number of training iterations. If 0, then it will be "
      "automatically\n"
      "    be determined based on the amount of data you provide.\n"
      "image_width : int\n"
      "    The input image width to the model\n"
      "image_height : int\n"
      "    The input image height to the model\n");

  REGISTER_CLASS_MEMBER_FUNCTION(style_transfer::init_train, "style", "content",
                                 "opts");

  REGISTER_CLASS_MEMBER_FUNCTION(style_transfer::iterate_training);
  REGISTER_CLASS_MEMBER_FUNCTION(style_transfer::finalize_training);

  REGISTER_CLASS_MEMBER_FUNCTION(style_transfer::export_to_coreml, "filename",
                                 "options");

  register_defaults("export_to_coreml",
                    {{"options",
                      to_variant(std::map<std::string, flexible_type>())}});

  REGISTER_CLASS_MEMBER_FUNCTION(style_transfer::predict, "data", "options");

  REGISTER_CLASS_MEMBER_FUNCTION(style_transfer::import_from_custom_model,
                                 "model_data", "version");

  END_CLASS_MEMBER_REGISTRATION

 protected:
  virtual std::unique_ptr<data_iterator> create_iterator(
      data_iterator::parameters iterator_params) const;

  std::unique_ptr<data_iterator> create_iterator(gl_sarray content,
                                                 gl_sarray style, bool repeat,
                                                 bool training,
                                                 int random_seed) const;

  virtual std::unique_ptr<neural_net::compute_context> create_compute_context()
      const;

  void perform_predict(gl_sarray images, gl_sframe_writer& result,
                       const std::vector<double>& style_idx);

  template <typename T>
  T read_state(const std::string& key) const {
    return variant_get_value<T>(get_state().at(key));
  }

 private:
  std::unique_ptr<neural_net::model_spec> m_resnet_spec;
  std::unique_ptr<neural_net::model_spec> m_vgg_spec;

  std::unique_ptr<data_iterator> m_training_data_iterator;
  std::unique_ptr<neural_net::compute_context> m_training_compute_context;
  std::unique_ptr<neural_net::model_backend> m_training_model;

  std::unique_ptr<table_printer> training_table_printer_;

  static gl_sarray convert_types_to_sarray(const variant_type& data);

  flex_int get_max_iterations() const;
  flex_int get_training_iterations() const;
  flex_int get_num_classes() const;

  void infer_derived_options();
};

}  // namespace style_transfer
}  // namespace turi

#endif  // __TOOLKITS_STYLE_TRANSFER_H_