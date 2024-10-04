#include "../../include/sql/SQLParser.h"
#include <sstream>
#include <algorithm>
#include <regex>
#include <iostream>
#include <cctype> // For std::isspace
#include <stdexcept> // For std::runtime_error

// Helper function to trim whitespace from both ends of a string
std::string SQLParser::trim(const std::string& str) {
    auto start = std::find_if_not(str.begin(), str.end(), [](unsigned char ch) { return std::isspace(ch); });
    auto end = std::find_if_not(str.rbegin(), str.rend(), [](unsigned char ch) { return std::isspace(ch); }).base();
    return (start < end ? std::string(start, end) : std::string());
}

// Helper function to remove surrounding quotes from a string and handle escaped quotes
std::string remove_quotes(const std::string& str) {
    if (str.size() >= 2 &&
        ((str.front() == '\'' && str.back() == '\'') ||
         (str.front() == '"' && str.back() == '"'))) {
        // Extract the substring inside the quotes
        std::string unquoted = str.substr(1, str.size() - 2);
        // Trim leading and trailing whitespace inside the quotes
        unquoted = SQLParser::trim(unquoted);
        // Replace escaped quotes with actual quotes
        std::string::size_type pos = 0;
        std::string escaped_quote = str.front() == '\'' ? "\\'" : "\\\"";
        while ((pos = unquoted.find(escaped_quote, pos)) != std::string::npos) {
            unquoted.replace(pos, 2, std::string(1, str.front()));
            pos += 1;
        }
        return unquoted;
    }
    // If there are no surrounding quotes, trim the entire string
    return SQLParser::trim(str);
}


// Function to convert a string to uppercase
std::string to_upper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c){ return std::toupper(c); });
    return result;
}

// Helper function to join strings with a delimiter
std::string join(const std::vector<std::string>& tokens, const std::string& delimiter) {
    std::ostringstream result;
    for (size_t i = 0; i < tokens.size(); ++i) {
        result << tokens[i];
        if (i != tokens.size() - 1) {
            result << delimiter;
        }
    }
    return result.str();
}

std::vector<std::string> acceptableConstraints = {
    "PRIMARY_KEY",
    "NOT_EMPTY",
    "FOREIGN_KEY_REFERENCES",
};

void SQLParser::parse_conditions(const std::string& condition_str, std::vector<SQLParser::Condition>& conditions) {
    // Regex to match conditions and logical operators in sequence
    std::regex tokenRegex(R"(([\w.]+\s*(?:[<>!=]+|\bLIKE\b|\bIN\b)\s*(?:'[^']*'|"[^"]*"|\S+)|\bAND\b|\bOR\b))", std::regex_constants::icase);

    std::vector<std::string> tokens;
    auto tokens_begin = std::sregex_iterator(condition_str.begin(), condition_str.end(), tokenRegex);
    auto tokens_end = std::sregex_iterator();

    // Collect tokens in order
    for (auto it = tokens_begin; it != tokens_end; ++it) {
        tokens.push_back(it->str());
    }

    std::string currentRelation = ""; // Initialize current logical operator to empty

    for (size_t i = 0; i < tokens.size(); ++i) {
        std::string token = SQLParser::trim(tokens[i]);
        std::string token_upper = to_upper(token);

        if (token_upper == "AND" || token_upper == "OR") {
            // Update currentRelation for the next condition
            currentRelation = token_upper;
        } else {
            // It's a condition
            // Parse the condition using the conditionRegex
            std::regex conditionRegex(R"(([\w.]+)\s*([<>!=]+|\bLIKE\b|\bIN\b)\s*(('[^']*'|"[^"]*"|\S+)))", std::regex_constants::icase);
            std::smatch match;
            if (std::regex_match(token, match, conditionRegex)) {
                SQLParser::Condition cond;
                cond.field = to_upper(SQLParser::trim(match.str(1)));
                cond.op = SQLParser::trim(match.str(2));
                cond.value = SQLParser::trim(match.str(3));

                // Remove surrounding quotes from the value if present
                cond.value = remove_quotes(cond.value);

                // Assign the logical operator (relation) to the current condition
                cond.relation = currentRelation;

                // Reset currentRelation after assigning
                currentRelation = "";

                conditions.push_back(cond);
            } else {
                // Handle invalid condition syntax if necessary
                std::cerr << "Invalid condition syntax: " << token << std::endl;
            }
        }
    }
}


// Helper function to parse field-value pairs
std::map<std::string, std::string> parse_field_value_pairs(const std::string& clause) {
    std::map<std::string, std::string> values;
    std::istringstream setStream(clause);
    std::string assignment;

    while (std::getline(setStream, assignment, ',')) {
        assignment = SQLParser::trim(assignment);
        auto pos = assignment.find('=');
        if (pos != std::string::npos) {
            auto field = SQLParser::trim(assignment.substr(0, pos));
            auto value = SQLParser::trim(assignment.substr(pos + 1));
            value = remove_quotes(value);
            values[to_upper(field)] = value;
        }
    }

    return values;
}

