#include "hash_index.hpp"

namespace dbcore::index
{
    hash_index::hash_index(std::shared_ptr<tx::transaction> tx, const std::string& idxname, const record::layout& layout)
        : m_buckets_num(100), m_tx(tx), m_idxname(idxname), m_layout(layout)
    {}
}