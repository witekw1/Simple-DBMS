#include <iostream>
#include "parser.h"
using namespace std;

int main() {
    Parser p;
    while (p.run) {
        p.parse_line();
    }
    return 0;
}
