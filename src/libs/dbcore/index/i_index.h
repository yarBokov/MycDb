#ifndef __INDEX_IDX_I_H
#define __INDEX_IDX_I_H

#include "libs/dbcore/record/record_id.h"
#include "libs/dbcore/query/constant.h"

namespace dbcore::index
{
    class i_index
    {
        public:
            virtual ~i_index() = default;
    
            virtual void before_first(const query::constant& searchkey) = 0;
            virtual bool next() = 0;
            virtual record::record_id get_data_rid() = 0;
            virtual void insert(const query::constant& dataval, const record::record_id& datarid) = 0;
            virtual void delete_record(const query::constant& dataval, const record::record_id& datarid) = 0;
            virtual void close() = 0;
    };
}

#endif