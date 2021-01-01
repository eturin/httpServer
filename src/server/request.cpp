#include "request.hpp"

namespace http {
    namespace server3 {
        bool request::save(pqxx::connection * conn) const {
            return true;
        }
    }
}