SQLParser::Query SQLParser::parse(const std::string& sql) {
    SQLParser::Query query;
    std::istringstream stream(sql);
    std::string token;

    // Get the operation (SELECT, INSERT, UPDATE, DELETE, CREATE, DROP)
    stream >> token;
    token = to_upper(token);
    query.operation = token;

    if (query.operation.empty()) {
        throw std::runtime_error("No operation specified in the SQL query.");
    }

    if (query.operation == "SELECT") {
        // Read tokens until "FROM"
        std::vector<std::string> fieldTokens;
        while (stream >> token) {
            token = to_upper(token);
            if (token == "FROM") {
                break;
            }
            fieldTokens.push_back(token);
        }

        if (fieldTokens.empty()) {
            throw std::runtime_error("No fields specified in SELECT statement.");
        }

        // Join field tokens into a single string and parse fields
        std::string fields = join(fieldTokens, " ");
        std::istringstream fieldsStream(fields);
        while (std::getline(fieldsStream, token, ',')) {
            query.fields.push_back(trim(token));
        }

        // Now read the table name
        if (!(stream >> query.table)) {
            throw std::runtime_error("No table specified in SELECT statement.");
        }
        
        query.table = to_upper(query.table);

        std::string nextToken;
        while (stream >> nextToken) {
            nextToken = to_upper(nextToken);
            if (nextToken == "INNER") {
                // Parse INNER JOIN
                while (nextToken == "INNER") {
                    Join join;
                    stream >> nextToken;  // Should be JOIN
                    nextToken = to_upper(nextToken);
                    if (nextToken == "JOIN") {
                        stream >> join.table;
                        join.table = to_upper(join.table);

                        stream >> nextToken;  // Should be ON
                        nextToken = to_upper(nextToken);
                        if (nextToken == "ON") {
                            // Read until the next keyword (WHERE or INNER)
                            std::string joinCondition;
                            std::string word;
                            while (stream >> word) {
                                std::string upperWord = to_upper(word);
                                if (upperWord == "WHERE" || upperWord == "INNER") {
                                    nextToken = upperWord;
                                    break;
                                }
                                joinCondition += word + " ";
                            }
                            join.onCondition = trim(joinCondition);
                            join.onCondition = to_upper(join.onCondition);
                            query.joins.push_back(join);
                            if (nextToken != "INNER") {
                                break;
                            }
                        } else {
                            throw std::runtime_error("Expected 'ON' after 'INNER JOIN' in SELECT statement.");
                        }
                    } else {
                        throw std::runtime_error("Expected 'JOIN' after 'INNER' in SELECT statement.");
                    }
                }
            }

            if (nextToken == "WHERE") {
                std::string condition;
                std::getline(stream, condition);  // get the rest of the line as condition
                parse_conditions(condition, query.conditions);
                for(Condition &c : query.conditions){
                    c.field = to_upper(c.field);
                }
                break; // Assuming WHERE is the last clause
            }
        }
    }
    else if (query.operation == "INSERT") {
        stream >> token; // Should be INTO
        token = to_upper(token);
        if (token != "INTO") {
            throw std::runtime_error("Expected 'INTO' keyword in INSERT statement.");
        }
        if (!(stream >> query.table)) {
            throw std::runtime_error("No table specified in INSERT statement.");
        }

        query.table = to_upper(query.table);
        // Parse fields
        stream >> token; // Should be '('
        if (token != "(") {
            throw std::runtime_error("Expected '(' after table name in INSERT statement.");
        }
        std::getline(stream, token, ')');
        std::istringstream fieldsStream(token);
        std::string field;
        while (std::getline(fieldsStream, field, ',')) {
            query.fields.push_back(to_upper(trim(field)));
        }

        stream >> token; // Should be VALUES
        token = to_upper(token);
        if (token != "VALUES") {
            throw std::runtime_error("Expected 'VALUES' keyword in INSERT statement.");
        }

        std::string valuesLine;
        std::getline(stream, valuesLine);

        // Use regex to match value groups
        std::regex valuesRegex(R"(\(([^)]+)\))");
        auto valuesBegin = std::sregex_iterator(valuesLine.begin(), valuesLine.end(), valuesRegex);
        auto valuesEnd = std::sregex_iterator();

        for (auto iter = valuesBegin; iter != valuesEnd; ++iter) {
            std::smatch match = *iter;
            std::string valueGroup = match.str(1);
            std::istringstream valueStream(valueGroup);
            std::string value;
            std::map<std::string, std::string> valueMap;
            size_t fieldIndex = 0;
            while (std::getline(valueStream, value, ',')) {
                value = trim(value);
                value = remove_quotes(value);
                if (fieldIndex >= query.fields.size()) {
                    throw std::runtime_error("Mismatched number of values in INSERT statement.");
                }
                valueMap[query.fields[fieldIndex]] = value;
                fieldIndex++;
            }
            if (fieldIndex != query.fields.size()) {
                throw std::runtime_error("Mismatched number of values in INSERT statement.");
            }
            query.multiValues.push_back(valueMap);
        }
    }
    else if (query.operation == "UPDATE") {
        if (!(stream >> query.table)) {
            throw std::runtime_error("No table specified in UPDATE statement.");
        }

        query.table = to_upper(query.table);

        // Read until "SET"
        stream >> token; // Should be "SET"
        token = to_upper(token);
        if (token != "SET") {
            throw std::runtime_error("Expected 'SET' keyword in UPDATE statement.");
        }

        // Read the rest of the line
        std::string restOfLine;
        std::getline(stream, restOfLine);

        // Split into SET and WHERE clauses
        size_t wherePos = restOfLine.find("WHERE");
        std::string setClause, whereClause;
        if (wherePos != std::string::npos) {
            setClause = restOfLine.substr(0, wherePos);
            whereClause = restOfLine.substr(wherePos + 5); // Skip "WHERE"
        } else {
            setClause = restOfLine;
        }

        // Parse SET clause
        query.values = parse_field_value_pairs(setClause);

        // Parse WHERE clause if present
        if (!whereClause.empty()) {
            parse_conditions(whereClause, query.conditions);
        }
    }
    else if (query.operation == "DELETE") {
        stream >> token; // Should be FROM
        token = to_upper(token);
        if (token != "FROM") {
            throw std::runtime_error("Expected 'FROM' keyword in DELETE statement.");
        }
        if (!(stream >> query.table)) {
            throw std::runtime_error("No table specified in DELETE statement.");
        }

        query.table = to_upper(query.table);

        // Parse WHERE conditions if any
        while (stream >> token) {
            token = to_upper(token);
            if (token == "WHERE") {
                std::string condition;
                std::getline(stream, condition);  // get the rest of the line as condition
                parse_conditions(condition, query.conditions);
                break;
            }
        }
    }else if (query.operation == "CREATE") {
        stream >> token; // TABLE
        token = to_upper(token);
        if (token == "TABLE") {
            stream >> query.table;
            query.table = to_upper(query.table);    

            // Read the columns definition between '(' and ')'
            std::string columnsDefinition;
            std::getline(stream, columnsDefinition, ';');
            size_t startPos = columnsDefinition.find('(');
            size_t endPos = columnsDefinition.rfind(')');
            if (startPos == std::string::npos || endPos == std::string::npos) {
                throw std::runtime_error("Invalid column definitions in CREATE TABLE statement.");
            }
            std::string columns = columnsDefinition.substr(startPos + 1, endPos - startPos - 1);

            std::istringstream columnsStream(columns);
            std::string columnDefStr;
            while (std::getline(columnsStream, columnDefStr, ',')) {
                columnDefStr = trim(columnDefStr);
                std::istringstream columnStream(columnDefStr);
                ColumnDefinition colDef;

                // Read the column name and type
                columnStream >> colDef.name >> colDef.type;
                colDef.name = to_upper(colDef.name);
                colDef.type = to_upper(colDef.type);

                // Parse constraints and foreign key references
                std::string constraintToken;
                while (columnStream >> constraintToken) {
                    constraintToken = to_upper(constraintToken);

                    if (constraintToken == "PRIMARY_KEY" || constraintToken == "NOT_EMPTY") {
                        colDef.constraints.push_back(constraintToken);
                    } else if (constraintToken == "FOREIGN_KEY_REFERENCES") {
                        // Parse the referenced table and column
                        std::string referenceStr;
                        if (!(columnStream >> referenceStr)) {
                            throw std::runtime_error("Expected referenced table and column after FOREIGN_KEY_REFERENCES.");
                        }
                        size_t dotPos = referenceStr.find('.');
                        if (dotPos == std::string::npos) {
                            throw std::runtime_error("Invalid FOREIGN_KEY_REFERENCES format. Expected another_table.column_name.");
                        }
                        colDef.referencedTable = to_upper(referenceStr.substr(0, dotPos));
                        colDef.referencedColumn = to_upper(referenceStr.substr(dotPos + 1));
                        colDef.constraints.push_back("FOREIGN_KEY_REFERENCES");
                    } else {
                        throw std::runtime_error("Invalid constraint: " + constraintToken);
                    }
                }

                query.columns.push_back(colDef);
            }
        } else {
            throw std::runtime_error("Expected 'TABLE' keyword after 'CREATE'.");
        }
    }else if (query.operation == "DROP") {
        stream >> token; // Should be TABLE
        token = to_upper(token);
        if (token == "TABLE") {
            if (!(stream >> query.table)) {
                throw std::runtime_error("No table specified in DROP TABLE statement.");
            }
        } else {
            throw std::runtime_error("Expected 'TABLE' keyword in DROP statement.");
        }
    }
    else {
        throw std::runtime_error("Unsupported SQL operation: " + query.operation);
    }

    return query;
}

