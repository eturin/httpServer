#include <iostream>
#include "Context.h"


int main() {
    Context cont;
    std::string str_outer_ref;
    while (std::cin >> str_outer_ref) {
        cont.outer_ref = cont.from_hex(str_outer_ref);
        RefType rt = RefType(cont.outer_ref, cont);
        rt.mkXMLs();
        int k=1;
    }

    return 0;
}
