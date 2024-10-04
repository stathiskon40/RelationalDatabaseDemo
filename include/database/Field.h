#ifndef FIELD_H
#define FIELD_H

#include <string>
#include <vector>
#include "Datatype.h"
#include "Constraint.h"

class Field {
public:
    // Constructor
    Field(const std::string& name, DataType* dataType, const std::vector<Constraint*>& constraints);

    // Destructor
    ~Field();

    // Get the name of the field
    std::string getName() const;

    // Validate a value according to the field's data type and constraints
    void validate(const std::string& value) const;

    // Get the data type
    DataType* getDataType() const;

    // Get the constraints
    const std::vector<Constraint*>& getConstraints() const;

private:
    std::string name;  // Field name
    DataType* dataType;  // Data type of the field
    std::vector<Constraint*> constraints;  // Constraints applied to the field
};

#endif // FIELD_H
