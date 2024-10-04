# Simple RDBMS in C++

A simple relational database management system (RDBMS) implemented in C++. This project supports basic SQL operations such as `SELECT`, `INSERT`, `UPDATE`, and `DELETE`, along with features like primary keys, foreign keys, and joins.

## Table of Contents

-   [Features](#features)
-   [Getting Started](#getting-started)
    -   [Prerequisites](#prerequisites)
    -   [Building the Project](#building-the-project)
-   [Usage](#usage)
    -   [Command-Line Interface](#command-line-interface)
    -   [Executing SQL Commands](#executing-sql-commands)
        -   [Interactive Mode](#interactive-mode)
        -   [Executing from a File](#executing-from-a-file)
-   [Supported SQL Commands](#supported-sql-commands)
    -   [SELECT](#select)
    -   [INSERT](#insert)
    -   [UPDATE](#update)
    -   [DELETE](#delete)
    -   [JOINs](#joins)
-   [Examples](#examples)
    -   [Inserting Data](#inserting-data)
    -   [Querying Data](#querying-data)
    -   [Updating Data](#updating-data)
    -   [Deleting Data](#deleting-data)

## Features

-   **SQL Parsing**: Parses SQL statements using a custom SQL parser.
-   **Data Manipulation**: Supports `SELECT`, `INSERT`, `UPDATE`, `DELETE` and `DROP` operations.
-   **Constraints Enforcement**: Enforces primary key and foreign key constraints.
-   **Joins**: Supports `INNER JOIN (single for now)` operations.
-   **Command-Line Interface**: Interactive CLI for executing SQL commands.
-   **File Execution**: Ability to execute SQL commands from a file.

## Getting Started

### Prerequisites

-   **C++17 Compiler**: Ensure you have a C++ compiler that supports C++17.
-   **CMake**: Version 3.10 or higher.

### Building the Project

1. **Clone the Repository**

    ```bash
    git clone https://github.com/yourusername/RelationalDatabase.git
    cd RelationalDatabase

    ```

2. **Create a Build Directory**

    ```bash
    mkdir build
    cd build
    ```

3. **Configure the Project with CMake**

    ```bash
     cmake ..
    ```

4. **Build the Project**
    ```bash
     make
    ```

## Usage

### Command-Line Interface

Run the `RelationalDatabase` executable to start the command-line interface (CLI):

```bash
./RelationalDatabase
```

You will be presented with a prompt where you can enter SQL commands or CLI commands.

### Executing SQL Commands

#### Interactive Mode

-   **Single-Line Commands**: Type your SQL command ending with a semicolon (`;`), and press `Enter`.

```sql
Enter the SQL command or CLI command: SELECT * FROM Customers;
```

-   **Note**: For this implementation, please ensure your SQL commands are entered on a single line and **each "token" is separated with space** (parentheseis, commas, words).

#### Executing From a File

-   To execute SQL commands from a file, use the i "filename" command and **each "token" is separated with space** (parentheseis, commas, words).

```sql
Enter the SQL command or CLI command: i "commands.sql"
```

-   Replace "commands.sql" with the path to your SQL file. The file should contain SQL commands separated by semicolons, with each command on a single line.

## Supported SQL Commands

### SELECT

Retrieve data from one or more tables.

**Syntax**:

```sql
SELECT column1, column2, ... FROM table_name [INNER JOIN other_table ON condition] [WHERE condition];
```

### INSERT

Add new records to a table.

**Syntax**:

```sql
INSERT INTO table_name ( column1 , column2, ... ) VALUES ( value1 , value2 , ... );

```

### UPDATE

Modify existing records in a table.

**Syntax**:

```sql
UPDATE table_name SET column1 = value1 , column2 = value2 , ... WHERE condition;
```

### DELETE

Remove records from a table.

**Syntax**:

```sql
DELETE FROM table_name WHERE condition;
```

### JOINs

Combine rows from two or more tables based on related columns.

**Supported**:

-   INNER JOIN

**Syntax**:

```sql
SELECT columns FROM table1 INNER JOIN table2 ON table1.column_name = table2.column_name [WHERE condition];
```

## Examples

### Creating Tables

```sql
CREATE TABLE Customers ( CustomerID INT PRIMARY_KEY , FirstName VARCHAR(100), LastName VARCHAR(100), Email VARCHAR(100), City VARCHAR(100) ) ;
```

### Inserting Data

```sql
INSERT INTO Customers ( CustomerID , FirstName , LastName , Email , City ) VALUES ( 1 , 'Alice' , 'Smith' , 'alice@example.com' , 'New York' );
```

### Querying Data

```sql
SELECT FirstName , LastName FROM Customers WHERE City = 'New York';
```

**With join:**

```sql
SELECT Orders.OrderID , Customers.FirstName , Customers.LastName FROM Orders INNER JOIN Customers ON Orders.CustomerID = Customers.CustomerID WHERE Orders.TotalAmount > 100.00;
```

### Updating Data

```sql
UPDATE Customers SET Email = 'newemail@example.com' WHERE CustomerID = 1;
```

### Deleting Data

```sql
DELETE FROM Customers WHERE CustomerID = 1;
```
