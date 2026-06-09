#include "parser.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include "database.h"
#include "where.h"
using namespace std;


Parser::~Parser() {
    for (const auto x: databases_)
        delete x;
    databases_.clear();
    db = nullptr;
}

istream &Parser::parse_line() {
    timed_backup();
    string line;
    output << ">>> " << flush;
    if (db != nullptr)
        output << "(" << db->name << ") " << flush;
    getline(input, line);
    stringstream res(line);
    string command, argument;
    Where where_statement;
    res >> command >> argument;
    if (!commands.contains(command)) {
        output << "This is not a valid command.\n" << flush;
        return input;
    }
    if (command == "OPEN") {
        auto it = ranges::find_if(databases_, [argument](const Database *x) { return x->name == argument; });
        if (it == databases_.end()) {
            input.clear();
            return input;
        }
        db = *it;
    } else if (command == "CREATE") {
        string field;
        vector<string> fields;
        if (argument.empty()) {
            output << "You have to specify the name and fields of the database you want to create.\n";
            return input;
        }
        while (res >> field) {
            fields.push_back(field);
        }
        for (const auto &d: databases_) {
            if (d->name == argument) {
                output << "A database with this name already exists.\n";
                return input;
            }
        }
        databases_.push_back(new Database(argument, fields));
        output << "Database " << argument << " created.\n";
    } else if (command == "PUT") {
        if (db == nullptr) {
            output << "No database opened.\n";
            return input;
        }
        vector<Database::row> to_put;
        vector<string> keys;
        string to_put_elem;
        while (res >> to_put_elem) {
            try {
                Database::row to_put_row = parse_row(to_put_elem);
                to_put.push_back(to_put_row);
                keys.push_back(to_put_row.first);
            } catch (exception &e) {
                output << e.what() << "\n";
                return input;
            }
        }
        vector<OperationResult> result;
        if (!to_put.empty()) {
            try {
                keys.push_back(parse_row(argument).first);
                to_put.push_back(parse_row(argument));
                result = db->put(to_put);
            } catch (exception &e) {
                output << e.what() << "\n";
                return input;
            }
        } else {
            try {
                auto [key, value] = parse_row(argument);
                keys.push_back(key);
                result.push_back(db->put(key, value));
            } catch (exception &e) {
                output << e.what() << "\n";
                return input;
            }
        }
        if (!print_failures(keys, result))
            output << "Inserted successfully.\n";
    } else if (command == "PUT_FILE") {
        if (db == nullptr) {
            output << "No database opened.\n";
            input.clear();
            return input;
        }
        vector<Database::row> to_put;
        vector<string> keys;
        string to_put_elem;
        while (res >> to_put_elem) {
            try {
                Database::row to_put_row = parse_row_from_file(to_put_elem);
                to_put.push_back(to_put_row);
                keys.push_back(to_put_row.first);
            } catch (exception &e) {
                output << e.what() << "\n";
                input.clear();
                return input;
            }
        }
        keys.push_back(argument);
        vector<OperationResult> result;
        if (!to_put.empty()) {
            try {
                auto parsed_row = parse_row_from_file(argument);
                to_put.emplace_back(std::move(parsed_row));
                result = db->put(to_put);
            } catch (exception &e) {
                output << e.what() << "\n";
                return input;
            }
        } else {
            try {
                auto [key, value] = parse_row_from_file(argument);
                result.push_back(db->put(key, value));
            } catch (exception &e) {
                output << e.what() << "\n";
                return input;
            }
        }
        if (!print_failures(keys, result))
            output << "Inserted successfully.\n";

    } else if (command == "GET") {
        if (db == nullptr) {
            output << "No database opened.\n";
            return input;
        }
        if (argument == "WHERE") {
            string where_arg_fragment;
            string where_arg;
            while (res >> where_arg_fragment)
                where_arg += " " + where_arg_fragment;
            try {
                where_statement = Where(where_arg);
            } catch (invalid_statement &e) {
                output << e.what();
                return input;
            }
        }
        vector<Database::row> result;
        vector<OperationResult> status;
        vector<string> keys;
        if (!where_statement.empty()) {
            result = db->get_where(where_statement);
        } else {
            string key;
            keys.push_back(argument);
            while (res >> key)
                keys.push_back(key);
            if (keys.size() == 1) {
                if (argument == "*") {
                    print_result_table(db->get(vector<string>{"*"}).second);
                } else if (db->get(argument).first == OperationResult::SUCCESS) {
                    print_result_table({db->get(argument).second});
                }
                status.push_back(db->get(argument).first);
            } else
                tie(status, result) = db->get(keys);
        }
        if (!result.empty()) {
            print_result_table(result);
        }
        print_failures(keys, status);
    } else if (command == "DEL") {
        if (db == nullptr) {
            output << "No database opened.\n";
            return input;
        }
        if (argument == "WHERE") {
            string where_arg_fragment;
            string where_arg;
            while (res >> where_arg_fragment)
                where_arg += " " + where_arg_fragment;
            try {
                where_statement = Where(where_arg);
            } catch (invalid_statement &e) {
                output << e.what();
                return input;
            }
        }
        vector<OperationResult> status;
        vector<string> keys;
        if (!where_statement.empty())
            db->remove_where(where_statement);
        else {
            string key;
            while (res >> key)
                keys.push_back(key);
            keys.push_back(argument);
            if (keys.size() == 1) {
                status.push_back(db->remove(argument));
            } else
                status = db->remove(keys);
        }
        print_failures(keys, status);
    } else if (command == "LIST") {
        output << "Databases: \n";
        ranges::for_each(databases_, [this](auto x) { output << "\t" << x->name << "\n"; });
    } else if (command == "BACKUP") {
        if (db == nullptr) {
            output << "No database opened.\n";
            return input;
        }
        try {
            backup();
        } catch (exception &e) {
            output << e.what() << "\n";
            return input;
        }
        output << "Backed up successfully.\n";
    } else if (command == "LOAD") {
        if (db == nullptr) {
            output << "No database opened.\n";
            return input;
        }
        try {
            if (load(argument))
                output << "Loaded backup " + argument + " successfully\n";
            else
                output << "Error loading backup\n";
        } catch (exception &e) {
            output << e.what() << "\n";
            return input;
        }
    } else if (command == "QUIT") {
        run = false;
    }
    return input;
}

