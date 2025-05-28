#ifndef __METADATA_CONSTANTS_H
#define __METADATA_CONSTANTS_H

#include <string>

namespace dbcore::metadata::detail
{
    // table
    const std::string fields_catalog_tbl = "fldcat";
    const std::string table_catalog_tbl = "tblcat";

    const std::string tblname_field = "tblname";
    const std::string slotsize_field = "slotsize";
    const std::string type_field = "type";
    const std::string length_field = "length";
    const std::string offset_field = "offset";
    const std::string fldname_field = "fldname";

    // view
    const std::string view_catalog_tbl = "viewcat";

    const std::string viewname_field = "viewname";
    const std::string viewdef_field = "viewdef";
}

#endif