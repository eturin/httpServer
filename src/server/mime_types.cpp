#include "mime_types.hpp"

namespace http {
    namespace server3 {
        namespace mime_types {
            struct mapping {
                const char* extension;
                const char* mime_type;
            } m[] = {
                        { "gif" , "image/gif"  },
                        { "htm" , "text/html"  },
                        { "html", "text/html"  },
                        { "jpg" , "image/jpeg" },
                        { "png" , "image/png"  }
            };

            std::string extension_to_type(const std::string& extension) {
                for (auto &e : m)
                    if (e.extension == extension)
                        return e.mime_type;

                return "text/plain";
            }

        } // namespace mime_types
    } // namespace server3
} // namespace http