class invalid_row : std::exception {
public:
    const char *what() const noexcept override {
        return "Invalid argument(s) of PUT; the arguments should be a space-separated sequence in the format "
               "\"[KEY];[VALUE]\", where [VALUE] is either a string representing a JSON or a filename of a file "
               "containing a JSON.\n";
    }
};

Database::row Parser::parse_row(const std::string &r) {
    if (!r.contains(";")) {
        throw invalid_row();
    }
    stringstream ss(r);
    string key, value;
    getline(ss, key, ';');
    getline(ss, value);
    try {
        nlohmann::json value_json = nlohmann::json::parse(value);
        return Database::row(key, value_json);
    } catch (std::exception &) {
        throw;
    }
}

Database::row Parser::parse_row_from_file(const std::string &r) {
    if (!r.contains(";")) {
        throw invalid_row();
    }
    stringstream ss(r);
    string key, filepath_string;
    getline(ss, key, ';');
    getline(ss, filepath_string);
    filesystem::path json_path(filepath_string);
    try {
        std::ifstream fs(filepath_string);
        if (!fs.is_open()) {
            throw ios_base::failure("The file containing the JSON did not open");
        }
        nlohmann::json value_json = nlohmann::json::parse(fs);
        return Database::row(key, value_json);
    } catch (std::exception &) {
        throw;
    }
}

void Parser::print_row(const Database::row &r) const { output << "| " << r.first << " | " << r.second << " |\n"; }

