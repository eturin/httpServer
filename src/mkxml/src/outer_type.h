#ifndef MKXML_OUTER_TYPE_H
#define MKXML_OUTER_TYPE_H

#include "row_table_type.h"
#include "Context.h"
#include <pqxx/pqxx>
#include <string>

struct OuterType: public RowTableType {
    OuterType(const std::string &id, Context & cont);
    std::string outer_code,
                outer_name,
                system_code;
    std::string system_ref,
                outer_ref;
};

#endif
