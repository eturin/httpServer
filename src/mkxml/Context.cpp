#include <cstring>
#include <algorithm>
#include <iostream>

#include "Context.h"


Context::Context():outer_ref(pqxx::binarystring("")) {
    prepared_sql[LIDS]           = "select t.КодВнешнегоСправочника from sТаблицаПереходныхКлючейid as t where t.ЗначениеСправочникаНСИ_Тип=E'\\\\x08' and t.ЗначениеСправочникаНСИ_Вид=$1 and t.ЗначениеСправочникаНСИ = $2 and t.ВнешнийСправочник = $3";

    DB &db=get_con();
    pqxx::result rs=db.p_W->exec("select * from tables_type");

    for (const auto &row : rs) {
        mTablesType[row["id"].c_str()] = {pqxx::binarystring(row["id"]),
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

    for (auto &x :mTablesType)
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
DB * prepare(DB * p_db, const Context &cont) {
    for (const auto &x : cont.prepared_sql) {
        //p_db->p_cn->unprepare(x.first);
        p_db->p_cn->prepare(x.first,x.second);
    }

    return p_db;
}
DB::DB(const std::string &str_connection, const Context &cont) : p_cn(new pqxx::connection(str_connection)), p_W(new pqxx::work(*p_cn)) {
    prepare(this,cont);
}
DB::~DB() {
    if (p_W) {
        commit();
        delete p_W, p_cn;
    }
}
void DB::commit() {
    p_W->commit();
    delete p_W;
    p_W = new pqxx::work(*p_cn);
}
DB & Context::get_con() {
    for (auto & p_db : pool) {
        return *activate_con(p_db);
    }

    pool.push_back(new DB(get_connection_string(),*this));
    return *activate_con(pool.front());
}
DB * Context::activate_con(DB * p_db) {
    if (!p_db->p_cn->is_open()) {
        delete p_db->p_cn;
        p_db->p_cn = new pqxx::connection(get_connection_string());
        if (!p_db->p_W)
            p_db->p_W = new pqxx::work(*p_db->p_cn);

        prepare(p_db,*this);
    }
    return p_db;
}
pqxx::binarystring Context::from_hex(const std::string &str_hex) const {
    //80D1005056AB252711E8AD30EE8F0B54
    unsigned char ref[REF_SIZE],
                  *p_ref=ref;
    bool k= false;
    for (char c : str_hex) {
        *p_ref *= k ? 16 : 0;
        *p_ref += '0'<=c && c<='9' ? c-'0' : 10+c-'A';
        p_ref+=k;
        k = !k;
    }

    return pqxx::binarystring(ref,REF_SIZE);
}
RowTableType::RowTableType(pqxx::binarystring ref,
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
RowTableType::RowTableType():ref(pqxx::binarystring("")),name("") {}
RowTableType::RowTableType(const std::string &name, Context & cont) : ref(pqxx::binarystring("")),
                                                                      name(name){
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
OuterType::OuterType(pqxx::binarystring outer_ref, Context & cont) : RowTableType(),
                                                                     outer_ref(outer_ref),
                                                                     system_ref(pqxx::binarystring("")){
    DB &db=cont.get_con();
    RowTableType rtt_idobj("rИдентификаторыОбъектовМетаданныхid",cont);
    if (cont.prepared_sql.find("ВнешнийСправочник") == cont.prepared_sql.end()) {
        cont.prepared_sql["ВнешнийСправочник"] = std::string(R"(
        select
            t.Код,
            t.Наименование,
            'r'||i.Имя||'id' as Имя,
            t.Владелец
        from rВнешниеСправочникиid as t
             join )") + rtt_idobj.table + R"( as i on(i.Ссылка = t.ИдентификаторСправочникаНСИ)
        where t. Ссылка = $1
        limit 1)";
        db.p_cn->prepare("ВнешнийСправочник", cont.prepared_sql["ВнешнийСправочник"]);
    }

    pqxx::result rs = db.p_W->exec_prepared("ВнешнийСправочник",outer_ref);
    outer_code  = rs[0]["Код"].c_str();
    outer_name  = rs[0]["Наименование"].c_str(),
    name        = rs[0]["Имя"].c_str();
    system_ref  = pqxx::binarystring(rs[0]["Владелец"]);

    RowTableType rtt = RowTableType(name,cont);
    table_type  = rtt.table_type;
    table       = rtt.table;
    table_change= rtt.table_change;
    field_name  = rtt.field_name;
    table_classifiers=rtt.table_classifiers;
    table_attributes =rtt.table_attributes;
    table_yarls      =rtt.table_yarls;
    ref         = rtt.ref;

}
RefType::RefType(pqxx::binarystring outer_ref, Context &cont): OuterType(outer_ref,cont), cont(cont) {
    DB &db=cont.get_con();
    std::vector<std::string> restrict;

    if (cont.prepared_sql.find("restrict") == cont.prepared_sql.end()) {
        pqxx::result rs = db.p_W->exec("select t.Таблица from rВнешниеСправочники_type as t where t.ИсходнаяТаблица = 'ОграничениеСостава' limit 1");

        std::string sql = std::string(R"(
        select
            case
                when t.Поле_Тип = E'\\x05' then t.Поле_Стр
                else                            h.Наименование
            end as Реквизит,

            case
                when t.Поле_Тип =E'\\x05' then 0
                else                           1
            end as Тип,

            t.Поле
        from )") + rs[0][0].c_str() + R"( as t
             left join hАтрибутыid           as h on(t.Поле_Тип = E'\\x08'
                                                     and t.Поле_Вид=E'\\x00000142'
                                                     and t.Поле = h.Ссылка)
        where t.Ссылка = $1
        )";
        cont.prepared_sql["restrict"] = sql;
        db.p_cn->prepare("restrict", sql);
    }
    pqxx::result rs = db.p_W->exec_prepared("restrict",outer_ref);
    for (const auto &row : rs)
        if (row[1].as<int>() == 0)
            restrict.push_back(row[0].c_str());
        else
            restricted_attrs[ row[2].c_str() ] = row[0].c_str();
    restrict.push_back("Ссылка");
    restrict.push_back("Version");

    std::string sql = std::string("select * from ").append(table_type).append(" as t order by 1");
    rs = db.p_W->exec(sql);
    for (const auto &row : rs) {
        std::string table1c(row["ИсходнаяТаблица"].c_str()),
                    field(row["Имя"].c_str()),
                    vid(row["Вид"].c_str());

        if ( "" == table1c && std::find(restrict.cbegin(),restrict.cend(),field) != restrict.cend()
             || std::find(restrict.cbegin(),restrict.cend(),table1c) != restrict.cend()
             || std::find(restrict.cbegin(),restrict.cend(),table1c + "." + field) != restrict.cend())
            continue;

        tables[row["ИсходнаяТаблица"].c_str()].push_back({row["Таблица"].c_str(),
                                                          field,
                                                          vid,
                                                          row["Описание"].c_str(),
                                                          row["ИсходнаяТаблица"].c_str(),
                                                          row["Поле"].c_str(),
                                                          row["Поле_Тип"].c_str(),
                                                          row["Поле_Вид"].c_str(),
                                                          row["Поле_Строка"].c_str(),
                                                          row["Поле_Число"].c_str(),
                                                          row["Поле_Булево"].c_str(),
                                                          row["Поле_Дата"].c_str(),
                                                          row["Поле_ref"].c_str()});
    }

}
std::vector<pqxx::binarystring> RefType::get_changes() const {
    DB &db=cont.get_con();
    if (cont.prepared_sql.find(name+"changes") == cont.prepared_sql.end()) {
        std::string sql("select t.Ссылка from nОбменНСИid as n join  ");
        sql.append(table_change).append(" as t on(t.node_Вид=E'\\\\x0000002E' and t.node=n.Ссылка) where n.ВнешнийСправочник = $1");
        cont.prepared_sql[name + "changes"] = sql;
        db.p_cn->prepare(name + "changes", sql);
    }
    pqxx::result rs = db.p_W->exec_prepared(name+"changes",outer_ref);
    std::vector<pqxx::binarystring> v;
    for (const auto &row : rs)
        v.push_back(pqxx::binarystring(row[0]));
    return v;
}
void RefType::send(const std::ostringstream &sout,
                   const std::vector<pqxx::binarystring> &vref,
                   const std::string &message_date,
                   const pqxx::binarystring &message_ref) const {
    DB &db=cont.get_con();

    for (const auto &x : vref)
        db.p_W->exec_prepared("sСеансыОбменаid",message_date,message_ref,outer_ref,ref,x);

    db.p_W->exec_prepared("sИсторияОбменаid",message_date,message_ref,sout.str(),'ok');
    db.commit();

}
void RefType::mkXMLs() const {
    unsigned cnt=0;

    DB &db=cont.get_con();
    if (cont.prepared_sql.find("спрСеансыОбмена") == cont.prepared_sql.end()) {
        std::string sql(R"(
        select pg_advisory_xact_lock(hashtext('insert_спрСеансыОбмена')::bigint);
        )");
        cont.prepared_sql["insert_спрСеансыОбмена"] = sql;
        db.p_cn->prepare("insert_спрСеансыОбмена", sql);

        sql = R"(
        insert into rСеансыОбменаid (ИмяПредопределенныхДанных             ,Ссылка                                                ,Владелец                              ,Код                                             ,Дата               ,ПометкаУдаления,Комментарий,ЭтоОшибка,СеансЗавершен,Направление                           ,ВнешнийСправочник)
                             values (E'\\x00000000000000000000000000000000',decode(replace(uuid_generate_v4()::text,'-',''),'hex'),E'\\x80d4005056ab252711e92084be4eff39',(select max(Код)+1 as Code from rСеансыОбменаid),now()::timestamp(0),false          ,''         ,false    ,false        ,E'\\xA0C2944C1269CD5445A9BA93230574B0',   $1            )
        returning Ссылка,Дата,Код;
        )";
        cont.prepared_sql["спрСеансыОбмена"] = sql;
        db.p_cn->prepare("спрСеансыОбмена", sql);

        sql = R"(
        insert into sСеансыОбменаid (Период,СеансОбмена,ВнешнийСправочник,СсылкаНаСправочник_Тип,СсылкаНаСправочник_Вид,СсылкаНаСправочник,Результат,Отправление,Принятие                    ,ОписаниеОшибки)
                             values ($1    , $2        , $3              ,E'\\x08'              , $4                   , $5               ,false    ,  $1       ,'0001-01-01 00:00:00.000000',''            )
        )";
        cont.prepared_sql["sСеансыОбменаid"] = sql;
        db.p_cn->prepare("sСеансыОбменаid", sql);

        sql = R"(
        insert into sИсторияОбменаid(Период,ИнтеграционнаяКомпонента              ,СеансОбмена,НаправлениеОбмена                     ,УИД                               ,ТекстЗапроса,ТекстОтвета,АдресВызова)
                              values($1    ,E'\\x80d4005056ab252711e92084be4eff39',$2         ,E'\\xA0C2944C1269CD5445A9BA93230574B0',uuid_generate_v4()::text::mvarchar,   $3       ,  $4       ,'mkXML'    )

        )";
        cont.prepared_sql["sИсторияОбменаid"] = sql;
        db.p_cn->prepare("sИсторияОбменаid", sql);

        sql = R"(
        select t.НомерВерсии , to_char(t.НачалоПериода, 'YYYY-MM-DD')
        from sВерсииОбъектовid as t
        where t.Объект_Тип = E'\\x08' and t.Объект_Вид = $1 and t.Объект = $2
              and t.НачалоПериода <= now() and (t.ОкончаниеПериода = '0001-01-01 00:00:00.000000' or now() <= t.ОкончаниеПериода);
        )";
        cont.prepared_sql["_sВерсииОбъектовid_"] = sql;
        db.p_cn->prepare("_sВерсииОбъектовid_", sql);
    }

    std::string message_id, message_date;
    pqxx::binarystring message_ref("");

    std::ostringstream sout;
    std::vector<pqxx::binarystring> vrefs;
    for (const auto &ref : get_changes()) {
        if (0 == cnt % cont.max_items) {
            if (cnt) {
                sout << "\t</Items>\n"
                     << "</Message>\n";
                send(sout,vrefs,message_date,message_ref);
                sout.clear();
                vrefs.clear();
            }
            db.p_W->exec_prepared("insert_спрСеансыОбмена");
            pqxx::result rs = db.p_W->exec_prepared("спрСеансыОбмена",outer_ref);
            message_id   = rs[0][2].c_str(),
            message_date = rs[0][1].c_str();
            message_ref  = pqxx::binarystring(rs[0][0]);
            db.commit();

            sout << "<?xml version='1.0' encoding='UTF-8'?>\n"
                 << "<Message xmlns='http://www.lmsoft.ru/mdm/exchange'\n"
                 << "         xmlns:xs='http://www.w3.org/2001/XMLSchema'\n"
                 << "         xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'>\n"
                 << "\t<MessageID>" << message_id << "</MessageID>\n"
                 << "\t<Items>\n";
        }
        vrefs.push_back(ref);
        item(ref, sout);
        ++cnt;
    }

    if (cnt) {
        sout << "\t</Items>\n"
             << "</Message>\n";
        send(sout,vrefs,message_date,message_ref);
        sout.clear();
        vrefs.clear();
    }
}
void field(const pqxx::row &row,
           const std::string &vid,
           const std::string &field,
           std::ostringstream &sout,
           Context &cont,
           const pqxx::binarystring &system_ref) {

    if (vid == "Строка")
        sout << "\t\t\t\t\t<Type>simple</Type>\n"
             << "\t\t\t\t\t<Value>\n"
             << "\t\t\t\t\t\t<valueString>"<< row[field].c_str() << "</valueString>\n"
             << "\t\t\t\t\t</Value>\n";
    else if (vid == "Число")
        sout << "\t\t\t\t\t<Type>simple</Type>\n"
             << "\t\t\t\t\t<Value>\n"
             << "\t\t\t\t\t\t<valueDecimal>"<< row[field].c_str() << "</valueDecimal>\n"
             << "\t\t\t\t\t</Value>\n";
    else if (vid == "Дата") {
        std::string val(row[field].c_str());
        val[10]='T';
        sout << "\t\t\t\t\t<Type>simple</Type>\n"
             << "\t\t\t\t\t<Value>\n"
             << "\t\t\t\t\t\t<valueDateTime>"<< val << "</valueDateTime>\n"
             << "\t\t\t\t\t</Value>\n";
    } else if (vid == "Булево")
        sout << "\t\t\t\t\t<Type>simple</Type>\n"
             << "\t\t\t\t\t<Value>\n"
             << "\t\t\t\t\t\t<valueBoolean>"<< (row[field].c_str()[0] =='f' ? "false":"true") << "</valueBoolean>\n"
             << "\t\t\t\t\t</Value>\n";
    else if (vid == "ХранилищеЗначения") {
        sout << "\t\t\t\t\t<Type>simple</Type>\n"
             << "\t\t\t\t\t<Value>\n"
             << "\t\t\t\t\t\t<valueString>"<< row[field].c_str() << "</valueString>\n"
             << "\t\t\t\t\t</Value>\n";
    } else {
        RowTableType &rtt = cont.mTablesType[cont.mTablesNameShort[vid]];
        DB &db = cont.get_con();
        if (cont.prepared_sql.find(rtt.table+"_short") == cont.prepared_sql.end()) {
            std::string sql("select t.");
            sql.append(rtt.get_field_code(cont)+" as Код, t.").append(rtt.get_field_name(cont) + " as Наименование from ").append(rtt.table).append(" as t where t.Ссылка = $1 limit 1");
            db.p_cn->prepare(rtt.table+"_short", sql);
            cont.prepared_sql[rtt.table+"_short"]=sql;
        }
        pqxx::result rs = db.p_W->exec_prepared(rtt.table+"_short",pqxx::binarystring(row[field]));
        std::string GID = rs.affected_rows() ? rs[0]["Код"].c_str() : "",
                    name= rs.affected_rows() ? rs[0]["Наименование"].c_str() : "";

        if (vid == "rЗначенияАтрибутовid") {
            sout << "\t\t\t\t\t<Type>enum</Type>\n"
                 << "\t\t\t\t\t<Value>\n"
                 << "\t\t\t\t\t\t<valueString>"<< name << "</valueString>\n"
                 << "\t\t\t\t\t</Value>\n"
                 << "\t\t\t\t\t<Name/>\n"
                 << "\t\t\t\t\t<TableName>ЗначенияАтрибутов</TableName>\n";
        }else {
            if (cont.prepared_sql.find("table_to_outer") == cont.prepared_sql.end()) {
                std::string sql(R"(
                SELECT
                        t.Ссылка,
                        t.Код,
                        t.Наименование
                FROM )");
                RowTableType rtt_idobj("rИдентификаторыОбъектовМетаданныхid", cont);
                sql.append(rtt_idobj.table).append(R"( as i
                    join rВнешниеСправочникиid as t on(i.Ссылка=t.ИдентификаторСправочникаНСИ
                                                       and t.Владелец = $1)
                where i.Имя = $2 )");

                db.p_cn->prepare("table_to_outer", sql);
                cont.prepared_sql["table_to_outer"] = sql;
            }
            sout << "\t\t\t\t\t<Type>reference</Type>\n"
                 << "\t\t\t\t\t<Value>\n"
                 << "\t\t\t\t\t\t<valueRef>\n";

            if (!GID.empty())
                sout << "\t\t\t\t\t\t\t<GID>" << GID << "</GID>\n";

            rs = db.p_W->exec_prepared("table_to_outer",system_ref,rtt.name.substr(1, rtt.name.size() - 3));
            if (rs.affected_rows()) {
                std::vector<std::string> lids;
                std::string outer_code = rs[0]["Код"].c_str(),
                        outer_name = rs[0]["Наименование"].c_str();


                pqxx::result rs2 = db.p_W->exec_prepared(LIDS,rtt.ref,pqxx::binarystring(row[field]),pqxx::binarystring(rs[0]["Ссылка"]));
                for (const auto &rl : rs2) // lids
                    lids.push_back(rl[0].c_str());


                sout << "\t\t\t\t\t\t\t<refLIDs>\n"
                     << "\t\t\t\t\t\t\t\t<ExtCatalog>\n"
                     << "\t\t\t\t\t\t\t\t\t<id>" << outer_code << "</id>\n"
                     << "\t\t\t\t\t\t\t\t\t<name>" << outer_name << "</name>\n";
                if (lids.size()) {
                    sout << "\t\t\t\t\t\t\t\t\t<LIDs>\n";
                    for (auto &x :lids)
                        sout << "\t\t\t\t\t\t\t\t\t\t<LID>" << x << "</LID>\n";
                    sout << "\t\t\t\t\t\t\t\t\t</LIDs>\n";
                } else if (!GID.empty())
                    sout << "\t\t\t\t\t\t\t\t\t\t<LIDs/>\n";

                sout << "\t\t\t\t\t\t\t\t</ExtCatalog>\n"
                     << "\t\t\t\t\t\t\t</refLIDs>\n";
            } else
                sout << "\t\t\t\t\t\t\t<tableName>" << rtt.name.substr(1, rtt.name.size() - 3) << "</tableName>\n"
                     << "\t\t\t\t\t\t\t<refLIDs/>\n";

            if (name.empty())
                sout << "\t\t\t\t\t\t\t<name/>\n";
            else
                sout << "\t\t\t\t\t\t\t<name>" << name << "</name>\n";

            sout << "\t\t\t\t\t\t</valueRef>\n"
                 << "\t\t\t\t\t</Value>\n";
        }
    }
}
void RefType::item(const pqxx::binarystring &item_ref, std::ostringstream &sout) const {
    DB &db=cont.get_con();
    // внешний спровочник
    std::vector<std::string> lids;
    {
        pqxx::result rs = db.p_W->exec_prepared(LIDS,ref,item_ref,outer_ref);
        for (const auto &rl : rs) // lids
            lids.push_back(rl[0].c_str());
    }

    if (cont.prepared_sql.find(name) == cont.prepared_sql.end()) {
        std::string sql("select t.* from ");
        sql.append(table).append(R"( as t where t.Ссылка = $1 and t.СтатусЗаписи = E'\\x80BF00505601131511E66F56BA579BBF' /*Эталон*/ limit 1 )");
        cont.prepared_sql[name] = sql;
        db.p_cn->prepare(name, sql);
    }

    pqxx::result rs = db.p_W->exec_prepared(name,item_ref);
    if (rs.affected_rows() == 0) return;

    // новый элемент
    sout << "\t\t<Item>\n"
         << "\t\t\t<GID>" << rs[0]["Код"].c_str() << "</GID>\n"
         << "\t\t\t<OuterCatalogs>\n"
         << "\t\t\t\t<OuterCatalog>\n"
         << "\t\t\t\t\t<id>" << outer_code <<"</id>\n"
         << "\t\t\t\t\t<LIDs>\n";
    for (auto &x : lids)
        sout << "\t\t\t\t\t\t<LID>" << x << "</LID>\n";
    sout << "\t\t\t\t\t</LIDs>\n"
         << "\t\t\t\t</OuterCatalog>\n"
         << "\t\t\t</OuterCatalogs>\n";

    // реквизиты
    for (const Field & x : tables.at("") ) {

        const std::string *p_vid = &x.vid,
                          *p_field = &x.field;
        std::string tmp;
        bool is_set=false;
        if (x.vid.find("Тип")!=std::string::npos) {
          if (rs[0][x.field_type].as<int>() == 5) {
              tmp = "Строка";
              p_field=&x.field_str;
              is_set = true;
          } else if (rs[0][x.field_type].as<int>() == 4) {
              tmp = "Дата";
              p_field=&x.field_date;
              is_set = true;
          } else if (rs[0][x.field_type].as<int>() == 3) {
              tmp = "Число";
              p_field=&x.field_num;
              is_set = true;
          } else if (rs[0][x.field_type].as<int>() == 2) {
              tmp = "Булево";
              p_field=&x.field_bool;
              is_set = true;
          }
          if (is_set) p_vid  = &tmp;
        } else if (x.vid.find("ХранилищеЗначения")!=std::string::npos) {
            tmp = "Строка";
            p_field=&x.field_str;
            is_set = true;
        }
        if (!is_set && x.vid.find("Вид")!=std::string::npos) {
            p_vid  =&cont.mTablesType[rs[0][x.field_vid].c_str()].table;
            p_field=&x.field_ref;
        }
        is_set = true;
        if (is_set) {
            sout << "\t\t\t<Property>\n"
                 << "\t\t\t\t<PropertyType>props</PropertyType>\n"
                 << "\t\t\t\t<PropertyID>" << x.name << "</PropertyID>\n"
                 << "\t\t\t\t<PropertyName>" << x.name << "</PropertyName>\n"
                 << "\t\t\t\t<PropertyValue>\n";

            field(rs[0], *p_vid, *p_field, sout, cont, system_ref);

            sout << "\t\t\t\t</PropertyValue>\n"
                 << "\t\t\t</Property>\n";
        }
    }

    //классификаторы
    if (cont.prepared_sql.find(name+"_classifiers") == cont.prepared_sql.end()) {
        std::string sql(R"(
        select k.Код          as Код_K,
               k.Наименование as Наименование_K,
               t.ЭлементКлассификатора
        from )");
        sql.append(table_classifiers+R"( as t
             join rКлассификаторыid as k on(k.Ссылка=t.Классификатор)
             join rЭлементыКлассификаторовid as e on(e.Ссылка=t.ЭлементКлассификатора)
        where t.Ссылка = $1 )");
        cont.prepared_sql[name+"_classifiers"] = sql;
        db.p_cn->prepare(name+"_classifiers", sql);
    }
    rs = db.p_W->exec_prepared(name+"_classifiers",item_ref);
    for (const auto &row:rs) {
        sout << "\t\t\t<Property>\n"
             << "\t\t\t\t<PropertyType>classifier</PropertyType>\n"
             << "\t\t\t\t<PropertyID>"  << row["Код_K"].c_str() << "</PropertyID>\n"
             << "\t\t\t\t<PropertyName>"<< row["Наименование_K"].c_str() << "</PropertyName>\n"
             << "\t\t\t\t<PropertyValue>\n";
        std::string a="rЭлементыКлассификаторовid",b="ЭлементКлассификатора";
        const std::string *p_vid = &a,
                          *p_field = &b;

        field(row, *p_vid, *p_field, sout, cont, system_ref);

        sout << "\t\t\t\t</PropertyValue>\n"
             << "\t\t\t</Property>\n";
    }
    //атрибуты
    if (cont.prepared_sql.find(name+"_attributes") == cont.prepared_sql.end()) {
        std::string sql(R"(
        select h.Ссылка,
               h.Код,
               h.Наименование,
               t.Значение_Тип,
               t.Значение_Вид,
               t.Значение_Стр,
               t.Значение_Дата,
               t.Значение_Число,
               t.Значение_Булево,
               t.Значение
        from )");
        sql.append(table_attributes+R"( as t
             join hАтрибутыid as h on(h.Ссылка=t.Атрибут)
        where t.Ссылка = $1 )");
        cont.prepared_sql[name+"_attributes"] = sql;
        db.p_cn->prepare(name+"_attributes", sql);
    }
    rs = db.p_W->exec_prepared(name+"_attributes",item_ref);
    for (const auto &row : rs) {
        if (restricted_attrs.find(row["Ссылка"].c_str()) != restricted_attrs.end())
            continue;

        std::string a,b;
        const std::string *p_vid = &a,
                        *p_field = &b;

        if (row["Значение_Тип"].c_str()[3] == '5') {
            a = "Строка";
            b = "Значение_Стр";
        } else if (row["Значение_Тип"].c_str()[3] == '4') {
            a = "Дата";
            b = "Значение_Дата";
        } else if (row["Значение_Тип"].c_str()[3] == '3') {
            a = "Число";
            b = "Значение_Число";
        } else if (row["Значение_Тип"].c_str()[3] == '2') {
            a = "Булево";
            b = "Значение_Булево";
        } else if (row["Значение_Тип"].c_str()[3] == '8') {
            p_vid  =&cont.mTablesType[row["Значение_Вид"].c_str()].table;
            b = "Значение";
            p_field=&b;
        } else
            continue;

        sout << "\t\t\t<Property>\n"
             << "\t\t\t\t<PropertyType>attribute</PropertyType>\n"
             << "\t\t\t\t<PropertyID>" << row["Код"].c_str() << "</PropertyID>\n"
             << "\t\t\t\t<PropertyName>" << row["Наименование"].c_str() << "</PropertyName>\n"
             << "\t\t\t\t<PropertyValue>\n";

        field(row, *p_vid, *p_field, sout, cont, system_ref);

        sout << "\t\t\t\t</PropertyValue>\n"
             << "\t\t\t</Property>\n";
    }

    //остальные таблицы
    for (auto &e : tables) {
        if (e.first == ""
           || e.first == "Атрибуты"
           || e.first == "Классификация"
           || e.first == "Ярлыки"
           || e.first == "Изменения"
           || e.second.size() == 0)
            continue;

        const std::string &tab_name = e.second[0].table;
        if (cont.prepared_sql.find(tab_name) == cont.prepared_sql.end()) {
            std::string sql("select * from ");
            sql.append(tab_name + " as t where t.Ссылка = $1");
            cont.prepared_sql[tab_name] = sql;
            db.p_cn->prepare(tab_name, sql);
        }
        rs = db.p_W->exec_prepared(tab_name,item_ref);
        if (rs.affected_rows()) {
            sout << "\t\t\t<Table>\n"
                 << "\t\t\t\t<Name>" << e.first << "</Name>\n";
            unsigned n=0;
            for (const auto &row:rs) {
                sout << "\t\t\t\t<Strings>\n"
                     << "\t\t\t\t\t<Number>" << n++ <<"</Number>\n";
                for (const Field &x : e.second) {

                    const std::string *p_vid = &x.vid,
                            *p_field = &x.field;
                    std::string tmp;
                    bool is_set = false;
                    if (x.vid.find("Тип") != std::string::npos) {
                        if (row[x.field_type].as<int>() == 5) {
                            tmp = "Строка";
                            p_field = &x.field_str;
                            is_set = true;
                        } else if (row[x.field_type].as<int>() == 4) {
                            tmp = "Дата";
                            p_field = &x.field_date;
                            is_set = true;
                        } else if (row[x.field_type].as<int>() == 3) {
                            tmp = "Число";
                            p_field = &x.field_num;
                            is_set = true;
                        } else if (row[x.field_type].as<int>() == 2) {
                            tmp = "Булево";
                            p_field = &x.field_bool;
                            is_set = true;
                        }
                        if (is_set) p_vid = &tmp;
                    }
                    if (!is_set && x.vid.find("Вид") != std::string::npos) {
                        p_vid = &cont.mTablesType[row[x.field_vid].c_str()].table;
                        p_field = &x.field_ref;
                    }
                    is_set = true;
                    if (is_set) {
                        sout << "\t\t\t<PropertyList>\n"
                             << "\t\t\t\t<PropertyType>props</PropertyType>\n"
                             << "\t\t\t\t<PropertyID>" << x.name << "</PropertyID>\n"
                             << "\t\t\t\t<PropertyName>" << x.name << "</PropertyName>\n"
                             << "\t\t\t\t<PropertyValue>\n";

                        field(row, *p_vid, *p_field, sout, cont, system_ref);

                        sout << "\t\t\t\t</PropertyValue>\n"
                             << "\t\t\t</PropertyList>\n";
                    }


                }
                sout << "\t\t\t\t</Strings>\n"
                     << "\t\t\t</Table>\n";
            }
        }
    }

    sout << "\t\t\t<IdStatus>000000002</IdStatus>\n";
    rs = db.p_W->exec_prepared("_sВерсииОбъектовid_",ref,item_ref);
    if (rs.affected_rows())
        sout << "\t\t\t<Version>\n"
             << "\t\t\t\t<Id>" << rs[0][0].c_str() << "</Id>\n"
             << "\t\t\t\t<Start>" << rs[0][1].c_str() << "</Start>\n"
             << "\t\t\t\t<End xsi:nil='true'/>\n"
             << "\t\t\t</Version>\n";
    sout << "\t\t</Item>\n";
}

