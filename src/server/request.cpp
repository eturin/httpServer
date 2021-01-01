#include "request.hpp"

namespace http {
    namespace server3 {
        bool request::save(pqxx::connection * conn, Context & cont) const {
            pqxx::work W(*conn);
            if (!conn->is_open() && !cont.prepare(conn))
                return false;

            pqxx::binarystring blob( &body[0], body_size );
            pqxx::result r = W.prepared("dml_queue")(method)(end_point)(uri)(blob).exec();
            if (r.affected_rows() != 1)
                return false;

            id = r[0]["id"].as<unsigned>();
            for (auto &e : headers)
                pqxx::result r = W.prepared("dml_headers")(id)(e.name)(e.value).exec();

            W.commit();
            return true;
        }
    }
}
