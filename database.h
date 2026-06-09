#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <tlx/container/btree_map.hpp>
#include <utility>
#include "where.h"

enum class OperationResult { SUCCESS, NOT_FOUND, INVALID_FIELDS, OVERWRITE, ALREADY_EXISTS };

class Database {
public:
    using row = std::pair<std::string, nlohmann::json>;

    const std::string name;
    // constructors
    Database() = default;
    explicit Database(std::string n, const std::vector<std::string> &f) :
        name{std::move(n)}, cols_{f.size()}, fields_{f} {
        std::ranges::sort(fields_);
    }

    // insertion
    OperationResult put(const std::string &k, const nlohmann::json &val);
    std::vector<OperationResult> put(const std::vector<row> &vals);

    // retrieval
    [[nodiscard]] std::pair<OperationResult, row> get(const std::string &k) const;
    [[nodiscard]] std::pair<std::vector<OperationResult>, std::vector<row>>
    get(const std::vector<std::string> &keys) const;

    // retrieval with a condition
    [[nodiscard]] std::vector<row> get_where(const Where &where) const;

    // deletion
    OperationResult remove(const std::string &key);
    std::vector<OperationResult> remove(const std::vector<std::string> &keys);

    // deletion with a condition
    std::vector<OperationResult> remove_where(const Where &where);

    // getters
    [[nodiscard]] size_t rows() const { return rows_; }
    [[nodiscard]] size_t columns() const { return cols_; }
    [[nodiscard]] std::vector<std::string> fields() const { return fields_; }

    // debug
    void printKeys();

private:
    tlx::btree_map<std::string, nlohmann::json> storage;
    size_t rows_{};
    size_t cols_{};
    std::vector<std::string> fields_;
};
