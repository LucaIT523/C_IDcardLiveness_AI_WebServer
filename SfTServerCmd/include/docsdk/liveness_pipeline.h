/* Copyright 2021 ID R&D Inc. All Rights Reserved. */

/**
 * @file liveness_pipeline.h
 * IDLive Doc SDK LivenessPipeline class
 */

#pragma once

#include <string>
#include <memory>
#include <ostream>
#include <utility>
#include <vector>
#include <unordered_map>

#include "config.h"
#include "image.h"

namespace docsdk {

/**
  * @brief Liveness pipeline result
  */
struct LivenessPipelineResult {
    /**
     * @brief Image validation status code (image validation is performed beforehand each liveness check)
     */
    enum class ValidationStatusCode {
        kOk,                       /**< Validation successful */
        kDocumentTooCloseToCamera, /**< Document is too close to the camera */
        kDocumentTooCloseToBorder, /**< Document is too close to the image border */
        kDocumentCropped,          /**< Document is cropped (part of the document is not present in the image),
                                        corresponds to @ref ImageQualityWarning::kDocumentBordersOutsideOfFrame */
        kDocumentNotFound,         /**< Document is not detected in the image */
        kDocumentTooSmall,         /**< Document size in the image is too small */
        kTooManyDocuments,         /**< Too many documents were detected in the image,
                                        corresponds to @ref ImageQualityWarning::kMultipleDocumentsInFrame */
        kDocumentIsColorless,      /**< Document is colorless (black and white) */
        kDocumentPhotoNotFound     /**< Document photo is not detected in the image */
    };

    /**
     * @brief Image quality warning (image quality check is performed along with liveness check)
     */
    enum class ImageQualityWarning {
        kRelativeDocumentSizeLowerThan10Percent, /**< Document size (area) is lower than 10% of the frame size */
        kDocumentBordersOutsideOfFrame,          /**< Document border(s) are outside of the frame,
                                                      corresponds to @ref ValidationStatusCode::kDocumentCropped */
        kMultipleDocumentsInFrame,               /**< More than one document are present in the frame,
                                                      corresponds to @ref ValidationStatusCode::kTooManyDocuments */
        kDocumentTooCloseToBorder,               /**< Document is too close to the frame border */
        kImageTooBlurry,                         /**< Image is too blurry */
        kImageTooCompressed,                     /**< Image is too compressed */
        kPoorImageExposure,                      /**< Image is either too bright or too dark */
        kGlareOnImage,                           /**< Image has glare */
    };

    /**
     * @brief Document liveness check result
     */
    struct Value {
        float liveness_probability; /**< Document liveness probability from 0 to 1 */
        float liveness_score;       /**< Raw liveness checking score, can be used for calibration */

        /**
         * @brief Image quality checking score
         * @deprecated This value is always zero. Use @ref image_quality_warnings instead
         */
        float quality_score = 0;

        std::vector<ImageQualityWarning> image_quality_warnings; /**< Image quality warnings */

        Value() = default;
        Value(float liveness_probability, float liveness_score, float quality_score,
              std::vector<ImageQualityWarning> image_quality_warnings = {})
            : liveness_probability(liveness_probability),
              liveness_score(liveness_score),
              quality_score(quality_score),
              image_quality_warnings(std::move(image_quality_warnings)) {}

        friend std::ostream& operator<<(std::ostream& os, const Value& result) {
            os << "LivenessPipelineResult::Value["
               << " liveness_probability: " << result.liveness_probability
               << ", liveness_score: " << result.liveness_score << ", quality_score: " << result.quality_score
               << ", image_quality_warnings: [ ";
            for (size_t idx = 0; idx < result.image_quality_warnings.size(); ++idx) {
                if (idx != 0) {
                    os << ", ";
                }
                os << static_cast<int>(result.image_quality_warnings[idx]);
            }
            os << " ]";
            return os;
        }
    };

    LivenessPipelineResult() = default;
    explicit LivenessPipelineResult(ValidationStatusCode status_code) : status_code(status_code) {}
    explicit LivenessPipelineResult(Value value) : value(std::move(value)), status_code(ValidationStatusCode::kOk) {}

    /** Returns document liveness checking result if validation was successful.
     * Otherwise, throws an exception.
     *
     * @return Liveness checking result
     * @throw std::runtime_error
     */
    const Value& GetValue() const {
        if (Ok()) {
            return value;
        } else {
            throw std::runtime_error("LivenessPipelineResult: value is not available, because status code is not kOk");
        }
    }