bool Parser::print_failures(const std::vector<std::string> &keys, const std::vector<OperationResult> &results) const {
    vector<string> failures;
    for (int i = 0; i < keys.size(); ++i) {
        if (results[i] != OperationResult::SUCCESS && keys[i] != "*")
            failures.push_back(keys[i]);
    }
    if (failures.empty())
        return false;
    if (!keys.empty()) {
        output << "Failed for keys: ";
        for (auto key: failures)
            output << key << " ";
        output << "\n";
    }
    return true;
}

void Parser::print_separator(const vector<size_t> &widths) const {
    for (size_t w: widths)
        output << string(w + 2, '-');
    output << "\n";
}

void Parser::print_result_table(const vector<Database::row> &rows) const {
    if (rows.empty())
        return;

    const vector<string> &fields = db->fields();

    vector<size_t> widths;
    for (const auto &f: fields)
        widths.push_back(f.size());
    widths.push_back(string("key").size());
    for (const auto &[key, json]: rows) {
        widths[0] = max(widths[0], key.size());
        for (size_t i = 0; i < fields.size(); ++i) {
            string val = json.contains(fields[i]) ? (json[fields[i]].is_string() ? json[fields[i]].get<string>()
                                                                                 : json[fields[i]].dump())
                                                  : "";
            widths[i + 1] = max(widths[i + 1], val.size());
        }
    }
    output << left << setw(widths[0] + 2) << "key";
    for (size_t i = 0; i < fields.size(); ++i)
        output << left << setw(widths[i + 1] + 2) << fields[i];
    output << "\n";

    print_separator(widths);

    for (const auto &[key, json]: rows) {
        output << left << setw(widths[0] + 2) << key;
        for (size_t i = 0; i < fields.size(); ++i) {
            string val = json.contains(fields[i]) ? (json[fields[i]].is_string() ? json[fields[i]].get<string>()
                                                                                 : json[fields[i]].dump())
                                                  : "";
            output << left << setw(widths[i + 1] + 2) << val;
        }

        output << "\n";
    }

    print_separator(widths);
    output << "(" << rows.size() << (rows.size() == 1 ? " row)\n" : " rows)\n");
}

void Parser::backup() const {
    ofstream file;
    const auto now = std::chrono::system_clock::now();
    auto now_sec = std::chrono::floor<std::chrono::seconds>(now);
    std::string filename = "backups/forced_" + std::format("{:%Y-%m-%d_%H-%M-%S}", now_sec) + ".bckp";
    file.open(filename, ios::out | ios::trunc);
    if (!file.is_open()) {
        throw ios_base::failure("Backup failed; the file did not open properly");
    }
    for (const auto &row: db->get(vector<string>{"*"}).second) {
        file << row.first << ";" << row.second.dump() << " ";
    }
    file.close();
}

bool Parser::load(std::string &filename) const {
    ifstream file;
    filename = "backups/" + filename;
    file.open(filename, ios::in);
    output << filename << endl;
    if (!file.is_open()) {
        throw ios_base::failure(
                "Load failed; the file did not open properly (check if the provided filename is correct)");
    }
    string row_str;
    vector<Database::row> to_insert;
    vector<string> keys;
    while (file >> row_str) {
        try {
            Database::row parsed_row = parse_row(row_str);
            keys.push_back(parsed_row.first);
            to_insert.push_back(parsed_row);
        } catch (exception &) {
            throw;
        }
    }
    auto result = db->put(to_insert);
    if (print_failures(keys, result))
        return false;
    return true;
}

void Parser::timed_backup() const {
    const auto end_time = chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<chrono::milliseconds>(end_time - start_time);
    if (duration < backup_interval || db == nullptr)
        return;
    ofstream file;
    const string filename = "backups/regular_backup_" + db->name + ".bckp";
    file.open(filename, ios::trunc);
    if (!file.is_open())
        throw ios_base::failure("Timed backup failed; error opening the \"regular_backup.bckp\" file");
    for (const auto &row: db->get(vector<string>{"*"}).second) {
        file << row.first << ";" << row.second.dump() << " ";
    }
    file.close();
}
