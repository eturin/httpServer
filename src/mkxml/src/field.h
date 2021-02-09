#ifndef MKXML_FIELD_H
#define MKXML_FIELD_H
#include <string>
struct Field {
    std::string table,
                name,
                vid,
                descr,
                table_1c,
                field,
                field_type,
                field_vid,
                field_str,
                field_num,
                field_bool,
                field_date,
                field_ref;
};
#endif
