/* Copyright 2022 ID R&D Inc. All Rights Reserved. */

/**
 * @file settings.h
 * IDLive Doc SDK Settings class
 */

#pragma once

#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "config.h"

namespace docsdk {

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
DOCSDK_API void SetNumThreads(unsigned int num_threads) noexcept;

/**
 * @brief Document spoofing attack type
 */
enum class AttackType {
    kScreenReplay,         /**< Screen replay attack */
    kPrintedCopy,          /**< Printed copy attack */
    kPortraitSubstitution, /**< Portrait substitution attack */
    kDigitalManipulation,  /**< Digital manipulation attack */
    kOther                 /**< Other attack */
};

/**
 * @brief DocSDK feature information
 */
struct DocSdkFeature {
    AttackType attack_type;      /**< Attack type addressed by the feature */
    std::string pipeline_name;   /**< Name of the liveness pipeline implementing the feature */
    std::string expiration_date; /**< Feature expiration date in YYYY-MM-DD format */

    DocSdkFeature(AttackType attack_type, std::string pipeline_name, std::string expiration_date)
        : attack_type(attack_type),
          pipeline_name(std::move(pipeline_name)),
          expiration_date(std::move(expiration_date)) {}

    friend std::ostream& operator<<(std::ostream& os, const DocSdkFeature& feature) {
        os << "DocSdkFeature[ attack_type: ";
#define PRINT_ATTACK_TYPE(t) case AttackType::t: os << #t; break;
        switch (feature.attack_type) {
            PRINT_ATTACK_TYPE(kScreenReplay);
            PRINT_ATTACK_TYPE(kPrintedCopy);
            PRINT_ATTACK_TYPE(kPortraitSubstitution);
            PRINT_ATTACK_TYPE(kDigitalManipulation);
            PRINT_ATTACK_TYPE(kOther);
        }
#undef PRINT_ATTACK_TYPE
        os << ", pipeline_name: " << feature.pipeline_name
           << ", expiration_date: " << feature.expiration_date << " ]";
        return os;
    }
};

/**
 * @brief Returns information (e.g. supported features and expiration dates) about the installed license if available.
 *
 * @return List of document liveness features available with the installed license.
 */
DOCSDK_API std::vector<DocSdkFeature> GetLicenseInfo();

/**
 * @brief Sets the runtime license. Only takes effect in "Lambda" SDK distribution and should be called before any other
 * SDK invocation.
 *
 * @param license_str String holding the license content.
 * @throw LicenseException on invalid license string.
 */
DOCSDK_API void SetRuntimeLicense(const std::string& license_str);

/**
 * @brief A type for exception which is thrown on license expiration or absence.
 */
class LicenseException : public std::runtime_error {
 public:
    explicit LicenseException(const std::string& message) : std::runtime_error(message) {}
    explicit LicenseException(const char* message) : std::runtime_error(message) {}
};

}  // namespace docsdk
