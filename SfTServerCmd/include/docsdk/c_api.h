/* Copyright 2021 ID R&D Inc. All Rights Reserved. */

/**
 * @file c_api.h
 * IDLive Doc SDK C API
 */

#pragma once

#include <stdint.h>
#include <stddef.h>

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
  * @brief Image representation
  */
typedef struct DocSdkImage DocSdkImage;

/**
  * @brief Document liveness checking pipeline
  */
typedef struct DocSdkLivenessPipeline DocSdkLivenessPipeline;

/**
  * @brief DocSDK error codes enumeration
  */
typedef enum DocSdkErrorCode {
    kDocSdkErrorCodeOk,              /**< No error */
    kDocSdkErrorCodeNullptr,         /**< Null pointer was passed as a parameter */
    kDocSdkErrorCodeNativeException, /**< Native exception was thrown by underlying C++ code */
    kDocSdkErrorCodeLicenseException /**< License exception was thrown by underlying C++ code */
} DocSdkErrorCode;

/**
  * @brief Image color encoding
  */
typedef enum DocSdkColorEncoding {
    kDocSdkColorEncodingRGB888, /**< RGB encoding, each channel value is represented as 8 bits */
    kDocSdkColorEncodingBGR888  /**< BGR encoding, each channel value is represented as 8 bits */
} DocSdkColorEncoding;

/**
  * @brief Image validation status code (image validation is performed beforehand each liveness check)
  */
typedef enum DocSdkValidationStatusCode {
    kDocSdkValidationStatusCodeOk,                       /**< Validation successful */
    kDocSdkValidationStatusCodeDocumentTooCloseToCamera, /**< Document is too close to the camera */
    kDocSdkValidationStatusCodeDocumentTooCloseToBorder, /**< Document is too close to the image border */
    kDocSdkValidationStatusCodeDocumentCropped, /**< Document is cropped (part of the document is not present in the image),
                                                     corresponds to @ref kDocSdkImageQualityWarningDocumentBordersOutsideOfFrame */
    kDocSdkValidationStatusCodeDocumentNotFound,     /**< Document is not detected in the image */
    kDocSdkValidationStatusCodeDocumentTooSmall,     /**< Document size in the image is too small */
    kDocSdkValidationStatusCodeTooManyDocuments,     /**< Too many documents were detected in the image,
                                                          corresponds to @ref kDocSdkImageQualityWarningMultipleDocumentsInFrame */
    kDocSdkValidationStatusCodeDocumentIsColorless,  /**< Document is colorless (black and white) */
    kDocSdkValidationStatusCodeDocumentPhotoNotFound /**< Document photo is not detected in the image */
} DocSdkValidationStatusCode;

/**
  * @brief Image quality warning (image quality check is performed along with liveness check)
  */
typedef enum DocSdkImageQualityWarning {
    kDocSdkImageQualityWarningRelativeDocumentSizeLowerThan10Percent, /**< Document size (area) is lower than 10% of the frame size */
    kDocSdkImageQualityWarningDocumentBordersOutsideOfFrame, /**< Document border(s) are outside of the frame,
                                                                  corresponds to @ref kDocSdkValidationStatusCodeDocumentCropped */
    kDocSdkImageQualityWarningMultipleDocumentsInFrame,      /**< More than one document are present in the frame,
                                                                  corresponds to @ref kDocSdkValidationStatusCodeTooManyDocuments */
    kDocSdkImageQualityWarningDocumentTooCloseToBorder,      /**< Document is too close to the frame border */
    kDocSdkImageQualityWarningImageTooBlurry,                /**< Image is too blurry */
    kDocSdkImageQualityWarningImageTooCompressed,            /**< Image is too compressed */
    kDocSdkImageQualityWarningPoorImageExposure,             /**< Image is either too bright or too dark */
    kDocSdkImageQualityWarningGlareOnImage                   /**< Image has glare */
} DocSdkImageQualityWarning;

/**
  * @brief Liveness pipeline result
  */
