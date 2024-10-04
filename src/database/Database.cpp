#include "../../include/database/Database.h"
#include "../../include/database/Table.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include <string>
#include <algorithm> // For std::max
#include <functional> // For std::greater
#include <cctype>    // For std::isdigit

#define _PRETTY_PRINT

Database::Database() {}

Database::~Database() {
    // Delete all tables to free memory
    for (auto& pair : tables) {
        delete pair.second;
    }
}

void Database::executeQuery(const SQLParser::Query& query) {
    if (query.operation == "CREATE") {
        createTable(query);
    } else if (query.operation == "INSERT") {
        insertIntoTable(query);
    } else if (query.operation == "SELECT") {
        executeSelectQuery(query);
    } else if (query.operation == "UPDATE") {
        updateTable(query);
    } else if (query.operation == "DELETE") {
        deleteFromTable(query);
    } else if (query.operation == "DROP"){
        dropTable(query);
    }else {
        throw std::runtime_error("Unsupported operation: " + query.operation);
    }
}

Table* Database::getTable(const std::string& tableName) const {
    auto it = tables.find(tableName);
    if (it != tables.end()) {
        return it->second;
    } else {
        return nullptr;
    }
}

void Database::dropTable(const SQLParser::Query& query) {
    // Find the table
    auto it = tables.find(query.table);
    if (it == tables.end()) {
        throw std::runtime_error("Table not found: " + query.table);
    }

    // Delete the table
    delete it->second;
    tables.erase(it);

    std::cout << "Table '" << query.table << "' dropped successfully." << std::endl;
}

void Database::createTable(const SQLParser::Query& query) {
      if (tables.find(query.table) != tables.end()) {
        throw std::runtime_error("Table already exists: " + query.table);
    }

    // Create a new table, passing 'this' as the Database pointer
    Table* newTable = new Table(query.table, this);

    // Add fields to the table
    for (const auto& colDef : query.columns) {
        Field* field = createField(colDef);
        newTable->addField(field);
    }

    // Add the table to the database
    tables[query.table] = newTable;

    std::cout << "Table '" << query.table << "' created successfully." << std::endl;
    
    for (const auto& pair : newTable->getFields()) {
        Field* field = pair.second;
        std::cout << field->getName() << ":";

        // Print data type
        std::cout << " " << field->getDataType()->getName() << std::endl;

        std::cout << "Constraints: ";

        // Print constraints
        for (const auto& constraint : field->getConstraints()) {
            std::cout << constraint->getName();

            // Print referenced table and column for foreign key constraints
            if (constraint->getName() == "FOREIGN_KEY_REFERENCES") {
                ForeignKeyConstraint* fkConstraint = dynamic_cast<ForeignKeyConstraint*>(constraint);
                std::cout << " " << fkConstraint->getReferencedTable() << "." << fkConstraint->getReferencedColumn();
            }

            // Print comma after each constraint except the last one
            if(constraint != field->getConstraints().back()) {
                std::cout << ", ";
            }
        }

        std::cout << std::endl << std::endl;
    }
}

Field* Database::createField(const SQLParser::ColumnDefinition& colDef) {
    // Create the appropriate DataType object based on the column type
    DataType* dataType = nullptr;
    if (colDef.type.find("VARCHAR") == 0) {
        // Extract length from VARCHAR(length)
        size_t startPos = colDef.type.find('(');
        size_t endPos = colDef.type.find(')');
        if (startPos == std::string::npos || endPos == std::string::npos) {
            throw std::runtime_error("Invalid VARCHAR type definition: " + colDef.type);
        }
        size_t length = std::stoul(colDef.type.substr(startPos + 1, endPos - startPos - 1));
        dataType = new VarcharType(length);
    } else if (colDef.type == "INT") {
        dataType = new IntType();
    } else if (colDef.type == "LONGINT") {
        dataType = new LongIntType();
    } else if (colDef.type == "DOUBLE") {
        dataType = new DoubleType();
    } else if (colDef.type == "DATETIME") {
        dataType = new DateTimeType();
    } else {
        throw std::runtime_error("Unsupported data type: " + colDef.type);
    }

    // Create Constraint objects
    std::vector<Constraint*> constraints;
    for (const auto& constraintStr : colDef.constraints) {
        if (constraintStr == "NOT_EMPTY") {
            constraints.push_back(new NotEmptyConstraint());
        } else if (constraintStr == "PRIMARY_KEY") {
            constraints.push_back(new PrimaryKeyConstraint());
        }else if (constraintStr == "FOREIGN_KEY_REFERENCES") {
            constraints.push_back(new ForeignKeyConstraint(
                colDef.referencedTable, colDef.referencedColumn));
        } else {
            throw std::runtime_error("Unsupported constraint: " + constraintStr);
        }

    }

    // Create and return the Field object
    Field* field = new Field(colDef.name, dataType, constraints);
    return field;
}

