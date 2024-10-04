#ifndef SQLPARSER_H
#define SQLPARSER_H

#include <string>
#include <vector>
#include <map>

class SQLParser {
public:

    struct Condition {
        std::string field;
        std::string op;
        std::string value;
        std::string relation;  // Relation with the next condition (AND, OR, or empty)
    };

    struct Join {
        std::string table;
        std::string onCondition;  // Join condition
    };

    struct ColumnDefinition {
        std::string name;
        std::string type;
        std::vector<std::string> constraints;
        // New fields to store foreign key reference details
        std::string referencedTable;
        std::string referencedColumn;
    };
    
    // Struct to hold parsed query components
   struct Query {
        std::string operation;
        std::vector<std::string> fields;
        std::string table;
        std::vector<Condition> conditions;
        std::vector<Join> joins;
        std::map<std::string, std::string> values; // For single set of values (used in UPDATE)
        std::vector<std::map<std::string, std::string>> multiValues; // For multiple sets of values (used in INSERT)
        std::vector<ColumnDefinition> columns; // For CREATE TABLE columns
    };

    // Method to parse a SQL query string
    static Query parse(const std::string& sql);
    static std::string trim(const std::string& str);
    static void parse_conditions(const std::string& condition_str, std::vector<Condition>& conditions);
};

#endif // SQLPARSER_H
