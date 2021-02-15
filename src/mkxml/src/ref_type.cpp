#include "ref_type.h"
#include <iostream>

static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                        "abcdefghijklmnopqrstuvwxyz"
                                        "0123456789+/";
std::string makeFormContent(const std::string & body) {
    std::ostringstream ss;

    ss << "--41d148306c0b2d3d0dd779d462883b7c\r\n"
       << "X-Seq-Num: 1\r\n"
       << "X-Is-Last: true\r\n"
       << "Content-Disposition: form-data; name=\"MessageText\"\r\n\r\n"
       << body << "\r\n";

    ss << "--41d148306c0b2d3d0dd779d462883b7c--";

    return ss.str();
}
static inline bool is_base64(char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}
std::string base64_encode(const char *buf,
                          unsigned int bufLen) {
    std::string ret;
    int i = 0;
    int j = 0;
    char char_array_3[3];
    char char_array_4[4];

    while (bufLen--) {
        char_array_3[i++] = *(buf++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for(i = 0; (i <4) ; i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i)
    {
        for(j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];

        while((i++ < 3))
            ret += '=';
    }

    return ret;
}
std::vector<char> base64_decode(std::string const& encoded_string) {
    int in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    char char_array_4[4], char_array_3[3];
    std::vector<char> ret;

    while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_]; in_++;
        if (i ==4) {
            for (i = 0; i <4; i++)
                char_array_4[i] = base64_chars.find(char_array_4[i]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret.push_back(char_array_3[i]);
            i = 0;
        }
    }

    if (i) {
        for (j = i; j <4; j++)
            char_array_4[j] = 0;

        for (j = 0; j <4; j++)
            char_array_4[j] = base64_chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++) ret.push_back(char_array_3[j]);
    }

    return ret;
}
inline void load_root_certificates(boost::asio::ssl::context& ctx) {
    const std::string cert = R"(-----BEGIN CERTIFICATE-----
MIIFKjCCBBKgAwIBAgITLwAAAA/kBUusWE4AFAABAAAADzANBgkqhkiG9w0BAQsF
ADBAMRQwEgYKCZImiZPyLGQBGRYEVEVTVDETMBEGCgmSJomT8ixkARkWA0RFVjET
MBEGA1UEAxMKREVWLVJPT1RDQTAeFw0yMDAyMTExMDU2MThaFw0yNTAyMDkxMDU2
MThaMEExFDASBgoJkiaJk/IsZAEZFgRURVNUMRMwEQYKCZImiZPyLGQBGRYDREVW
MRQwEgYDVQQDEwtERVYtU1VCQ0EwMjCCASIwDQYJKoZIhvcNAQEBBQADggEPADCC
AQoCggEBAOgnOMdu0wLFdYqWN0RNabq2YaOowS8t/xr4Q5d1eZdJ8AyiaakOXLoe
Ef77805obJOrXn8yC9ov7GOnxWhIwx8IFmuVzOaT3pyP4GGXZ0nZDESyOpRucQGc
pOzwzDhRhlFYexFd34TDDFRdsDjaD0VDTm6OL03SzQS0XFGqbPZ7chQyJHEqhOgV
qSF/PE9cGHxhsqlYwcMXtzX/RTDUCS2FOMzYBPFK0z90aDQUNsUycXbWofHBv97P
9GB1nr7SgdTPLIR0+jA0smOqwcgqcU6X18flFiAHrLlf2pdvWNMjb0APuVfk4jPf
pRUKyFuGWvUmW8o8fuamNfJ1p1A4lgkCAwEAAaOCAhowggIWMBAGCSsGAQQBgjcV
AQQDAgEAMB0GA1UdDgQWBBRNY3HiYirva/FLNVZC6MsJOLpXTDAZBgkrBgEEAYI3
FAIEDB4KAFMAdQBiAEMAQTAOBgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB
/zAfBgNVHSMEGDAWgBQfJgu9ve/Sf9jqUgX4s7NRPJwPbjCByQYDVR0fBIHBMIG+
MIG7oIG4oIG1hoGybGRhcDovLy9DTj1ERVYtUk9PVENBLENOPURFVi1TUi1EQzAx
LENOPUNEUCxDTj1QdWJsaWMlMjBLZXklMjBTZXJ2aWNlcyxDTj1TZXJ2aWNlcyxD
Tj1Db25maWd1cmF0aW9uLERDPURFVixEQz1URVNUP2NlcnRpZmljYXRlUmV2b2Nh
dGlvbkxpc3Q/YmFzZT9vYmplY3RDbGFzcz1jUkxEaXN0cmlidXRpb25Qb2ludDCB
uQYIKwYBBQUHAQEEgawwgakwgaYGCCsGAQUFBzAChoGZbGRhcDovLy9DTj1ERVYt
Uk9PVENBLENOPUFJQSxDTj1QdWJsaWMlMjBLZXklMjBTZXJ2aWNlcyxDTj1TZXJ2
aWNlcyxDTj1Db25maWd1cmF0aW9uLERDPURFVixEQz1URVNUP2NBQ2VydGlmaWNh
dGU/YmFzZT9vYmplY3RDbGFzcz1jZXJ0aWZpY2F0aW9uQXV0aG9yaXR5MA0GCSqG
SIb3DQEBCwUAA4IBAQBK7oe3pGKtcPL3b0R9BAqkdbQKqiE7XK0MTM7rB31BCtaQ
A5bXQpuWcJK9RXBqsECTH2K1VAszDsyonhiqzdhLbiAx/NlDWQ40UijbGUjmQAeS
iMSCqvR2KZnuU9r3I1tjj3Kyo11cbl8CBvb61Ax4GXRlAKsfQG13j+26OJoxmnF7
B2zU9pvDpRMkfMakM8oj66tsJfUXRS/ZXsuUfgINX0hkvSgnqGPb0wQxi463zdTA
U8YpPs0EvK+Os6lYFVj15tqB9nlU27FBCOQRkKyZB2dsTWMcnnEFVcBHXlNYWVLR
OO964AN3MRjvy/3AgncRbYqWokrqqnEfTNQ8rTUF
-----END CERTIFICATE-----
-----BEGIN CERTIFICATE-----
MIIDgjCCAmqgAwIBAgIQVIfDgh0x5oZFBioW1mvbhDANBgkqhkiG9w0BAQsFADBA
MRQwEgYKCZImiZPyLGQBGRYEVEVTVDETMBEGCgmSJomT8ixkARkWA0RFVjETMBEG
A1UEAxMKREVWLVJPT1RDQTAgFw0yMDAyMDYwNTM1MzRaGA8yMDUwMDIxMTA5MDcx
N1owQDEUMBIGCgmSJomT8ixkARkWBFRFU1QxEzARBgoJkiaJk/IsZAEZFgNERVYx
EzARBgNVBAMTCkRFVi1ST09UQ0EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEK
AoIBAQCzJy5rZ/EXhuH/lee+oKwdXMFyREMiGztPDh4DztmNjhlofAluc9eZy5SX
R0vYnjUbqDbJaKSJ8GmbPWQj21nAh8y5zCOUomWRZr9iv3Oj9S92W1N9dCDW6Dgc
ITKX/JT2IVkFltSARfI6iGmJuf+OcjYrkCo2auXM9GRkH0y4sXNSoRBII15f5OFv
bKtvbyVs5yXdiwG5a7a4dPPMPb7F5JQLyPJXvTqdxZrwZiWe4tpieAXMhzFfXj8v
sUK+8frjwaLmyVw8eShDCqd0hTJajndK1Fy1o/XBjrlK5s2J08CitzRFCrZ0NW9m
CphCOlRa+oOmzh1wIEdhuQDCssw1AgMBAAGjdjB0MAsGA1UdDwQEAwIBhjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBQfJgu9ve/Sf9jqUgX4s7NRPJwPbjAQBgkr
BgEEAYI3FQEEAwIBATAjBgkrBgEEAYI3FQIEFgQU6Q1K76KrgJ4KWUwSZOjme/zk
mA8wDQYJKoZIhvcNAQELBQADggEBAIKgwDRFuWoFcZ0J/RvyK7qyTYMx2pBSacjA
I8eRwoK4EjPp0VJ124VFhp3bI1Au4GI+NfB0527C7/NiJOP11qXIR7AmWRZjrlFg
/htVqm6eS0qZ6bc9B7GoMueHg5+kW5UEdeVIU3CP1kYaqu88yiiAlvmCfkqKTyh+
CO9ETnqvKAA9gdGNIPIAB1tlHnxKpXuk8+2Xt9v5zgshCfhvJlvxM6Ac5COKig//
VE0/b8lXkJ51u1Jbasa+CRPxettLJ3kEMZEfRBl9tlbutkyWZSGFk1iqskwA1eyg
FujqWSe4XcsJs6KiotdTe1avTuW2x1cWXdCP8WMMHq7K7LTGhhc=
-----END CERTIFICATE-----)";

    boost::system::error_code ec;
    ctx.add_certificate_authority(boost::asio::buffer(cert.data(), cert.size()), ec);
    if(ec) throw boost::system::system_error{ec};
}
void field(const pqxx::row &row,
           const std::string &vid,
           const std::string &field,
           std::ostringstream &sout,
           Context &cont,
           const std::string &system_ref) {

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
        pqxx::result rs = db.p_W->exec_prepared(rtt.table+"_short", pqxx::to_string(row[field]));
        std::string GID = rs.affected_rows() ? rs[0]["Код"].c_str() : "",
                name= rs.affected_rows() ? rs[0]["Наименование"].c_str() : "";

        if (vid == "rЗначенияАтрибутовid") {
            sout << "\t\t\t\t\t<Type>enum</Type>\n"
                 << "\t\t\t\t\t<Value>\n"
                 << "\t\t\t\t\t\t<valueString>" << name << "</valueString>\n"
                 << "\t\t\t\t\t</Value>\n"
                 << "\t\t\t\t\t<Name/>\n"
                 << "\t\t\t\t\t<TableName>ЗначенияАтрибутов</TableName>\n";
        } else if (vid == "rФайлid") {
            pqxx::result rs = db.p_W->exec_prepared("_rФайлid_",rtt.ref);
            sout << "\t\t\t\t\t<Type>simple</Type>\n"
                 << "\t\t\t\t\t<Value>\n"
                 << "\t\t\t\t\t\t<valueString>"<< (rs.affected_rows() ? rs[0]["ДвоичныеДанныеФайла_b64"].c_str() : "") << "</valueString>\n"
                 << "\t\t\t\t\t</Value>\n";
        } else {
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


                pqxx::result rs2 = db.p_W->exec_prepared(LIDS, rtt.ref, pqxx::to_string(row[field]), pqxx::to_string(rs[0]["Ссылка"]));
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
RefType::RefType(std::string outer_ref,
                 Context &cont): OuterType(outer_ref,cont),
                                 cont(cont) {
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
std::vector<std::pair<std::string, unsigned> > RefType::get_changes() const {
    DB &db=cont.get_con();
    if (cont.prepared_sql.find(name+"changes") == cont.prepared_sql.end()) {
        std::string sql("select t.Ссылка, t.НомерСообщения  from nОбменНСИid as n join  ");
        sql.append(table_change).append(" as t on(t.node_Вид=E'\\\\x0000002E' and t.node=n.Ссылка) where n.ВнешнийСправочник = $1");
        cont.prepared_sql[name + "changes"] = sql;
        db.p_cn->prepare(name + "changes", sql);
    }
    pqxx::result rs = db.p_W->exec_prepared(name+"changes",outer_ref);
    std::vector<std::pair<std::string, unsigned> > v;
    for (const auto &row : rs)
        v.push_back({pqxx::to_string(row[0]), row[1].as<unsigned>()});
    return v;
}
void RefType::send(const std::ostringstream &sout,
                   const std::vector<std::pair<std::string, unsigned> > &vref,
                   const std::string &message_date,
                   const std::string &message_ref,
                   const std::string &message_id,
                   bool isSSL,
                   bool isMultipart,
                   const std::string &host,
                   const std::string &port,
                   const std::string &target,
                   const std::string &user,
                   const std::string &pass,
                   const std::basic_string<std::byte> &integ_ref,
                   const std::string &node_ref,
                   bool dontSend) const {

    std::stringstream ss;

    std::string new_target(target);
    if (!isMultipart) {
        std::size_t pos = target.find("{КодыИнформационныхСистем}");
        if (pos != std::string::npos)
            new_target = new_target.substr(0,pos)+system_code;
    }

    try {
        int  version = 11; //или 10 (т.е. 1.0)
        // формируем HTTP-запрос
        boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::post,
                                                                         new_target,
                                                                         version};
        req.set(boost::beast::http::field::host, host);
        req.set( "Accept","*/*");
        req.set("charset", "utf-8");
        std::string strUserPass = std::string(user)+":"+pass;
        req.set(boost::beast::http::field::authorization, std::string("Basic ").append(base64_encode(strUserPass.c_str(),strUserPass.size())));
        if (isMultipart) {
            req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
            req.set("X-Data-Source", std::string("urn://dts/nsi/to_").append(system_code + "/v1.0.1"));
            req.set("X-Message-Type", "application/xml");
            req.set("X-Request-Id", std::string("urn:pts:nsi:").append(message_id));
            req.set(boost::beast::http::field::content_type,
                    "multipart/form-data; boundary=41d148306c0b2d3d0dd779d462883b7c");

            req.body() = makeFormContent(sout.str());
        } else {
            req.set(boost::beast::http::field::user_agent,"1C+Enterprise/8.3");
            req.set(boost::beast::http::field::content_type, "application/xml");
            req.body() = sout.str();
        }
        req.prepare_payload(); // вычисление размера тела

        // io_context требуется для всех операций I/O (ввода/вывода)
        boost::asio::io_context ioc;
        // этот объект выполняет операции I/O (ввода/вывода)
        boost::asio::ip::tcp::resolver resolver(ioc);

        // определяем ip-адрес по доменну
        auto const results = resolver.resolve(host, port);
        //isSSL = false;
        //auto const results = resolver.resolve("127.0.0.1", "7777");

        if (isSSL) {
            // SSL нужен для работы с сертификатами
            boost::asio::ssl::context ctx(boost::asio::ssl::context::tlsv12_client);
            // указываем доверенные сертификаты
            load_root_certificates(ctx);
            // проверяем сертификат сервера
            ctx.set_verify_mode(boost::asio::ssl::verify_peer);

            boost::beast::ssl_stream<boost::beast::tcp_stream> stream(ioc, ctx);
            // устанавливаем SNI Hostname (для успешной установки связи)
            if (!SSL_set_tlsext_host_name(stream.native_handle(), host.c_str())) {
                boost::beast::error_code ec{static_cast<int>(::ERR_get_error()),
                                            boost::asio::error::get_ssl_category()};
                throw boost::beast::system_error{ec};
            }

            // устанавливаем соединение c ip-адресом, полученным выше
            boost::beast::get_lowest_layer(stream).connect(results);
            // SSL handshake
            stream.handshake(boost::asio::ssl::stream_base::client);

            if (!dontSend) {
                // отправляем запрос
                boost::beast::http::write(stream, req);
                // буфер, для извлечения ответа
                boost::beast::flat_buffer buffer;
                // контейнер для хранения ответа
                boost::beast::http::response<boost::beast::http::dynamic_body> res;
                // вычитываем HTTP-ответ
                boost::beast::http::read(stream, buffer, res);

                // Write the message to standard out
                ss << res << std::endl;
            } else ss << "Не отправлялось сообщение" << std::endl;

            // закрываем сокет
            boost::beast::error_code ec;
            stream.shutdown(ec);
            if (ec == boost::asio::error::eof) {
                ec = {}; // причина: http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
            }
            if (ec) throw boost::beast::system_error{ec};

            // в этом месте соединение корректно закрыто
        } else {
            boost::beast::tcp_stream stream(ioc);
            // устанавливаем соединение c ip-адресом, полученным выше
            stream.connect(results);

            if (!dontSend) {
                // отправляем запрос
                boost::beast::http::write(stream, req);
                // буфер, для извлечения ответа
                boost::beast::flat_buffer buffer;
                // контейнер для хранения ответа
                boost::beast::http::response<boost::beast::http::dynamic_body> res;
                // вычитываем HTTP-ответ
                boost::beast::http::read(stream, buffer, res);

                // Write the message to standard out
                ss << res << std::endl;
            } else ss << "Не отправлялось сообщение" << std::endl;

            // закрываем сокет
            boost::beast::error_code ec;
            stream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

            if(ec && ec != boost::beast::errc::not_connected)
                throw boost::beast::system_error{ec};

            // в этом месте соединение корректно закрыто
        }
    } catch(std::exception const& e) {
        ss << "Error: " << e.what() << std::endl;
        std::cerr << "Error: " << ss.str() << std::endl;
    }

    DB &db=cont.get_con();

    for (const auto &x : vref) {
        db.p_W->exec_prepared("sСеансыОбменаid", message_date, message_ref, outer_ref, ref, x.first);
        db.p_W->exec_prepared(table_change+"_update", std::stoul(message_id), node_ref, x.first);
    }

    db.p_W->exec_prepared("sИсторияОбменаid",message_date,integ_ref,message_ref,sout.str(),ss.str());
    db.commit();

}
std::size_t RefType::mkXMLs(bool isSSL,
                            bool isMultipart,
                            const std::string &host,
                            const std::string &port,
                            const std::string &target,
                            const std::string &user,
                            const std::string &pwd,
                            const std::basic_string<std::byte> &integ_ref,
                            const std::string &node_ref,
                            bool dontSend) const {
    std::size_t cnt=0;

    DB &db=cont.get_con();
    if (cont.prepared_sql.find("спрСеансыОбмена") == cont.prepared_sql.end()) {
        std::string sql(R"(
        select pg_advisory_xact_lock(hashtext('insert_спрСеансыОбмена')::bigint);
        )");
        cont.prepared_sql["insert_спрСеансыОбмена"] = sql;
        db.p_cn->prepare("insert_спрСеансыОбмена", sql);

        sql = R"(
        insert into rСеансыОбменаid (ИмяПредопределенныхДанных             ,Ссылка                                                ,Владелец,Код                                             ,Дата               ,ПометкаУдаления,Комментарий,ЭтоОшибка,СеансЗавершен,Направление                           ,ВнешнийСправочник)
                             values (E'\\x00000000000000000000000000000000',decode(replace(uuid_generate_v4()::text,'-',''),'hex'),  $1    ,(select max(Код)+1 as Code from rСеансыОбменаid),now()::timestamp(0),false          ,''         ,false    ,false        ,E'\\xA0C2944C1269CD5445A9BA93230574B0',   $2            )
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
        insert into sИсторияОбменаid(Период,ИнтеграционнаяКомпонента,СеансОбмена,НаправлениеОбмена                     ,УИД                               ,ТекстЗапроса,ТекстОтвета,АдресВызова)
                              values($1    ,   $2                   ,$3         ,E'\\xA0C2944C1269CD5445A9BA93230574B0',uuid_generate_v4()::text::mvarchar,   $4       ,  $5       ,'mkXML'    )

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

        sql = R"(/*получение данных файла*/
        select r.ДвоичныеДанныеФайла_b64
        from rФайлыid as f
             join sДвоичныеДанныеФайловid as r on(r.Файл=f.ТекущаяВерсия)
        where f.ссылка = $1)";
        cont.prepared_sql["_rФайлid_"] = sql;
        db.p_cn->prepare("_rФайлid_", sql);
    }

    std::string message_id, message_date;
    std::string message_ref("");

    std::ostringstream sout;
    std::vector<std::pair<std::string, unsigned> > vrefs;
    for (const auto &pair : get_changes()) {
        if (0 == cnt % cont.max_items) {
            if (cnt) {
                sout << "\t</Items>\n"
                     << "</Message>\n";
                send(sout,
                     vrefs,
                     message_date,
                     message_ref,
                     message_id,
                     isSSL,
                     isMultipart,
                     host,
                     port,
                     target,
                     user,
                     pwd,
                     integ_ref,
                     node_ref,
                     dontSend);
                sout.clear();
                vrefs.clear();
            }
            db.p_W->exec_prepared("insert_спрСеансыОбмена");
            pqxx::result rs = db.p_W->exec_prepared("спрСеансыОбмена",integ_ref,outer_ref);
            message_id   = rs[0][2].c_str();
            message_date = rs[0][1].c_str();
            message_ref  = pqxx::to_string(rs[0][0]);
            db.commit();

            sout << "<?xml version='1.0' encoding='UTF-8'?>\n"
                 << "<Message xmlns='http://www.lmsoft.ru/mdm/exchange'\n"
                 << "         xmlns:xs='http://www.w3.org/2001/XMLSchema'\n"
                 << "         xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'>\n"
                 << "\t<MessageID>" << message_id << "</MessageID>\n"
                 << "\t<Items>\n";
        }
        vrefs.push_back(pair);
        item(pair.first, sout);
        ++cnt;
    }

    if (cnt) {
        sout << "\t</Items>\n"
             << "</Message>\n";
        send(sout,
             vrefs,
             message_date,
             message_ref,
             message_id,
             isSSL,
             isMultipart,
             host,
             port,
             target,
             user,
             pwd,
             integ_ref,
             node_ref,
             dontSend);
        sout.clear();
        vrefs.clear();
    }

    return cnt;
}
void RefType::item(const std::string &item_ref, std::ostringstream &sout) const {
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

        std::string dml("update ");
        dml.append(table_change).append(R"( set НомерСообщения = $1 where node_Вид = E'\\x0000002E' /*обмен НСИ*/ and  node = $2 and Ссылка = $3 and НомерСообщения < $1 )");
        cont.prepared_sql[table_change+"_update"] = dml;
        db.p_cn->prepare(table_change+"_update", dml);
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


