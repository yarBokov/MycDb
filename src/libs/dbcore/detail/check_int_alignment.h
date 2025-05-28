#ifndef __LIB_CORE_CHECK_INT_ALIGN_H
#define __LIB_CORE_CHECK_INT_ALIGN_H

namespace dbcore
{
    bool check_int_alignment(int offset)
    {
        return offset % alignof(int) == 0;
    }
}

#endif