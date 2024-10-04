// Constraint.h

#ifndef CONSTRAINT_H
#define CONSTRAINT_H
#include <string>
#include <stdexcept>

class Constraint {
public:
    virtual ~Constraint() {}
    virtual std::string getName() const = 0;
    virtual void check(const std::string& value) const = 0;
};

class NotEmptyConstraint : public Constraint {
public:
    std::string getName() const override { return "NOT_EMPTY"; }
    void check(const std::string& value) const override {
        if (value.empty()) {
            throw std::runtime_error("Value cannot be empty.");
        }
    }
};

class PrimaryKeyConstraint : public Constraint {
public:
    std::string getName() const override { return "PRIMARY_KEY"; }
    void check(const std::string& value) const override {
        // Should be enforced at the table level
    }
};

class ForeignKeyConstraint : public Constraint {
public:
    ForeignKeyConstraint(const std::string& referencedTable, const std::string& referencedColumn)
        : referencedTable(referencedTable), referencedColumn(referencedColumn) {}

    std::string getName() const override { return "FOREIGN_KEY_REFERENCES"; }

    void check(const std::string& value) const override {
        // Should be enforced at the table level
    }

    const std::string& getReferencedTable() const { return referencedTable; }
    const std::string& getReferencedColumn() const { return referencedColumn; }

private:
    std::string referencedTable;
    std::string referencedColumn;
};

#endif // CONSTRAINT_H