    /** Returns validation status code.
     *
     * @return Validation status code
     */
    const ValidationStatusCode& GetStatusCode() const noexcept {
        return status_code;
    }

    /** Returns a binary value indicated whether the validation was successful.
     *
     * @return True if validation was successful, false otherwise
     */
    bool Ok() const noexcept {
        return status_code == ValidationStatusCode::kOk;
    }

    friend std::ostream& operator<<(std::ostream& os, const LivenessPipelineResult& result) {
        os << "LivenessPipelineResult[ status: " << static_cast<int>(result.status_code) << ", value: ";
        if (result.Ok()) {
            os << result.value;
        } else {
            os << "unavailable";
        }
        os << " ]";
        return os;
    }

 private:
    Value value{};
    ValidationStatusCode status_code = ValidationStatusCode::kOk;
};

/**
  * @brief Configurable options used for document liveness check
  */
struct LivenessCheckOptions {
    /**
     * @brief Score/threshold calibration to be used
     */
    enum class Calibration {
        kRegular, /**< Regular calibration, targets low APCER */
        kSoft,    /**< Soft calibration, achieves lower BPCER while still having acceptable APCER */
        kHard     /**< Hard calibration, achieves lower APCER while still having acceptable BPCER */
    };

    /**
     * @brief Defines which validation errors should be ignored during the liveness check
     */
    std::vector<LivenessPipelineResult::ValidationStatusCode> errors_to_ignore;

    /**
     * @brief Score/threshold calibration to be used for document liveness check
     */
    Calibration calibration = Calibration::kRegular;

    friend std::ostream& operator<<(std::ostream& os, const LivenessCheckOptions& options) {
        os << "LivenessCheckOptions[ calibration: " << static_cast<int>(options.calibration) << " ]";
        return os;
    }
};

/**
  * @brief Document liveness checking pipeline
  */
class DOCSDK_API LivenessPipeline {
 public:
    using Ptr = std::shared_ptr<LivenessPipeline>;

    /** Liveness pipeline factory method.
     *
     * @param path_to_config Path to liveness pipeline configuration file
     * @return Smart pointer to created LivenessPipeline instance
     * @throw std::runtime_error
     * @throw docsdk::LicenseException
     */
    static Ptr Create(const std::string& path_to_config);

    /** Checks liveness using the provided document photo.
     *
     * @param image Image containing the photo of document
     * @param options Optional liveness check options
     * @return Liveness checking result
     * @throw std::runtime_error
     * @throw docsdk::LicenseException
     */
    virtual LivenessPipelineResult CheckLiveness(Image::Ptr image, const LivenessCheckOptions& options = {}) const = 0;

    /** Performs liveness check using a sequence of photos (e.g. taken from video)
     * producing a single aggregated liveness checking result.
     *
     * @param images Photo sequence
     * @param options Optional liveness check options
     * @return Liveness checking result
     * @throw std::runtime_error
     * @throw docsdk::LicenseException
     */
    virtual LivenessPipelineResult CheckLivenessSequence(const std::vector<Image::Ptr>& images,
                                                         const LivenessCheckOptions& options = {}) const = 0;

    virtual ~LivenessPipeline() noexcept = default;
};

/**
  * @brief Document liveness checking pipeline composition
  */
class DOCSDK_API LivenessPipelineComposition {
 public:
    using Ptr = std::shared_ptr<LivenessPipelineComposition>;

    /** Pipeline composition factory method.
     *
     * @param paths_to_configs Paths to liveness pipeline configuration files to use in composition
     * @return Smart pointer to created LivenessPipelineComposition instance
     * @throw std::runtime_error
     * @throw docsdk::LicenseException
     */
    static Ptr Create(const std::vector<std::string>& paths_to_configs);

    /** Checks liveness using the provided document photo.
     *
     * @param image Image containing the photo of document
     * @param options Optional liveness check options. If empty all pipelines in composition are run,
     * otherwise only pipelines in options are run
     * @return Liveness checking result
     * @throw std::runtime_error
     * @throw docsdk::LicenseException
     */
    virtual std::unordered_map<std::string, LivenessPipelineResult> CheckLiveness(
            Image::Ptr image, const std::unordered_map<std::string, LivenessCheckOptions>& options = {}) const = 0;

    virtual ~LivenessPipelineComposition() noexcept = default;
};

}  // namespace docsdk
