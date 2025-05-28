#ifndef __QUERY_QUERYPLANNER_H
#define __QUERY_QUERYPLANNER_H

#include "plan.h"
#include "query_data.h"

namespace dbcore
{
    class query_planner
    {
        public:
            virtual plan create_plan(const query_data& data, const );
    };
}

#endif