typedef struct DocSdkLivenessPipelineResult {
    /** Document liveness probability from 0 to 1, only filled if status_code == kDocSdkValidationStatusCodeOk */
    float liveness_probability;

    /** Raw liveness checking score, can be used for calibration, only filled if status_code == kDocSdkValidationStatusCodeOk */
    float liveness_score;

    /**
     * @brief Image quality checking score, only filled if status_code == kDocSdkValidationStatusCodeOk
     * @deprecated This value is always zero, use @ref image_quality_warnings instead
     */
    float quality_score;

    /** A bitmask holding raised image quality warnings (bit flags are defined by @ref DocSdkImageQualityWarning) */
    uint64_t image_quality_warnings;

    /** Image validation status code */
    DocSdkValidationStatusCode status_code;
} DocSdkLivenessPipelineResult;

/**
 * @brief Score/threshold calibration to be used for document liveness check
 */
typedef enum DocSdkLivenessCheckCalibration {
    kDocSdkLivenessCheckCalibrationRegular, /**< Regular calibration, targets low APCER */
    kDocSdkLivenessCheckCalibrationSoft, /**< Soft calibration, achieves lower BPCER while still having acceptable APCER */
    kDocSdkLivenessCheckCalibrationHard /**< Hard calibration, achieves lower APCER while still having acceptable BPCER */
} DocSdkLivenessCheckCalibration;

/**
  * @brief Configurable options used for document liveness check
  */
typedef struct DocSdkLivenessCheckOptions {
    /**
     * @brief Score/threshold calibration to be used for document liveness check
     */
    DocSdkLivenessCheckCalibration calibration;

    /**
     * @brief Defines which validation errors should be ignored during the liveness check, null allowed
     */
    DocSdkValidationStatusCode* errors_to_ignore;

    /**
     * @brief Specifies the number of elements in errors_to_ignore array
     */
    size_t num_errors_to_ignore;
} DocSdkLivenessCheckOptions;

/**
 * @brief Document spoofing attack type
 */
typedef enum DocSdkAttackType {
    kDocSdkAttackTypeScreenReplay,         /**< Screen replay attack */
    kDocSdkAttackTypePrintedCopy,          /**< Printed copy attack */
    kDocSdkAttackTypePortraitSubstitution, /**< Portrait substitution attack */
    kDocSdkAttackTypeDigitalManipulation,  /**< Digital manipulation attack */
    kDocSdkAttackTypeOther,                /**< Other attack */
} DocSdkAttackType;

/**
 * @brief DocSDK feature information
 */
typedef struct DocSdkFeature {
    DocSdkAttackType attack_type; /**< Attack type addressed by the feature */
    char* pipeline_name;          /**< Name of the liveness pipeline implementing the feature */
    char* expiration_date;        /**< Feature expiration date in YYYY-MM-DD format */
} DocSdkFeature;

/**
 * @brief Information about DocSDK features available with the installed license
 */
typedef struct DocSdkLicenseInfo {
    DocSdkFeature* features; /**< Array holding available features */
    size_t num_features;     /**< Number of available features */
} DocSdkLicenseInfo;

/** Sets error logging callback. By default, all error messages are printed to stderr.
  * Callback accepts two parameters: an error code and a null-terminated string containing
  * error description.
  *
  * @param callback [in] Pointer to a function for handling error messages
  */
DOCSDK_API void DocSdkSetErrorLogCallback(void (*callback)(DocSdkErrorCode, const char*));

/**
  * @brief Sets the maximum number of CPU threads available for DocSDK.
  * If 0 is passed, then the optimal number of CPU threads is detected automatically
  * (the same effect is achieved if the function is not called).
  *
  * @param num_threads Maximum number of CPU threads available for DocSDK
  * @note Function call is equivalent to setting DOCSDK_NUM_THREADS_PIPELINE, DOCSDK_NUM_THREADS_ENGINE
  * and DOCSDK_NUM_THREADS_OPERATOR environment variables to the same value before loading the DocSDK library.
  * @note Function call takes precedence over environment variables.
 */
DOCSDK_API void DocSdkSetNumThreads(unsigned int num_threads);

/**
 * @brief Returns information (e.g. supported features and expiration dates) about the installed license if available.
 *
 * @param error_code [out] Error code
 * @return Pointer to the DocSdkLicense info instance describing DocSDK features available with the installed license.
 * The DocSdkLicenseInfo should be destroy with @DocSdkDestroyLicenseInfo.
 */
DOCSDK_API DocSdkLicenseInfo* DocSdkGetLicenseInfo(DocSdkErrorCode* error_code);

/** Destroys DocSdkLicenseInfo instance.
 *
 * @param license_info [in] Pointer to DocSdkLicenseInfo instance
 */
