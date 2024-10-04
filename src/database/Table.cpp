#include "../../include/database/Table.h"
#include <stdexcept>
#include <algorithm>

// Constructor
Table::Table(const std::string& name) : name(name) {}

// Destructor
Table::~Table() {
    clearFields();
}

// Constructor with database pointer
Table::Table(const std::string& name, Database* database)
    : name(name), database(database) {
}

bool Table::checkForeignKeyConstraint(const std::string& referencedTableName,
                                      const std::string& referencedColumnName,
                                      const std::string& value) const {
    // Access the referenced table via the database
    Table* referencedTable = database->getTable(referencedTableName);
    if (!referencedTable) {
        throw std::runtime_error("Referenced table not found: " + referencedTableName);
    }

    // Check if the value exists in the referenced table's column
    for (const auto& record : referencedTable->records) {
        auto it = record.find(referencedColumnName);
        if (it != record.end() && it->second == value) {
            return true; // Value exists in the referenced table
        }
    }

    return false; // Value not found, constraint violated
}



/// <summary>
/// Constructs a new Table with the given name.
/// </summary>
/// <param name="name">The name of the table.</param>
void Table::clearFields() {
    for (auto& pair : fields) {
        delete pair.second;
    }
    fields.clear();
}

// Get the fields of the table
const std::map<std::string, Field*>& Table::getFields() const {
    return fields;
}
    
// Add a field to the table
void Table::addField(Field* field) {
    if (fields.find(field->getName()) != fields.end()) {
        throw std::invalid_argument("Field already exists: " + field->getName());
    }
    fields[field->getName()] = field;

    // If the field has a UNIQUE or PRIMARY_KEY constraint, initialize its unique value set
    for (const auto& constraint : field->getConstraints()) {
        std::string constraintName = constraint->getName();
        if (constraintName == "PRIMARY_KEY") {
            uniqueFields[field->getName()] = std::set<std::string>();
            break; // PRIMARY_KEY implies uniqueness, so we can stop checking
        }else if (constraintName == "FOREIGN_KEY_REFERENCES"){

            std::string referencedTable = dynamic_cast<ForeignKeyConstraint*>(constraint)->getReferencedTable();
            std::string referencedColumn = dynamic_cast<ForeignKeyConstraint*>(constraint)->getReferencedColumn();

            if (!database->getTable(referencedTable)) {
                throw std::invalid_argument("Referenced table not found: " + referencedTable);
            }

            if (referencedColumn != field->getName()) {
                throw std::invalid_argument("Referenced column does not match field name: " + referencedColumn);
            }

            // Check if the referenced column is a primary key of the other table
            bool isPrimaryKey = false;
            for (const auto& constraint : database->getTable(referencedTable)->fields.at(referencedColumn)->getConstraints()) {
                if (constraint->getName() == "PRIMARY_KEY") {
                    isPrimaryKey = true;
                    break;
                }
            }
            
            if (!isPrimaryKey) {
                throw std::invalid_argument("Referenced column is not a primary key: " + referencedColumn);
            }
        
        }
        // Add handling for UNIQUE constraint if implemented
    }
}

// Insert a record into the table
void Table::insertRecord(const std::map<std::string, std::string>& record) {
    // Validate fields and enforce constraints
    enforceConstraintsOnInsert(record);

    // Insert the record
    records.push_back(record);

    // Update unique fields
    for (const auto& [fieldName, field] : fields) {
        auto it = uniqueFields.find(fieldName);
        if (it != uniqueFields.end()) {
            const std::string& value = record.at(fieldName);
            it->second.insert(value);
        }
    }
}

// Enforce constraints during insertion
void Table::enforceConstraintsOnInsert(const std::map<std::string, std::string>& record) {
    // Validate and enforce constraints
    for (const auto& [fieldName, field] : fields) {
        auto it = record.find(fieldName);
        if (it != record.end()) {
            const std::string& value = it->second;
            // Field-level validation
            field->validate(value);
        } else {
            // Handle missing field (e.g., default values or error)
            throw std::invalid_argument("Missing value for field: " + fieldName);
        }
    }

    // Enforce table-level constraints
    for (const auto& [fieldName, field] : fields) {
        auto it = record.find(fieldName);
        const std::string& value = it->second;

        for (const auto& constraint : field->getConstraints()) {
            std::string constraintName = constraint->getName();
            if (constraintName == "PRIMARY_KEY") {
                // Enforce uniqueness
                if (uniqueFields[fieldName].find(value) != uniqueFields[fieldName].end()) {
                    throw std::invalid_argument("Primary key constraint violated for field: " + fieldName);
                }
            }
        }
    }

      // Enforce foreign key constraints
    for (const auto& [fieldName, field] : fields) {
        auto it = record.find(fieldName);
        if (it != record.end()) {
            const std::string& value = it->second;

            for (const auto& constraint : field->getConstraints()) {
                if (constraint->getName() == "FOREIGN_KEY") {
                    ForeignKeyConstraint* fkConstraint = dynamic_cast<ForeignKeyConstraint*>(constraint);
                    if (fkConstraint) {
                        if (!checkForeignKeyConstraint(fkConstraint->getReferencedTable(),
                                                       fkConstraint->getReferencedColumn(), value)) {
                            throw std::invalid_argument("Foreign key constraint violated for field: " + fieldName);
                        }
                    }
                }
            }
        }
    }
}

