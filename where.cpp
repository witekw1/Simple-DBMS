#include "where.h"
#include <sstream>
#include "database.h"

using namespace std;


Where::Where(const std::string &statement) {
    stringstream ss(statement);
    ss >> lhs >> op >> rhs;
    if (!ss.eof() || lhs.empty() || rhs.empty() || !valid_ops.contains(op)) {
        throw invalid_statement();
    }
}

bool Where::empty() const { return lhs.empty() && op.empty() && rhs.empty(); }
