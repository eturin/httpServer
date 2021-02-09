#include "db.h"
#include "Context.h"


DB * DB::prepare(const Context *cont) {
    for (const auto &x : cont->prepared_sql) {
        p_cn->prepare(x.first,x.second);
    }

    return this;
}
DB::DB(const Context *cont) : p_cn(new pqxx::connection(cont->connection_string)),
                              p_W(new pqxx::work(*p_cn)) {
    prepare(cont);
}
DB::~DB() {
    if (p_W) {
        commit();
        delete p_W;
        delete p_cn;
    }
}
void DB::commit() {
    p_W->commit();
    delete p_W;
    p_W = new pqxx::work(*p_cn);
}
