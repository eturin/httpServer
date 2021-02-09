#include <cstring>
#include <algorithm>
#include <iostream>
#include <string>

#include "Context.h"
#include "row_table_type.h"


Context::Context() {
    prepared_sql[LIDS] = "select t.КодВнешнегоСправочника from sТаблицаПереходныхКлючейid as t where t.ЗначениеСправочникаНСИ_Тип=E'\\\\x08' and t.ЗначениеСправочникаНСИ_Вид=$1 and t.ЗначениеСправочникаНСИ = $2 and t.ВнешнийСправочник = $3";
    connection_string = get_connection_string();

    DB &db=get_con();
    pqxx::result rs=db.p_W->exec("select * from tables_type");

    for (const auto &row : rs) {
        mTablesType[row["id"].c_str()] = {pqxx::to_string(row["id"]),
                                          row["Описание"].c_str(),
                                          row["type"].c_str(),
                                          row["Таблица"].c_str(),
                                          row["Изменения"].c_str(),
                                          row["Классификация"].c_str(),
                                          row["Атрибуты"].c_str(),
                                          row["Ярлыки"].c_str()};
        mTablesName[row["Описание"].c_str()]     = row["id"].c_str();
        mTablesNameShort[row["Таблица"].c_str()] = row["id"].c_str();
    }

    for (auto &x : mTablesType)
        if (x.second.table_type!="") {
            std::string sql("select 1 from ");
            sql.append(x.second.table_type + " as t where t.Имя = 'НаименованиеПолное' limit 1");
            pqxx::result rs = db.p_W->exec(sql);
            x.second.field_name = rs.affected_rows() ? "НаименованиеПолное" : "Наименование";
        }


    const char *maxItems = std::getenv("MAX_ITEMS");
    max_items = maxItems ? std::atoi(maxItems) : 5000;

}
Context::~Context() {
    for (auto x : pool) delete x;
    pool.clear();
}
std::string Context::get_connection_string() const {
    const char *db_host = std::getenv("DB_HOST"),
               *db_port = std::getenv("DB_PORT"),
               *db_name = std::getenv("DB_NAME"),
               *db_user = std::getenv("DB_USER"),
               *db_user_pass = std::getenv("DB_USER_PASS");
    std::string con_str;
    con_str.append("dbname    = ").append(db_name      ? db_name : static_cast<const char *>("NSI")            )
           .append(" user     = ").append(db_user      ? db_user : static_cast<const char *>("user1c")         )
           .append(" password = ").append(db_user_pass ? db_user_pass : static_cast<const char *>("sGLaVj4PUw"))
           .append(" hostaddr = ").append(db_host      ? db_host : static_cast<const char *>("192.168.52.80")  )
           .append(" port     = ").append(db_port   ? db_port : static_cast<const char *>("5432")           );

    return con_str;
}
DB & Context::get_con() {
    for (auto & p_db : pool)
        return *activate_con(p_db);


    pool.push_back(new DB(this));
    return *activate_con(pool.front());
}
DB * Context::activate_con(DB * p_db) {
    if (!p_db->p_cn->is_open()) {
        delete p_db->p_cn;
        p_db->p_cn = new pqxx::connection(get_connection_string());
        if (!p_db->p_W)
            p_db->p_W = new pqxx::work(*p_db->p_cn);

        p_db->prepare(this);
    }
    return p_db;
}
std::basic_string<std::byte> Context::from_hex(const std::string &str_hex) const {

    int val;
    bool k = false;
    std::basic_string<std::byte> r;
    for (char c : str_hex) {
        val *= k ? 16 : 0;
        val += '0'<=c && c<='9' ?
                        c-'0'
               : 'A'<=c && c<='Z' ?
                        10+c-'A'
               :
                        10+c-'a';
        if (k) r.push_back(std::byte(val));

        k = !k;
    }

    return r;
}

