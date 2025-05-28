#ifndef __SCAN_SCAN_INTERFACE_H
#define __SCAN_SCAN_INTERFACE_H

#include <string>
#include "libs/dbcore/query/constant.h"

namespace dbcore::scan
{
    class i_scan
    {
        public:
            virtual ~i_scan() = default;
    
            virtual void before_first() = 0;
            virtual bool next() = 0;
            virtual int get_int(const std::string& fldname) = 0;
            virtual std::string get_str(const std::string& fldname) = 0;
            virtual query::constant get_val(const std::string& fldname) = 0;
            virtual bool has_field(const std::string& fldname) = 0;
            virtual void close() = 0;
    };
}

#endif