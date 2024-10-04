#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <map>
#include "Field.h"
#include "../sql/SQLParser.h"

class Table;
class Database {
public:
    // Constructor
    Database();

    // Destructor
    ~Database();

    // Execute a parsed SQL query
    void executeQuery(const SQLParser::Query& query);
    Table* getTable(const std::string& tableName) const;
    std::vector<std::map<std::string, std::string>> executeSelectQuery(const SQLParser::Query& query);


private:
    std::map<std::string, Table*> tables; // Map of table names to Table objects

    // Methods to handle different query types
    void createTable(const SQLParser::Query& query);
    void insertIntoTable(const SQLParser::Query& query);
    void selectFromTable(const SQLParser::Query& query);
    void updateTable(const SQLParser::Query& query);
    void deleteFromTable(const SQLParser::Query& query);
    void dropTable(const SQLParser::Query& query);

    // Helper method to create a Field from ColumnDefinition
    Field* createField(const SQLParser::ColumnDefinition& colDef);
};

#endif // DATABASE_H
