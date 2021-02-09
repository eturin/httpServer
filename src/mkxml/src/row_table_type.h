#ifndef MKXML_ROW_TABLE_TYPE_H
#define MKXML_ROW_TABLE_TYPE_H

#include "Context.h"
#include <pqxx/pqxx>
#include <string>

struct RowTableType {
    RowTableType();
    RowTableType(pqxx::binarystring ref,
                 const char * name,
                 const char * table_type,
                 const char * table,
                 const char * table_change,
                 const char * table_classifiers,
                 const char * table_attributes,
                 const char * table_yarls);
    RowTableType(const std::string & name, Context & cont);
    std::string name,
                table_type,
                table,
                table_change,
                field_code,
                field_name,
                table_classifiers,
                table_attributes,
                table_yarls;
    pqxx::binarystring ref;
    const std::string & get_field_name(Context & cont);
    const std::string & get_field_code(Context & cont);
};

#endif
