#ifndef MKXML_DB_H
#define MKXML_DB_H


#include <string>
#include <pqxx/pqxx>

struct Context;

struct DB {
    DB(const Context *cont);
    ~DB();
    pqxx::connection *p_cn;
    pqxx::work       *p_W;
    void commit();
    DB * prepare(const Context *cont);
};

#endif