// Select records based on conditions
std::vector<std::map<std::string, std::string>> Table::selectRecords(const std::vector<std::string>& fieldsToSelect,const std::vector<SQLParser::Condition>& conditions) const {

    std::vector<std::map<std::string, std::string>> result;

    for (const auto& record : records) {
        if (evaluateConditions(record, conditions)) {
            // Create a new record with only the selected fields
            std::map<std::string, std::string> selectedRecord;
            if (fieldsToSelect.size() == 1 && fieldsToSelect[0] == "*") {
                selectedRecord = record; // Select all fields
            } else {
                for (const auto& fieldName : fieldsToSelect) {
                    auto it = record.find(fieldName);
                    if (it != record.end()) {
                        selectedRecord[fieldName] = it->second;
                    } else {
                        throw std::invalid_argument("Field not found: " + fieldName);
                    }
                }
            }
            result.push_back(selectedRecord);
        }
    }

    return result;
}

void Table::enforceConstraintsOnUpdate(const std::map<std::string, std::string>& originalRecord,
                                       const std::map<std::string, std::string>& updatedRecord) {
    // Iterate over the fields to be updated
    for (const auto& [fieldName, newValue] : updatedRecord) {
        // Get the original value
        auto itOriginal = originalRecord.find(fieldName);
        const std::string& originalValue = itOriginal != originalRecord.end() ? itOriginal->second : "";

        // Validate the new value
        auto fieldIt = fields.find(fieldName);
        if (fieldIt != fields.end()) {
            Field* field = fieldIt->second;
            field->validate(newValue);

            // Enforce constraints if the value has changed
            if (newValue != originalValue) {
                for (const auto& constraint : field->getConstraints()) {
                    std::string constraintName = constraint->getName();

                    if (constraintName == "PRIMARY_KEY") {
                        // Check uniqueness only if primary key is being modified
                        if (uniqueFields[fieldName].find(newValue) != uniqueFields[fieldName].end()) {
                            throw std::invalid_argument("Primary key constraint violated for field: " + fieldName);
                        }
                    } else if (constraintName == "FOREIGN_KEY") {
                        // Enforce foreign key constraint
                        ForeignKeyConstraint* fkConstraint = dynamic_cast<ForeignKeyConstraint*>(constraint);
                        if (fkConstraint) {
                            if (!checkForeignKeyConstraint(fkConstraint->getReferencedTable(),
                                                           fkConstraint->getReferencedColumn(), newValue)) {
                                throw std::invalid_argument("Foreign key constraint violated for field: " + fieldName);
                            }
                        }
                    }
                    // Add checks for other constraints as needed
                }
            }
        } else {
            throw std::invalid_argument("Field not found: " + fieldName);
        }
    }
}

// Update records based on conditions
void Table::updateRecords(const std::map<std::string, std::string>& newValues, const std::vector<SQLParser::Condition>& conditions) {
    bool updated = false;  
    for (auto& record : records) {
        if (evaluateConditions(record, conditions)) {
            updated = true;
            // Make copies of the original and updated records
            auto originalRecord = record;
            auto updatedRecord = record;

            // Apply the updates to the updatedRecord
            for (const auto& [fieldName, newValue] : newValues) {
                // Validate the new value
                auto fieldIt = fields.find(fieldName);
                if (fieldIt != fields.end()) {
                    Field* field = fieldIt->second;
                    field->validate(newValue);
                    updatedRecord[fieldName] = newValue;
                } else {
                    throw std::invalid_argument("Field not found: " + fieldName);
                }
            }

            // Enforce constraints on the updated record
            enforceConstraintsOnUpdate(originalRecord, updatedRecord);

            // Update unique fields (if necessary)
            for (const auto& [fieldName, newValue] : newValues) {
                if (uniqueFields.find(fieldName) != uniqueFields.end()) {
                    // Check if the value has actually changed
                    const std::string& originalValue = originalRecord[fieldName];
                    if (newValue != originalValue) {
                        // Update uniqueFields map
                        uniqueFields[fieldName].erase(originalValue); // Remove old value
                        uniqueFields[fieldName].insert(newValue);     // Insert new value
                    }
                }
            }

            // Apply the updates
            record = updatedRecord;
        }
    }

    if (!updated) {
        throw std::invalid_argument("No records matched the update conditions.");
    }
}