DOCSDK_API void DocSdkDestroyLicenseInfo(DocSdkLicenseInfo* license_info);

/**
 * @brief Sets the runtime license. Only takes effect in "Lambda" SDK distribution and should be called before any other
 * SDK invocation.
 *
 * @param license_str [in] String holding the license content
 * @param error_code [out] Error code
 */
DOCSDK_API void DocSdkSetRuntimeLicense(const char* license_str, DocSdkErrorCode* error_code);

/** Image factory method, creates Image from a buffer containing image file contents.
  *
  * @param bytes [in] Pointer to a buffer containing image file contents
  * @param size [in] Buffer size
  * @param error_code [out] Error code
  * @return Pointer to created DocSdkImage instance
  */
DOCSDK_API DocSdkImage* DocSdkCreateImageFromBuffer(const uint8_t* bytes, size_t size, DocSdkErrorCode* error_code);

/** Image factory method, creates Image from an image file using given file path.
  *
  * @param file_path [in] Null-terminated string containing path to the image file
  * @param error_code [out] Error code
  * @return Pointer to created DocSdkImage instance
  */
DOCSDK_API DocSdkImage* DocSdkCreateImageFromFile(const char* file_path, DocSdkErrorCode* error_code);

/** Image factory method, creates Image from raw color data.
  *
  * @param data [in] Raw color data (three-dimensional array containing color channel values for each image pixel)
  * @param num_rows [in] Number of image rows
  * @param num_cols [in] Number of image columns
  * @param encoding Image color encoding
  * @param error_code [out] Error code
  * @return Pointer to created DocSdkImage instance
  */
DOCSDK_API DocSdkImage* DocSdkCreateImageFromRawData(const uint8_t* data, size_t num_rows, size_t num_cols,
                                                     DocSdkColorEncoding encoding, DocSdkErrorCode* error_code);

/** Destroys DocSdkImage instance.
 *
 * @param image [in] Pointer to DocSdkImage instance
 */
DOCSDK_API void DocSdkDestroyImage(DocSdkImage* image);

/** Liveness pipeline factory method.
  *
  * @param config_path [in] Null-terminated string containing path to liveness pipeline configuration file
  * @param error_code [out] Error code
  * @return Pointer to created DocSdkLivenessPipeline instance
  */
DOCSDK_API DocSdkLivenessPipeline* DocSdkCreateLivenessPipeline(const char* config_path, DocSdkErrorCode* error_code);

/** Destroys DocSdkLivenessPipeline instance.
 *
 * @param liveness_pipeline [in] Pointer to DocSdkLivenessPipeline instance
 */
DOCSDK_API void DocSdkDestroyLivenessPipeline(DocSdkLivenessPipeline* liveness_pipeline);

/** Default-initializes DocSdkLivenessCheckOptions instance.
 *
 * @return DocSdkLivenessCheckOptions instance
 */
DOCSDK_API DocSdkLivenessCheckOptions DocSdkLivenessCheckOptionsDefault();

/** Checks liveness using the provided document photo.
  *
  * @param liveness_pipeline [in] Pointer to DocSdkLivenessPipeline instance
  * @param image [in] Image containing the photo of document
  * @param options [in] Liveness check options, can be NULL
  * @param error_code [out] Error code
  * @return Liveness checking result
  */
DOCSDK_API DocSdkLivenessPipelineResult DocSdkLivenessPipelineCheckLiveness(DocSdkLivenessPipeline* liveness_pipeline,
                                                                            DocSdkImage* image,
                                                                            DocSdkLivenessCheckOptions* options,
                                                                            DocSdkErrorCode* error_code);

/** Performs liveness check using a sequence of photos (e.g. taken from video)
  * producing a single aggregated liveness checking result.
  *
  * @param liveness_pipeline [in] Pointer to DocSdkLivenessPipeline instance
  * @param images [in] Images array (photo sequence)
  * @param num_images [in] Images array size
  * @param options [in] Liveness check options, can be NULL
  * @param error_code [out] Error code
  * @return Liveness checking result
  */
DOCSDK_API DocSdkLivenessPipelineResult DocSdkLivenessPipelineCheckLivenessSequence(
        DocSdkLivenessPipeline* liveness_pipeline, DocSdkImage** images, size_t num_images,
        DocSdkLivenessCheckOptions* options, DocSdkErrorCode* error_code);

#ifdef __cplusplus
}
#endif
