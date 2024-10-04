#ifndef DATATYPE_H
#define DATATYPE_H

#include <string>
#include <stdexcept>

// Base class for data types
class DataType {
public:
    virtual void validate(const std::string& value) const = 0;
    virtual ~DataType() = default;
    virtual std::string getName() const = 0;
};

// Varchar data type
class VarcharType : public DataType {
    size_t maxLength;
public:
    VarcharType(size_t maxLength);
    void validate(const std::string& value) const override;
    std::string getName() const override { return "VARCHAR"; }
};

// Integer data type
class IntType : public DataType {
public:
    void validate(const std::string& value) const override;
    std::string getName() const override { return "INT"; }
};

// Long Integer data type
class LongIntType : public DataType {
public:
    void validate(const std::string& value) const override;
    std::string getName() const override { return "LONGINT"; }
};

// Double data type
class DoubleType : public DataType {
public:
    void validate(const std::string& value) const override;
    std::string getName() const override { return "DOUBLE"; }
};

// DateTime data type (basic validation example)
class DateTimeType : public DataType {
public:
    void validate(const std::string& value) const override;
    std::string getName() const override { return "DATETIME"; }
};

#endif // DATATYPE_H
