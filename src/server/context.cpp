#include "context.hpp"
#include "spdlog/spdlog.h"
#include "common.hpp"

namespace http {
    namespace server3 {
        Context::Context(std::shared_ptr<spdlog::logger> &syslog_logger) : uuid_num(),conns(),syslog_logger(syslog_logger) {}

        bool Context::prepare(pqxx::connection *conn) {
            if (conn->is_open()) {
                syslog_logger->info("Настройка соединения с СУБД");
                conn->prepare("dml_queue_post", R"(
                --запрос
                insert into rq_ВходящаяОчередьid (Ссылка                                                ,ПометкаУдаления,ИмяПредопределенныхДанных             ,Дата ,URL,Метод,ТипСообщения,Размер,КоличествоПопыток,СообщениеОбработчика,ДатаОбработки               ,ВходящийИдентификатор,data)
                                          values (decode(replace(uuid_generate_v4()::text,'-',''),'hex'),false          ,E'\\x00000000000000000000000000000000',now(),$1 ,$2   ,$3          ,$4    ,0                ,''                  ,'0001-01-01 00:00:00.000000',''                   ,E'\\x01013D00000000000000EFBBBF7B2223222C38373132363230302D336539382D343465302D623933312D6363623164376564633439372C0D0A7B312C0D0A7B307D0D0A7D0D0A7D00000000'::bytea || $5 || E'\\x00000000'::bytea || $6)
                returning Ссылка;
                )");
                conn->prepare("dml_queue_get", R"(
                --запрос
                insert into rq_ВходящаяОчередьid (Ссылка                                                ,ПометкаУдаления,ИмяПредопределенныхДанных             ,Дата ,URL,Метод,ТипСообщения,Размер,КоличествоПопыток,СообщениеОбработчика,ДатаОбработки               ,ВходящийИдентификатор,data                                      )
                                          values (decode(replace(uuid_generate_v4()::text,'-',''),'hex'),false          ,E'\\x00000000000000000000000000000000',now(),$1 ,$2   ,$3          ,0     ,0                ,''                  ,'0001-01-01 00:00:00.000000',''                   ,E'\\x01010800000000000000EFBBBF7B2255227D')
                returning Ссылка;
                )");
                conn->prepare("dml_headers", R"(
                --заголовки запроса
                insert into rq_ВходящаяОчередь_Заголовкиid(Ссылка,KeyField,НомерСтроки,Имя,Значение)
                                                    values(   $1 , $2     , $3        , $4,  $5    )
                )");
                conn->prepare("dml_params", R"(
                --заголовки запроса
                insert into rq_ВходящаяОчередь_Параметрыid(Ссылка,KeyField,НомерСтроки,Имя,Значение)
                                                    values(   $1 , $2     , $3        , $4,  $5    )
                )");
                conn->prepare("is_cpp", R"(
                --определение обработчика
                select t.Синхронно, t.cpp from rq_Запросыid as t where t.Код = $1 and t.Использовать;
                )");
                conn->prepare("save_result_to_queue", R"(
                --сохранение результатов обработки в элемент очереди
                update rq_ВходящаяОчередьid set КоличествоПопыток=КоличествоПопыток+1,СообщениеОбработчика =  substr($2::text || E'\n' || СообщениеОбработчика,0,500)::mvarchar, ДатаОбработки=now() where Ссылка = $1;
                )");
                conn->prepare("arh_queue", R"(
                --перенос элемента очереди в архив
                insert into rq_АрхивОчередиid (Ссылка,ПометкаУдаления,ИмяПредопределенныхДанных,Дата,url,Метод,ТипСообщения,Размер,КоличествоПопыток,СообщениеОбработчика,ДатаОбработки,ВходящийИдентификатор,data)
                select Ссылка,ПометкаУдаления,ИмяПредопределенныхДанных,Дата,url,Метод,ТипСообщения,Размер,КоличествоПопыток,СообщениеОбработчика,ДатаОбработки,ВходящийИдентификатор,data
                from rq_ВходящаяОчередьid
                where Ссылка = $1;
                )");
                conn->prepare("arh_queue_headers", R"(
                --перенос заголовков элемента очереди в архив
                insert into rq_АрхивОчереди_Заголовкиid (Ссылка,keyfield,НомерСтроки,Имя,Значение)
                select Ссылка,keyfield,НомерСтроки,Имя,Значение
                from rq_ВходящаяОчередь_Заголовкиid
                where Ссылка = $1;
                )");
                conn->prepare("arh_queue_params", R"(
                --перенос параметров элемента очереди в архив
                insert into rq_АрхивОчереди_Параметрыid (Ссылка,keyfield,НомерСтроки,Имя,Значение)
                select Ссылка,keyfield,НомерСтроки,Имя,Значение
                from rq_ВходящаяОчередь_Параметрыid
                where Ссылка = $1;
                )");
                conn->prepare("del_queue", R"(/*удаление элемента из очереди*/ delete from rq_ВходящаяОчередьid where Ссылка = $1;)");
                conn->prepare("del_queue_headers", R"(/*удаление заголовков элемента очереди*/ delete from rq_ВходящаяОчередь_Заголовкиid where Ссылка = $1;)");
                conn->prepare("del_queue_params", R"(/*удаление параметров элемента очереди*/ delete from rq_ВходящаяОчередь_Параметрыid where Ссылка = $1;)");
                return true;
            } else return false;
        }

        bool Context::make_pool(std::size_t cnt, const std::string & connection_string){
            try {
                syslog_logger->info("Формирования пула соединений с СУБД");
                for (std::size_t i = 0; i<cnt; ++i) {
                    conns.push_back(new pqxx::connection(connection_string));
                    if (!conns[i]->is_open()) return false;
                    else prepare(conns[i]);
                }
            } catch (const std::exception &e) {
                syslog_logger->error("Ошибка формирования пула соединений с СУБД {}", e.what());
                return false;
            }

            return true;
        }

        pqxx::connection* Context::get_conn() {
            auto it = uuid_num.find(boost::lexical_cast<std::string>(boost::this_thread::get_id()));
            if ( it != uuid_num.end())
                return conns[it->second];

            return nullptr;
        }

        void Context::add_uuid_num(boost::thread::id uuid,char num) {
            uuid_num[boost::lexical_cast<std::string>(uuid)] = num;
        }

        Context::~Context() {
            for (pqxx::connection * e : conns) {
                if (e->is_open()) e->close();
                delete e;
            }
        }
    }
}