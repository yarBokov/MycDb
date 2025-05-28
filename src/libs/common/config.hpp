#ifndef __COMMON_CONFIG_H
#define __COMMON_CONFIG_H

#include <string>

namespace config
{
    //paths
    extern const std::string db_directory = "/opt/mycdb/";
    extern const std::string dbs = "/opt/mycdb/db/";
    extern const std::string db_conf_file = "opt/mycdb/mycdb.conf";
    extern const std::string sql_query_dict = "opt/mycdb/querydict.txt";

    //db
    extern const std::string table_ext = ".tbl";
}

#endif