void Database::insertIntoTable(const SQLParser::Query& query) {
    // Find the table
    auto it = tables.find(query.table);
    if (it == tables.end()) {
        throw std::runtime_error("Table not found: " + query.table);
    }
    Table* table = it->second;

    // Insert records
    for (const auto& recordValues : query.multiValues) {
        std::map<std::string, std::string> record;

        if (query.fields.size() != recordValues.size()) {
            throw std::runtime_error("Field count does not match value count.");
        }

        // Map field names to values
        for (size_t i = 0; i < query.fields.size(); ++i) {
            record[query.fields[i]] = recordValues.at(query.fields[i]);
            std::cout << query.fields[i] << " : " << recordValues.at(query.fields[i]) << std::endl;
        }

        table->insertRecord(record);
    }
    
    std::cout << std::endl;
}

void printQueryResults(const std::vector<std::map<std::string, std::string>>& results) {
    // Check if results are empty
    if (results.empty()) {
        std::cout << "No records found." << std::endl;
        return;
    }

    // Get the list of columns from the first record
    std::vector<std::string> columns;
    for (const auto& [key, value] : results[0]) {
        columns.push_back(key);
    }

    // Compute the maximum width for each column
    std::map<std::string, size_t> columnWidths;
    for (const auto& column : columns) {
        size_t maxWidth = column.length(); // Start with the header length
        for (const auto& record : results) {
            auto it = record.find(column);
            if (it != record.end()) {
                maxWidth = std::max(maxWidth, it->second.length());
            }
        }
        columnWidths[column] = maxWidth;
    }

    // Function to print a separator line
    auto printSeparator = [&]() {
        std::cout << "+";
        for (const auto& column : columns) {
            std::cout << std::string(columnWidths[column] + 2, '-') << "+";
        }
        std::cout << std::endl;
    };

    // Print the header
    printSeparator();
    std::cout << "|";
    for (const auto& column : columns) {
        std::cout << " " << std::left << std::setw(columnWidths[column]) << column << " |";
    }
    std::cout << std::endl;
    printSeparator();

    // Print each record
    for (const auto& record : results) {
        std::cout << "|";
        for (const auto& column : columns) {
            auto it = record.find(column);
            std::string value = (it != record.end()) ? it->second : "";
            // Check if the value is numeric
            bool isNumeric = !value.empty() && std::all_of(value.begin(), value.end(), [](char c) {
                return std::isdigit(c) || c == '.' || c == '-';
            });
            if (isNumeric) {
                std::cout << " " << std::right << std::setw(columnWidths[column]) << value << " |";
            } else {
                std::cout << " " << std::left << std::setw(columnWidths[column]) << value << " |";
            }
        }
        std::cout << std::endl;
    }
    printSeparator();
}


void Database::updateTable(const SQLParser::Query& query) {
    // Find the table
    auto tableIt = tables.find(query.table);
    if (tableIt == tables.end()) {
        throw std::runtime_error("Table not found: " + query.table);
    }
    Table* table = tableIt->second;

    // Use query.values directly since it's a map of field names to new values
    const std::map<std::string, std::string>& newValues = query.values;

    // Update records
    table->updateRecords(newValues, query.conditions);

    std::cout << "Records updated in table '" << query.table << "'." << std::endl;
}


void Database::deleteFromTable(const SQLParser::Query& query) {
    // Find the table
    auto it = tables.find(query.table);
    if (it == tables.end()) {
        throw std::runtime_error("Table not found: " + query.table);
    }
    Table* table = it->second;

    // Delete records
    table->deleteRecords(query.conditions);

    std::cout << "Records deleted from table '" << query.table << "'." << std::endl;
}

