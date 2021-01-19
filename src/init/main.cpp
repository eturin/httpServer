#include <cstdlib>
#include <iostream>
#include <pqxx/pqxx>
#include <string>

int main(int argc, char* argv[]) {

    try {
        std::cout<<"Start init db...\n";
        const char *db_host      = std::getenv("DB_HOST"),
                   *db_port      = std::getenv("DB_PORT"),
                   *db_name      = std::getenv("DB_NAME"),
                   *db_user      = std::getenv("DB_USER"),
                   *db_user_pass = std::getenv("DB_USER_PASS");
        std::string connection_str =std::string(R"(
            dbname   = )")+ (db_name      ? db_name     : static_cast<const char*>("mdm"))     + R"(
            user     = )" + (db_user      ? db_user     : static_cast<const char*>("postgres"))+ R"(
            password = )" + (db_user_pass ? db_user_pass: static_cast<const char*>("knpo1980"))+ R"(
            hostaddr = )" + (db_host      ? db_host     : static_cast<const char*>("127.0.0.1"))+R"(
            port     = )" + (db_port      ? db_port     : static_cast<const char*>("5432"))    + R"(
        )";
        std::cout << connection_str<<'\n';
        pqxx::connection C(connection_str);
        if (C.is_open()) {
            std::cout << "Opened database successfully: " << C.dbname() <<'\n'<< std::endl;
            pqxx::work W(C);
            W.exec(R"(
            CREATE EXTENSION IF NOT EXISTS "uuid-ossp";
            CREATE EXTENSION IF NOT EXISTS "pageinspect";
            )");
            W.commit();
        } else {
            std::cout << "Can't open database" << std::endl;
            return 1;
        }
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
