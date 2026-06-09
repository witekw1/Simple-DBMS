#pragma once
#include <set>
#include <string>

class Where {
    inline static const std::set<std::string> valid_ops{"<", ">", "==", "!=", "<=", ">="};

public:
    Where() = default;
    // parsing the "WHERE" statement
    explicit Where(const std::string &statement);
    [[nodiscard]] bool empty() const;
    std::string lhs;
    std::string op;
    std::string rhs;
};

class invalid_statement : std::exception {
public:
    const char *what() const noexcept override { return "Invalid arguments after WHERE\n"; }
};
