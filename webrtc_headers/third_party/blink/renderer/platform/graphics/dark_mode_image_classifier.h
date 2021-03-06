// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_PLATFORM_GRAPHICS_DARK_MODE_IMAGE_CLASSIFIER_H_
#define THIRD_PARTY_BLINK_RENDERER_PLATFORM_GRAPHICS_DARK_MODE_IMAGE_CLASSIFIER_H_

#include "base/gtest_prod_util.h"
#include "base/optional.h"
#include "third_party/blink/renderer/platform/geometry/float_rect.h"
#include "third_party/blink/renderer/platform/graphics/graphics_types.h"
#include "third_party/blink/renderer/platform/wtf/allocator/allocator.h"
#include "third_party/skia/include/core/SkBitmap.h"

namespace blink {

class Image;

FORWARD_DECLARE_TEST(DarkModeImageClassifierTest, FeaturesAndClassification);
FORWARD_DECLARE_TEST(DarkModeImageClassifierTest, Caching);
FORWARD_DECLARE_TEST(DarkModeImageClassifierTest, BlocksCount);
FORWARD_DECLARE_TEST(SVGImageTest, DarkModeClassification);

class PLATFORM_EXPORT DarkModeImageClassifier {
  DISALLOW_NEW();

 public:
  struct Features {
    // True if the image is in color, false if it is grayscale.
    bool is_colorful;

    // Ratio of the number of bucketed colors used in the image to all
    // possibilities. Color buckets are represented with 4 bits per color
    // channel.
    float color_buckets_ratio;

    // How much of the image is transparent or considered part of the
    // background.
    float transparency_ratio;
    float background_ratio;
  };

  DarkModeImageClassifier();
  ~DarkModeImageClassifier() = default;

  DarkModeClassification Classify(Image* image,
                                  const FloatRect& src_rect,
                                  const FloatRect& dest_rect);

  enum class ImageType { kBitmap = 0, kSvg = 1 };
  void SetImageType(ImageType image_type) { image_type_ = image_type; }

 private:
  DarkModeClassification ClassifyWithFeatures(const Features& features);
  DarkModeClassification ClassifyUsingDecisionTree(const Features& features);
  base::Optional<Features> GetFeatures(Image* image, const FloatRect& src_rect);
  void Reset();

  enum class ColorMode { kColor = 0, kGrayscale = 1 };

  // Given a SkBitmap, extracts a sample set of pixels (|sampled_pixels|),
  // |transparency_ratio|, and |background_ratio|.
  void GetSamples(const SkBitmap& bitmap,
                  Vector<SkColor>* sampled_pixels,
                  float* transparency_ratio,
                  float* background_ratio);

  // Gets the |required_samples_count| for a specific |block| of the given
  // SkBitmap, and returns |sampled_pixels| and |transparent_pixels_count|.
  void GetBlockSamples(const SkBitmap& bitmap,
                       const IntRect& block,
                       const int required_samples_count,
                       Vector<SkColor>* sampled_pixels,
                       int* transparent_pixels_count);

  // Given |sampled_pixels|, |transparency_ratio|, and |background_ratio| for an
  // image, computes and returns the features required for classification.
  Features ComputeFeatures(const Vector<SkColor>& sampled_pixels,
                           const float transparency_ratio,
                           const float background_ratio);

  // Receives sampled pixels and color mode, and returns the ratio of color
  // buckets count to all possible color buckets. If image is in color, a color
  // bucket is a 4 bit per channel representation of each RGB color, and if it
  // is grayscale, each bucket is a 4 bit representation of luminance.
  float ComputeColorBucketsRatio(const Vector<SkColor>& sampled_pixels,
                                 const ColorMode color_mode);

  int pixels_to_sample_;
  // Holds the number of blocks in the horizontal direction when the image is
  // divided into a grid of blocks.
  int blocks_count_horizontal_;
  // Holds the number of blocks in the vertical direction when the image is
  // divided into a grid of blocks.
  int blocks_count_vertical_;

  ImageType image_type_;

  FRIEND_TEST_ALL_PREFIXES(DarkModeImageClassifierTest,
                           FeaturesAndClassification);
  FRIEND_TEST_ALL_PREFIXES(DarkModeImageClassifierTest, Caching);
  FRIEND_TEST_ALL_PREFIXES(DarkModeImageClassifierTest, BlocksCount);
  FRIEND_TEST_ALL_PREFIXES(SVGImageTest, DarkModeClassification);
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_PLATFORM_GRAPHICS_DARK_MODE_IMAGE_CLASSIFIER_H_