// Delete records based on conditions
void Table::deleteRecords(const std::vector<SQLParser::Condition>& conditions) {
    auto it = records.begin();
    while (it != records.end()) {
        if (evaluateConditions(*it, conditions)) {
            // Update unique fields (if necessary)
            for (const auto& [fieldName, field] : fields) {
                auto uniqueIt = uniqueFields.find(fieldName);
                if (uniqueIt != uniqueFields.end()) {
                    uniqueIt->second.erase((*it)[fieldName]); // Remove value from unique set
                }
            }
            // Erase the record
            it = records.erase(it);
        } else {
            ++it;
        }
    }
}

// Evaluate conditions for a record
bool Table::evaluateConditions(const std::map<std::string, std::string>& record, const std::vector<SQLParser::Condition>& conditions) const {
    if (conditions.empty()) {
        return true; // No conditions, select all
    }

    bool result = evaluateCondition(record, conditions[0]);
    for (size_t i = 1; i < conditions.size(); ++i) {
        const auto& condition = conditions[i];
        if (condition.relation == "AND") {
            result = result && evaluateCondition(record, condition);
        } else if (condition.relation == "OR") {
            result = result || evaluateCondition(record, condition);
        } else {
            // Should not reach here
            throw std::runtime_error("Unknown condition relation: " + condition.relation);
        }
    }

    return result;
}

// Evaluate a single condition for a record
bool Table::evaluateCondition(const std::map<std::string, std::string>& record, const SQLParser::Condition& condition) const {
    auto it = record.find(condition.field);
    if (it == record.end()) {
        throw std::invalid_argument("Field not found in condition: " + condition.field);
    }
    const std::string& value = it->second;
    const std::string& condValue = condition.value;

    if (condition.op == "=" || condition.op == "==") {
        return value == condValue;
    } else if (condition.op == "!=" || condition.op == "<>") {
        return value != condValue;
    } else if (condition.op == "<") {
        bool check_truth = std::stod(value) < std::stod(condValue);   
        return check_truth;
    } else if (condition.op == ">") {
        bool check_truth = std::stod(value) > std::stod(condValue);   
        return check_truth;
    } else if (condition.op == "<=") {
        bool check_truth = std::stod(value) <= std::stod(condValue);   
        return check_truth;
    } else if (condition.op == ">=") {
        bool check_truth = std::stod(value) >= std::stod(condValue);   
        return check_truth;
    } else {
        throw std::runtime_error("Unsupported operator in condition: " + condition.op);
    }
}

// Get the name of the table
std::string Table::getName() const {
    return name;
}

// Get all records
const std::vector<std::map<std::string, std::string>>& Table::getRecords() const {
    return records;
}

bool evaluateJoinCondition(
    const std::map<std::string, std::string>& leftRecord,
    const std::map<std::string, std::string>& rightRecord,
    const SQLParser::Condition& condition) {

    auto getValue = [&](const std::map<std::string, std::string>& record, const std::string& fieldName) -> std::string {
        size_t dotPos = fieldName.find('.');
        std::string key = (dotPos != std::string::npos) ? fieldName.substr(dotPos + 1) : fieldName;
        auto it = record.find(key);
        if (it != record.end()) {
            return it->second;
        }
        return ""; // Return empty string if not found
    };

    std::string leftValue = getValue(leftRecord, condition.field);
    std::string rightValue = getValue(rightRecord, condition.value); // Assuming 'value' holds the right field

    // Evaluate the condition
    if (condition.op == "=") {
        return leftValue == rightValue;
    } else {
        // Handle other operators if necessary
        throw std::runtime_error("Unsupported operator in join condition: " + condition.op);
    }
}


std::vector<std::map<std::string, std::string>> Table::performInnerJoin(
    const std::vector<std::map<std::string, std::string>>& leftRecords,
    const std::vector<std::map<std::string, std::string>>& rightRecords,
    const SQLParser::Condition& joinCondition,
    const std::string& rightTableName,
    const std::string& leftTableName) {

    std::vector<std::map<std::string, std::string>> result;

    for (const auto& leftRecord : leftRecords) {
        for (const auto& rightRecord : rightRecords) {
            if (evaluateJoinCondition(leftRecord, rightRecord, joinCondition)) {
                // Combine the two records without re-prefixing leftRecord's field names

                // Copy the leftRecord as is
                std::map<std::string, std::string> combinedRecord = leftRecord;

                // Prefix right table fields
                for (const auto& [key, value] : rightRecord) {
                    combinedRecord[rightTableName + "." + key] = value;
                }

                result.push_back(combinedRecord);
            }
        }
    }

    return result;
}

