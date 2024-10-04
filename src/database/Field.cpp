#include "../../include/database/Field.h"

// Field constructor
Field::Field(const std::string& name, DataType* dataType, const std::vector<Constraint*>& constraints)
    : name(name), dataType(dataType), constraints(constraints) {}

// Field destructor
Field::~Field() {
    // Delete the data type object
    delete dataType;

    // Delete each constraint object
    for (Constraint* constraint : constraints) {
        delete constraint;
    }
}

// Get the name of the field
std::string Field::getName() const {
    return name;
}

// Validate a value according to the field's data type and constraints
void Field::validate(const std::string& value) const {
    dataType->validate(value);
    for (const auto& constraint : constraints) {
        constraint->check(value);
    }
}

// Get the data type
DataType* Field::getDataType() const {
    return dataType;
}

// Get the constraints
const std::vector<Constraint*>& Field::getConstraints() const {
    return constraints;
}
