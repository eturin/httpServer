#ifndef MKXML_REF_TYPE_H
#define MKXML_REF_TYPE_H

#include "outer_type.h"
#include "Context.h"
#include "field.h"

#include <pqxx/pqxx>
#include <string>
#include <vector>

struct RefType: public OuterType {
    RefType(pqxx::binarystring outer_id, Context & cont);
    std::vector<std::pair<pqxx::binarystring, unsigned> > get_changes() const;
    void send(const std::ostringstream &sout,
              const std::vector<std::pair<pqxx::binarystring, unsigned> > &vref,
              const std::string &message_date,
              const pqxx::binarystring &message_ref,
              const std::string &message_id,
              bool isSSL,
              bool isMultipart,
              const std::string &host,
              const std::string &port,
              const std::string &target,
              const std::string &user,
              const std::string &pass,
              const pqxx::binarystring &integ_ref,
              const pqxx::binarystring &node_ref,
              bool dontSend) const;
    std::size_t mkXMLs(bool isSSL,
                       bool isMultipart,
                       const std::string &host,
                       const std::string &port,
                       const std::string &target,
                       const std::string &user,
                       const std::string &pass,
                       const pqxx::binarystring &integ_ref,
                       const pqxx::binarystring &node_ref,
                       bool dontSend) const;
    void item(const pqxx::binarystring &ref, std::ostringstream &sout) const;
    std::map<std::string, std::vector<Field> > tables;
    std::map<std::string, std::string>         restricted_attrs;
    Context & cont;
};

#endif
