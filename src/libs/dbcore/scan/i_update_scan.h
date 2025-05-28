#ifndef __SCAN_UPDATESCAN_H
#define __SCAN_UPDATESCAN_H

#include "i_scan.h"
#include "libs/dbcore/record/record_id.h"

namespace dbcore::scan
{
    class i_update_scan : public i_scan
    {
        public:
            virtual void set_val(const std::string& fldname, const query::constant& val) = 0;
            virtual void set_int(const std::string& fldname, int val) = 0;
            virtual void set_str(const std::string& fldname, const std::string& val) = 0;
            virtual void insert() = 0;
            virtual void delete_record() = 0;
            virtual record::record_id get_rid() = 0;
            virtual void move_to_rid(const record::record_id& rid) = 0;
    };
}

#endif