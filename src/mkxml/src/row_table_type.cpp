#include "row_table_type.h"

RowTableType::RowTableType(const std::string &ref,
                           const char *name,
                           const char *table_type,
                           const char *table,
                           const char *table_change,
                           const char * table_classifiers,
                           const char * table_attributes,
                           const char * table_yarls) :name(name),
                                                      table_type(table_type),
                                                      table(table),
                                                      table_change(table_change),
                                                      table_classifiers(table_classifiers),
                                                      table_attributes(table_attributes),
                                                      table_yarls(table_yarls),
                                                      ref(ref) {}
RowTableType::RowTableType():name("") {}
RowTableType::RowTableType(const std::string &name, Context & cont) : name(name){
    RowTableType &tmp = cont.mTablesType[cont.mTablesName[name]];
    table_type  = tmp.table_type;
    table       = tmp.table;
    table_change= tmp.table_change;
    field_name  = tmp.field_name;
    table_classifiers=tmp.table_classifiers;
    table_attributes =tmp.table_attributes;
    table_yarls      =tmp.table_yarls;
    ref         = tmp.ref;
}
const std::string & RowTableType::get_field_name(Context &cont) {
    if (field_name == "") {
        DB &db=cont.get_con();
        std::string sql("select 1 from ");
        sql.append(table_type + " as t where t.Имя = 'НаименованиеПолное' limit 1");
        pqxx::result rs = db.p_W->exec(sql);
        field_name = rs.affected_rows() ? "НаименованиеПолное" : "Наименование";
    }

    return field_name;
}
const std::string & RowTableType::get_field_code(Context &cont) {
    if (field_code == "") {
        DB &db=cont.get_con();
        std::string sql("select 1 from ");
        sql.append(table_type + " as t where t.Имя = 'Код' limit 1");
        pqxx::result rs = db.p_W->exec(sql);
        field_code = rs.affected_rows() ? "Код" : "Наименование";
    }

    return field_code;
}