bool evaluateCombinedConditions(
    const std::map<std::string, std::string>& record,
    const std::vector<SQLParser::Condition>& conditions) {

    bool result = true;
    std::string lastRelation = "AND";

    for (const auto& condition : conditions) {
        // Extract the value from the record
        auto it = record.find(condition.field);
        if (it == record.end()) {
            throw std::runtime_error("Field not found in record: " + condition.field);
        }
        std::string fieldValue = it->second;

        // Compare with the condition value
        bool conditionResult = false;
        if (condition.op == "=" || condition.op == "==") {
            conditionResult = (fieldValue == condition.value);
        } else if (condition.op == "!=" || condition.op == "<>") {
            conditionResult = (fieldValue != condition.value);
        } else if (condition.op == "<") {
            conditionResult = (std::stod(fieldValue) < std::stod(condition.value));
        } else if (condition.op == ">") {
            conditionResult = (std::stod(fieldValue) > std::stod(condition.value));
        } else if (condition.op == "<=") {
            conditionResult = (std::stod(fieldValue) <= std::stod(condition.value));
        } else if (condition.op == ">=") {
            conditionResult = (std::stod(fieldValue) >= std::stod(condition.value));
        } else {
            throw std::runtime_error("Unsupported operator in condition: " + condition.op);
        }

        // Combine condition results based on logical relations
        if (lastRelation == "AND") {
            result = result && conditionResult;
        } else if (lastRelation == "OR") {
            result = result || conditionResult;
        }

        lastRelation = condition.relation.empty() ? "AND" : condition.relation;
    }

    return result;
}

std::vector<std::map<std::string, std::string>> Database::executeSelectQuery(const SQLParser::Query& query) {
    //check if the table exists
    if (tables.find(query.table) == tables.end()) {
        throw std::runtime_error("Table not found: " + query.table);
    } 

    // Get the primary table
    Table& primaryTable = *tables.at(query.table);

    // If there are no joins, use selectRecords directly
    if (query.joins.empty()) {
        std::vector<std::map<std::string, std::string>> finalResults = primaryTable.selectRecords(query.fields, query.conditions);
        printQueryResults(finalResults);
        return finalResults;
    }

    // Process INNER JOINs
    std::vector<std::map<std::string, std::string>> result;
    std::vector<std::map<std::string, std::string>> currentRecords = primaryTable.records;

    for (const auto& join : query.joins) {
        // Check if the joined table exists
        if (tables.find(join.table) == tables.end()) {
            throw std::runtime_error("Table not found: " + join.table);
        }
        
        // Get the joined table
        Table& joinTable = *tables.at(join.table);

        // Parse the join condition
        std::vector<SQLParser::Condition> joinConditions;
        SQLParser::parse_conditions(join.onCondition, joinConditions);
        SQLParser::Condition joinCondition = joinConditions[0];

        // Perform INNER JOIN
        result = Table::performInnerJoin(currentRecords,joinTable.records,joinCondition,joinTable.getName(),this->getTable(query.table)->getName());

        // Update currentRecords for the next join (if any)
        currentRecords = result;
    }

    // Apply WHERE conditions to the combined records
    std::vector<std::map<std::string, std::string>> filteredResults;
    for (const auto& record : currentRecords) {
        if (evaluateCombinedConditions(record, query.conditions)) {
            filteredResults.push_back(record);
        }
    }

    // Select the requested fields
    std::vector<std::map<std::string, std::string>> finalResults;
    for (const auto& record : filteredResults) {
        std::map<std::string, std::string> selectedRecord;
        if (query.fields.size() == 1 && query.fields[0] == "*") {
            selectedRecord = record; // Select all fields
        } else {
            for (const auto& fieldName : query.fields) {
                auto it = record.find(fieldName);
                if (it != record.end()) {
                    selectedRecord[fieldName] = it->second;
                } else {
                    throw std::invalid_argument("Field not found: " + fieldName);
                }
            }
        }
        finalResults.push_back(selectedRecord);
    }

    printQueryResults(finalResults);

    return finalResults;
}

