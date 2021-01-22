#include <iostream>
#include "Context.h"


int main() {
    Context cont;
    DB &db=cont.get_con();
    if (cont.prepared_sql.find("get_ИнтеграционныеКомпоненты") == cont.prepared_sql.end()) {
        RowTableType obj("sИнтеграционныеКомпонентыВнешнегоСправочникаid",cont);
        std::string sql = std::string(R"(
        select
            t.ВнешнийСправочник,
            i.ЗащищенноеСоединение,
            i.multipart,
            i.Сервер,
            i.Порт,
            i.urlОтправкаДанных,
            i.Логин,
            i.Пароль
        from )"+obj.table +R"( as t
            join rИнтеграционныеКомпонентыid     as i on(i.Ссылка = t.ИнтеграционнаяКомпонента
                                                         and i.ТипИнтеграционнойКомпоненты = E'\\xB9BC00E6850DF7F34528F7A8B2890A1F' /* rest */)
            join rВнешниеСправочникиid           as v on(v.Ссылка = t.ВнешнийСправочник)
            join rВнешниеИнформационныеСистемыid as s on(s.Ссылка = v.Владелец
                                                         and s.Трансформацияout = E'\\x00000000000000000000000000000000')
            join nОбменНСИid                     as n on(n.Ссылка = v.СценарийОбмена
                                                         and n.Состояние = E'\\x82B491E12C75E78D4D8883EE5E515FF1' /* накопление и отправка */)
        where
            t.ИнтеграционнаяКомпонента = $1
        )");
        db.p_cn->prepare("get_ИнтеграционныеКомпоненты", sql);
        cont.prepared_sql["get_ИнтеграционныеКомпоненты"] = sql;
    }

    std::string str_integr_ref;
    while (std::cin >> str_integr_ref) {
        pqxx::binarystring integr_ref = cont.from_hex(str_integr_ref.c_str());
        pqxx::result rs = db.p_W->exec_prepared("get_ИнтеграционныеКомпоненты",integr_ref);
        int n = rs.affected_rows();
        for (const auto &row : rs) {
            cont.outer_ref = pqxx::binarystring(row["ВнешнийСправочник"]);
            std::string str_outer_ref;
            RefType rt = RefType(cont.outer_ref, cont);
            rt.mkXMLs(row["ЗащищенноеСоединение"].c_str()[0] == 't',
                      row["multipart"].c_str()[0] == 't',
                      row["Сервер"].c_str(),
                      row["Порт"].c_str(),
                      row["urlОтправкаДанных"].c_str(),
                      row["Логин"].c_str(),
                      row["Пароль"].c_str(),
                      integr_ref);
        }
    }

    return 0;
}
