#pragma once

#include <iostream>
#include <set>
#include <string>
#include "database.h"

class Parser {
public:
    Parser() : input{std::cin}, output(std::cout) { start_time = std::chrono::high_resolution_clock::now(); };
    Parser(std::istream &inp, std::ostream &out, std::ostream &err) : input{inp}, output{out} {
        std::chrono::high_resolution_clock::now();
    }
    std::istream &parse_line();
    static Database::row parse_row(const std::string &r);
    static Database::row parse_row_from_file(const std::string &r);

    void print_separator(const std::vector<size_t> &widths) const;
    void print_result_table(const std::vector<Database::row> &rows) const;
    void print_row(const Database::row &r) const;
    bool print_failures(const std::vector<std::string> &keys, const std::vector<OperationResult> &results) const;
    void backup() const;
    void timed_backup() const;
    bool load(std::string &filename) const;
    ~Parser();
    std::vector<Database *> databases_;
    bool run = true;

private:
    static inline const std::set<std::string> commands = {"PUT",  "PUT_FILE", "GET",    "DEL",  "OPEN", "CREATE",
                                                          "LIST", "QUIT",     "BACKUP", "LOAD", "HELP"};
    static inline const std::chrono::milliseconds backup_interval{300'000};
    std::istream &input;
    std::ostream &output;
    Database *db = nullptr;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
};
