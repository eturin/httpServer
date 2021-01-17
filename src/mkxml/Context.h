#ifndef MKXML_CONTEXT_H
#define MKXML_CONTEXT_H

#define REF_SIZE 16
#define LIDS "LIDS"

#include <string>
#include <vector>
#include <sstream>
#include <pqxx/pqxx>

struct RowTableType;
struct Context;

struct DB {
    DB(const std::string & str_connection, const Context &cont);
    ~DB();
    pqxx::connection *p_cn;
    pqxx::work       *p_W;
    void commit();
};


struct Context {
    Context();
    ~Context();

    DB & get_con();
    DB * activate_con(DB *);
    pqxx::binarystring from_hex(const std::string & str) const;

    std::map<std::string,std::string> prepared_sql;
    unsigned max_items;
    pqxx::binarystring outer_ref;
    std::vector<DB*> pool;
    std::string get_connection_string() const;
    std::map<std::string, RowTableType> mTablesType;
    std::map<std::string, std::string>  mTablesName;
    std::map<std::string, std::string>  mTablesNameShort;
};

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

struct OuterType: public RowTableType {
    OuterType(pqxx::binarystring id, Context & cont);
    std::string outer_code,
                outer_name;
    pqxx::binarystring system_ref,
                       outer_ref;
};

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

struct RefType: public OuterType {
    RefType(pqxx::binarystring outer_id, Context & cont);
    std::vector<pqxx::binarystring> get_changes() const;
    void send(const std::ostringstream &sout) const;
    void mkXMLs() const;
    void item(const pqxx::binarystring &ref, std::ostringstream &sout) const;
    std::map<std::string, std::vector<Field> > tables;
    std::map<std::string, std::string>         restricted_attrs;
    Context & cont;
};
#endif //MKXML_CONTEXT_H
