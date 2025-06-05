#ifndef __MULTIBUFFER_BUFFER_NEEDS_H
#define __MULTIBUFFER_BUFFER_NEEDS_H

#include <cmath>
#include <climits>

namespace dbcore::multibuffer
{
    struct buffer_needs
    {
        public:
            static std::size_t best_root(int available, int size)
            {
                int avail = available - 2;
                if (avail <= 1)
                    return 1;
                
                std::size_t k = INT_MAX;
                double i = 1.0;
                while (k > avail)
                {
                    i++;
                    k = static_cast<std::size_t>(std::ceil(std::pow(size, 1.0 / i)));
                }
                return k;
            }

            static std::size_t best_factor(int available, int size)
            {
                int avail = available - 2;
                if (avail <= 1)
                    return 1;

                std::size_t k = size;
                double i = 1.0;
                while (k > avail)
                {
                    i++;
                    k = static_cast<std::size_t>(std::ceil(size / i));
                }
                return k;
            }
    };
}

#endif