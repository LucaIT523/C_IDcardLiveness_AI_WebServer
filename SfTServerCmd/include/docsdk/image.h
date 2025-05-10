/* Copyright 2021 ID R&D Inc. All Rights Reserved. */

/**
 * @file image.h
 * IDLive Doc SDK Image class
 */

#pragma once

#include <cstdint>
#include <string>
#include <memory>

#include "config.h"

namespace docsdk {

/**
  * @brief Image representation
  */
struct DOCSDK_API Image {
    using Ptr = std::shared_ptr<Image>;

    /**
     * @brief Image color encoding
     */
    enum class ColorEncoding {
        kRGB888, /**< RGB encoding, each channel value is represented as 8 bits */
        kBGR888  /**< BGR encoding, each channel value is represented as 8 bits */
    };

    /** Image factory method, creates Image from a buffer containing image file contents.
     *
     * @param bytes Pointer to a buffer containing image file contents
     * @param num_bytes Buffer size
     * @return Smart pointer to created Image instance
     * @throw std::runtime_error
     */
    static Ptr Create(const uint8_t* bytes, size_t num_bytes);

    /** Image factory method, creates Image from an image file using given file path.
     *
     * @param path Path to image file
     * @return Smart pointer to created Image instance
     * @throw std::runtime_error
     */
    static Ptr Create(const std::string& path);

    /** Image factory method, creates Image from raw color data.
     *
     * @param raw_data Raw color data (three-dimensional array containing color channel values for each image pixel)
     * @param num_rows Number of image rows
     * @param num_cols Number of image columns
     * @param encoding Image color encoding
     * @return Smart pointer to created Image instance
     * @throw std::runtime_error
     */
    static Ptr Create(const uint8_t* raw_data, uint32_t num_rows, uint32_t num_cols, ColorEncoding encoding);

    virtual ~Image() noexcept = default;
};

}  // namespace docsdk
