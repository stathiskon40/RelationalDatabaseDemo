// #include "../include/database/Table.h"
// #include "../include/database/Field.h"
// #include "../include/database/Constraint.h"
// #include "../include/sql/SQLParser.h"
// #include "../include/database/Datatype.h"
// #include <iostream>
// #include <string>
// #include "sql/SQLParser.h"
// #include "database/Database.h"

// #include <iostream>
// #include <fstream>
// #include <string>

// void read_from_file (std::string filename ,  Database& db) {
//     std::ifstream inputFile(filename);

//     std::string sql;

//     if (!inputFile) {
//         std::cerr << "Unable to open file.\n";
//         return;
//     }

//     std::string line;
//     SQLParser parser = SQLParser();

//     // Read the file line by line
//     while (std::getline(inputFile, line)) {
//         // Remove the trailing semicolon
//         if (!line.empty() && line.back() == ';') {
//             line.pop_back(); // Remove the semicolon
//         }

//         try
//         {
//             sql += line;
//             SQLParser::Query query = parser.parse(sql);
//             db.executeQuery(query);
//             sql.clear();
//         }
//         catch(const std::exception& e)
//         {
//             std::cerr << e.what() << '\n';
//             sql.clear();
//         }
        
 
//     }
//     inputFile.close();
// }

// int main() {

//     Database db = Database();
//     while(1){
//         std::string input;
//         if (input == "exit") {
//             break;
//         }
//         std::cout << "Enter the SQL command or cli command: ";
//         std::getline(std::cin, input);
//         try
//         {
//             SQLParser parser = SQLParser();
//             SQLParser::Query query = parser.parse(input);
//             db.executeQuery(query);
//         }
//         catch(const std::exception& e)
//         {
//             std::cerr << e.what() << '\n';
//         }


//     //     std::cout << "Enter the filename: ";
//     //     std::cin >> filename;
//     //     read_from_file(filename, db);
//     // }


//     return 0;
// }

#include "../include/database/Table.h"
#include "../include/database/Field.h"
#include "../include/database/Constraint.h"
#include "../include/sql/SQLParser.h"
#include "../include/database/Datatype.h"
#include "database/Database.h"

#include <iostream>
#include <fstream>
#include <string>

// ... (rest of your includes and code)

void read_from_file(const std::string& filename, Database& db) {
    std::ifstream inputFile(filename);

    if (!inputFile) {
        std::cerr << "Unable to open file: " << filename << std::endl;
        return;
    }

    std::string line;
    std::string sql;
    SQLParser parser;

    // Read the file line by line
    while (std::getline(inputFile, line)) {
        // Append the line to the current SQL command
        sql += line + "\n";

        // Check if the line ends with a semicolon
        if (!line.empty() && line.back() == ';') {
            try {
                SQLParser::Query query = parser.parse(sql);
                db.executeQuery(query);
            } catch (const std::exception& e) {
                std::cerr << e.what() << std::endl;
            }
            sql.clear(); // Reset for the next command
        }
    }

    // Handle any remaining SQL command without a trailing semicolon
    if (!sql.empty()) {
        try {
            SQLParser::Query query = parser.parse(sql);
            db.executeQuery(query);
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }

    inputFile.close();
}

int main() {
    Database db;

    while (true) {
        std::cout << "Enter the SQL command or CLI command: ";
        std::string input;
        std::getline(std::cin, input);

        // Trim leading and trailing whitespace
        input.erase(0, input.find_first_not_of(" \t\n\r"));
        input.erase(input.find_last_not_of(" \t\n\r") + 1);

        // Check for 'exit' command
        if (input == "exit") {
            break;
        }

        try {
            // Check if the command is 'i "filename"' or 'i filename'
            if (input.length() > 2 && input[0] == 'i' && (input[1] == ' ' || input[1] == '\t')) {
                std::string filename = input.substr(2);
                // Trim leading whitespace
                filename.erase(0, filename.find_first_not_of(" \t\n\r"));

                // Remove surrounding quotes if present
                if (!filename.empty() && ((filename.front() == '"' && filename.back() == '"') ||
                                          (filename.front() == '\'' && filename.back() == '\''))) {
                    filename = filename.substr(1, filename.size() - 2);
                }

                read_from_file(filename, db);
            } else {
                // Process SQL command
                SQLParser parser;
                std::string sql = input;

                // Continue reading lines if the command is incomplete (no semicolon at the end)
                while (!sql.empty() && sql.back() != ';') {
                    std::cout << "-> ";
                    std::string nextLine;
                    std::getline(std::cin, nextLine);
                    sql += "\n" + nextLine;
                }

                // Remove the trailing semicolon
                if (!sql.empty() && sql.back() == ';') {
                    sql.pop_back();
                }

                SQLParser::Query query = parser.parse(sql);
                db.executeQuery(query);
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }

    return 0;
}


