#ifndef MKXML_CONTEXT_H
#define MKXML_CONTEXT_H

#define REF_SIZE 16
#define LIDS "LIDS"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
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
                outer_name,
                system_code;
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
    std::vector<std::pair<pqxx::binarystring, unsigned> > get_changes() const;
    void send(const std::ostringstream &sout,
              const std::vector<std::pair<pqxx::binarystring, unsigned> > &vref,
              const std::string &message_date,
              const pqxx::binarystring &message_ref,
              const std::string &message_id,
              bool isSSL,
              bool isMultipart,
              const std::string &host,
              const std::string &port,
              const std::string &target,
              const std::string &user,
              const std::string &pass,
              const pqxx::binarystring &integ_ref,
              const pqxx::binarystring &node_ref,
              bool dontSend) const;
    std::size_t mkXMLs(bool isSSL,
                        bool isMultipart,
                        const std::string &host,
                        const std::string &port,
                        const std::string &target,
                        const std::string &user,
                        const std::string &pass,
                        const pqxx::binarystring &integ_ref,
                        const pqxx::binarystring &node_ref,
                        bool dontSend) const;
    void item(const pqxx::binarystring &ref, std::ostringstream &sout) const;
    std::map<std::string, std::vector<Field> > tables;
    std::map<std::string, std::string>         restricted_attrs;
    Context & cont;
};
#endif //MKXML_CONTEXT_H
