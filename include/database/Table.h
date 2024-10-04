#ifndef TABLE_H
#define TABLE_H

#include <string>
#include <map>
#include <vector>
#include <set>
#include "Field.h"
#include "Database.h"
#include "../sql/SQLParser.h"

class Table {
public:
    // Constructor
    Table() = default;

    Table(const std::string& name);

    Table(const std::string& name, Database* database);

    // Destructor
    ~Table();

    // Add a field to the table
    void addField(Field* field);

    // Insert a record into the table
    void insertRecord(const std::map<std::string, std::string>& record);

    // Select records based on conditions
    std::vector<std::map<std::string, std::string>> selectRecords(
        const std::vector<std::string>& fieldsToSelect,
        const std::vector<SQLParser::Condition>& conditions) const;

    // Update records based on conditions
    void updateRecords(
        const std::map<std::string, std::string>& newValues,
        const std::vector<SQLParser::Condition>& conditions);

    void enforceConstraintsOnUpdate(const std::map<std::string, std::string> &originalRecord, const std::map<std::string, std::string> &updatedRecord);

    // Delete records based on conditions
    void deleteRecords(const std::vector<SQLParser::Condition>& conditions);

    // Get the name of the table
    std::string getName() const;

    // Get all records (for testing or other purposes)
    const std::vector<std::map<std::string, std::string>>& getRecords() const;

    // Get the fields of the table
    const std::map<std::string, Field*>& getFields() const;

    bool checkForeignKeyConstraint(const std::string& referencedTableName,
                                   const std::string& referencedColumnName,
                                   const std::string& value) const;


    static std::vector<std::map<std::string, std::string>> performInnerJoin(
        const std::vector<std::map<std::string, std::string>>& leftRecords,
        const std::vector<std::map<std::string, std::string>>& rightRecords,
        const SQLParser::Condition& joinCondition,
        const std::string& rightTableName,
        const std::string& leftTableName);

     std::string name;
     std::vector<std::map<std::string, std::string>> records;

private:
    Database* database;
    std::map<std::string, Field*> fields;

    // Indexes for enforcing constraints (e.g., primary keys)
    std::map<std::string, std::set<std::string>> uniqueFields; // Field name -> set of unique values

    // Helper methods to enforce table-level constraints
    void enforceConstraintsOnInsert(const std::map<std::string, std::string>& record);
    bool evaluateConditions(const std::map<std::string, std::string>& record, const std::vector<SQLParser::Condition>& conditions) const;
    bool evaluateCondition(const std::map<std::string, std::string>& record, const SQLParser::Condition& condition) const;

    // Memory management helpers
    void clearFields();
};

#endif // TABLE_H
