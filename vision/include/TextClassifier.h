#ifndef TEXT_CLASSIFICATION_H
#define TEXT_CLASSIFICATION_H

#include "IModel.h"

class TextClassifier : public IModel {
public:
  TextClassifier(zmqpp::context&, const std::string&);

  OCRResult runModel(const std::filesystem::path&) override;
  OCRResult handleClassification(const std::filesystem::path&) override;
};

#endif
