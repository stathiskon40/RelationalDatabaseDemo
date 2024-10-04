#include "../../include/database/Datatype.h"
#include <regex>

// Varchar constructor
VarcharType::VarcharType(size_t maxLength) : maxLength(maxLength) {}

// Varchar validation
void VarcharType::validate(const std::string& value) const {
    if (value.size() > maxLength) {
        throw std::invalid_argument("Value exceeds maximum length for Varchar");
    }
}

// Integer validation
void IntType::validate(const std::string& value) const {
    try {
        size_t idx;
        std::stoi(value, &idx);
        if (idx != value.size()) {
            throw std::invalid_argument("Invalid value for Int");
        }
    } catch (...) {
        throw std::invalid_argument("Invalid value for Int");
    }
}

// LongInt validation
void LongIntType::validate(const std::string& value) const {
    try {
        size_t idx;
        std::stol(value, &idx);
        if (idx != value.size()) {
            throw std::invalid_argument("Invalid value for LongInt");
        }
    } catch (...) {
        throw std::invalid_argument("Invalid value for LongInt");
    }
}

// Double validation
void DoubleType::validate(const std::string& value) const {
    try {
        size_t idx;
        std::stod(value, &idx);
        if (idx != value.size()) {
            throw std::invalid_argument("Invalid value for Double");
        }
    } catch (...) {
        throw std::invalid_argument("Invalid value for Double");
    }
}

// DateTime validation
void DateTimeType::validate(const std::string& value) const {
    // Regex pattern for ISO 8601 date-time format: YYYY-MM-DD HH:MM:SS
    std::regex pattern(R"(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})");
    if (!std::regex_match(value, pattern)) {
        throw std::invalid_argument("Invalid value for DateTime");
    }
    // Further validation can be added to check actual date and time values
}
