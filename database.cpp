#include "database.h"
#include <algorithm>
#include <iostream>
#include <ranges>


bool validateFields(std::vector<std::string> fields1, nlohmann::json fields2) {
    // for (auto x: fields2.items()) std::cout << x.key() << " ";
    // std::cout << std::endl;
    // for (auto x: fields1) std::cout << x << " ";
    // std::cout << std::endl;
    if (fields1.size() != fields2.size())
        return false;
    for (const auto &[field1, field2]: std::views::zip(fields1, fields2.items())) {
        if (field1 != field2.key() || field1 == "key")
            return false;
    }
    return true;
}

OperationResult Database::put(const std::string &k, const nlohmann::json &val) {
    if (!validateFields(fields_, val))
        return OperationResult::INVALID_FIELDS;
    if (storage.exists(k) && storage[k] == val)
        return OperationResult::ALREADY_EXISTS;
    if (storage.exists(k) && storage[k] != val) {
        storage[k] = val;
        return OperationResult::OVERWRITE;
    }
    storage.insert({k, val});
    ++rows_;
    return OperationResult::SUCCESS;
}

std::vector<OperationResult> Database::put(const std::vector<row> &vals) {
    std::vector<OperationResult> opRes;
    for (auto x: vals) {
        opRes.push_back(put(x.first, x.second));
    }
    return opRes;
}

std::pair<std::vector<OperationResult>, std::vector<Database::row>>
Database::get(const std::vector<std::string> &keys) const {
    std::vector<OperationResult> opRes;
    std::vector<row> res;
    res.reserve(keys.size());
    if (keys.size() == 1 && keys[0] == "*") {
        res.reserve(rows());
        for (auto row: storage) {
            opRes.push_back(OperationResult::SUCCESS);
            res.push_back(row);
        }
    } else {
        for (const auto &key: keys) {
            const auto it = storage.find(key);
            if (it == storage.end()) {
                opRes.push_back(OperationResult::NOT_FOUND);
            } else {
                opRes.push_back(OperationResult::SUCCESS);
                res.push_back(*it);
            }
        }
    }
    return {opRes, res};
}

std::pair<OperationResult, Database::row> Database::get(const std::string &k) const {
    const auto it = storage.find(k);
    if (it == storage.end())
        return {OperationResult::NOT_FOUND, row()};
    return {OperationResult::SUCCESS, *it};
}

std::vector<Database::row> Database::get_where(const Where &where) const {
    std::vector<row> result;
    if (where.lhs == "key") {
        auto start = storage.begin();
        auto end = storage.end();
        std::string argument = where.rhs;
        auto it = storage.lower_bound(argument);
        if (where.op == ">") {
            if (storage.find(argument) == storage.end())
                start = it;
            else
                start = ++it;
            end = storage.end();
        } else if (where.op == ">=") {
            start = it;
            end = storage.end();
        } else if (where.op == "<") {
            start = storage.begin();
            end = it;
        } else if (where.op == "<=") {
            start = storage.begin();
            end = storage.upper_bound(argument);
        } else if (where.op == "==") {
            result.push_back(get(where.rhs).second);
            return result;
        }
        auto oper = where.op;
        std::for_each(start, end, [oper, argument, &result](const row &x) {
            if (oper == "!=" && x.first == argument) {
            } else
                result.push_back(x);
        });
    } else if (std::ranges::find(fields_, where.lhs) != fields_.end()) {
        for (const auto &x: storage) {
          auto& json_val = x.second[where.lhs];
          if (json_val.is_number()) {
            double val_lhs = json_val.get<double>();
            double val_rhs = std::stod(where.rhs);

            if ((where.op == ">" && val_lhs > val_rhs) ||
              (where.op == ">=" && val_lhs >= val_rhs) ||
              (where.op == "<" && val_lhs < val_rhs) ||
              (where.op == "<=" && val_lhs <= val_rhs) ||
              (where.op == "==" && val_lhs == val_rhs) ||
              (where.op == "!=" && val_lhs != val_rhs))
              result.push_back(x);
          }
        }
    }
    return result;
}

OperationResult Database::remove(const std::string &key) {
    if (storage.find(key) == storage.end())
        return OperationResult::NOT_FOUND;
    storage.erase(key);
    --rows_;
    return OperationResult::SUCCESS;
}

std::vector<OperationResult> Database::remove(const std::vector<std::string> &keys) {
    std::vector<OperationResult> opRes;
    for (const auto &x: keys) {
        opRes.push_back(remove(x));
    }
    return opRes;
}

std::vector<OperationResult> Database::remove_where(const Where &where) {
    const std::vector<row> rows_to_delete = get_where(where);
    std::vector<std::string> keys_to_delete;
    keys_to_delete.reserve(rows_to_delete.size());
    for (const auto &row: rows_to_delete)
        keys_to_delete.push_back(row.first);
    return remove(keys_to_delete);
}

void Database::printKeys() {
    std::cout << storage.size() << std::endl;
    for (auto x: storage) {
        std::cout << x.first << " ";
    }
    std::cout << std::endl;
}
