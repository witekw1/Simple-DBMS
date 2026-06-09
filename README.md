# Simple-DBMS
[![C++ Version](https://img.shields.io/badge/C++-23+-darkblue.svg)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.15+-violet.svg)](https://cmake.org/)

This is an educational project implementing a simple single-threaded in-memory NoSQL database managemenet system. The system supports databases made up of key-value pairs, where the key is a string and the value is a JSON object.

## Usage
Use the `SDBMS` command in your terminal to launch the CLI. 

The CLI supports the following commands:
 * `CREATE [NAME] [FIELDS]` - creates a database with the name `[NAME]` and the fields (column names) `[FIELDS]` (none of the fields can be "key"). The fields should be space-separated, and each database has to have a unique name.
 * `LIST`- lists all the created databases.
 * `OPEN [NAME]` - open the database with the name `[NAME]`.
 * `PUT [KEY1];[VALUE1] [KEY2];[VALUE2] ...` - inserts the specified key-value pairs into the currently open database. Note that the `[VALUE]` field has to be a valid JSON and can't contain whitespaces. The keys at the first indentation level of the JSON have to be the same as the field names of the database.
 * `PUT_FILE [KEY1];[FILENAME1] [KEY2];[FILENAME2]` - same as `PUT` but with the names of files containing the JSON objects (the files can contain whitespaces). `FILENAME` has to be the absolute path to the file.
 * `GET [KEY1] [KEY2] ...` - displays the data stored in the currently open database under the specified keys. You can also use "*" to get all of the available data.
 * `GET WHERE [CONDITION]` - displays the data satisfying the `[CONDITION]`. The condition has to be in the format: `[FIELD] [OPERATOR] [VALUE]`. `[FIELD]` has to be one of the fields of the currently open database or `key`, in which case the the command will display the data where the key satisfies the condition. `[OPERATOR]` has to be one of the following binary operators: `<, >, ==, !=, <=, >= `. `[VALUE]` has to be a numeric value.
 * `DEL [KEY1] [KEY2] ...` - deletes the data stored in the currently open database under the specified keys.
 * `DEL WHERE [CONDITION]` - deletes the data stored in the currently open database that satisfies the condition. What is a condition is specified in the description of the `GET WHERE` command.
 * `BACKUP` - dumps the contents of the currently open database into a file stored in the `backups/` directory. The name of the backup file is in the format `forced_[YYYY-MM-DD]_[HH-mm-ss].bckp`. Note that the file may not appear in the filesystem until the CLI is closed.
 * `LOAD [FILENAME]` - load the backup with the specified filename. The file has to exist in the `backups/` directory.
 * `QUIT` - close the CLI.
 * `HELP` - display this information.
   
> [!NOTE]
> Additionally, the engine will automatically create a backup of the currently open database every 5 minutes (checked when running a command), and save it in the backup folder under the name `"regular_backup_[NAME].bckp"` where `[NAME]` is the name of the currently open database.

### Some usage examples

```
// Creating and opening a database:
CREATE users age city
OPEN users

// Inserting data
PUT user1;{"age":25,"city":"Krakow"} user2;{"age":30,"city":"Warsaw"}
PUT_FILE user3;/home/user/data1.json

// Retrieving data
GET user1 user3
GET *
GET WHERE age > 28

// Deleting data
DEL WHERE city == Warsaw

// Creating a backup
BACKUP
QUIT
```

## Installation
### Dependencies
 - C++ compiler supporting C++23
 - CMake (version >= 3.15)
 - Git
### Linux/MacOS 

Clone the repository and run the installation script:
   ```
   git clone git@github.com:witekw1/Simple-DBMS.git
   cd Simple-DBMS
   ./install.sh
   ```
   
To uninstall the project, simply run:
```
cat build/install_manifest.txt | xargs sudo rm
``` 
### Windows
WIP




