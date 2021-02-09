#include "outer_type.h"

OuterType::OuterType(const std::string &outer_ref,
                     Context & cont) : RowTableType(),
                                       outer_ref(outer_ref) {
    DB &db=cont.get_con();
    RowTableType rtt_idobj("rИдентификаторыОбъектовМетаданныхid",cont);
    if (cont.prepared_sql.find("ВнешнийСправочник") == cont.prepared_sql.end()) {
        cont.prepared_sql["ВнешнийСправочник"] = std::string(R"(
        select
            t.Код,
            t.Наименование,
            'r'||i.Имя||'id' as Имя,
            t.Владелец,
            s.Код as ВладелецКод
        from rВнешниеСправочникиid as t
             join )") + rtt_idobj.table + R"( as i on(i.Ссылка = t.ИдентификаторСправочникаНСИ)
             join rВнешниеИнформационныеСистемыid as s on(s.Ссылка = t.Владелец)
        where t. Ссылка = $1
        limit 1)";
        db.p_cn->prepare("ВнешнийСправочник", cont.prepared_sql["ВнешнийСправочник"]);
    }

    pqxx::result rs = db.p_W->exec_prepared("ВнешнийСправочник",outer_ref);
    outer_code  = rs[0]["Код"].c_str();
    outer_name  = rs[0]["Наименование"].c_str(),
            name        = rs[0]["Имя"].c_str();
    system_ref  = pqxx::to_string(rs[0]["Владелец"]);
    system_code = rs[0]["ВладелецКод"].c_str();

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

