#ifndef MKXML_CONTEXT_H
#define MKXML_CONTEXT_H

#define REF_SIZE 16
#define LIDS "LIDS"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <string>
#include <vector>
#include <sstream>
#include "db.h"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/syslog_sink.h"

struct RowTableType;

struct Context {
    Context();
    ~Context();

    DB & get_con();
    DB * activate_con(DB *);
    std::basic_string<std::byte> from_hex(const std::string & str) const;

    std::map<std::string,std::string> prepared_sql;
    unsigned max_items;
    std::string outer_ref;
    std::vector<DB*> pool;
    std::string get_connection_string() const;
    std::map<std::string, RowTableType> mTablesType;
    std::map<std::string, std::string>  mTablesName;
    std::map<std::string, std::string>  mTablesNameShort;
    std::string connection_string;
};

#endif
