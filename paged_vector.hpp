#ifndef _paged_vector_hpp_
#define _paged_vector_hpp_

/*
	Paged-Vector implementation 
	Autor	: Dimitris Vlachos
	Email	: DimitrisV22@gmail.com
	Git		: http://github.com/DimitrisVlachos
	Licence	: MIT
*/

#include <stdint.h>
#include <algorithm>

template				<typename base_t,							/*Base type/object*/
						const uint32_t k_bank_bit_range = 14U,		/*Page Size = 2 ^ k_bank_bit_range - 1*/
						const uint32_t k_min_page_count = 1U,			/*During reallocs allocate at least k_min_page_count*/
						const uint32_t k_elements_per_page = (1U << k_bank_bit_range),	/* XXX Do not touch*/
						const uint32_t k_page_mod = k_elements_per_page - 1U,			 /* XXX Do not touch*/
						const uint32_t k_page_shr = k_bank_bit_range>				 /* XXX Do not touch*/
class paged_vector {
    public:
    paged_vector() : m_page_cnt(k_min_page_count),m_page_ptr(0U),m_elements(0U) {
        m_page = new base_t*[m_page_cnt];
        for (uint32_t i = 0U;i < m_page_cnt;++i) {
            m_page[i] = new base_t[k_elements_per_page];
        }

        m_active_page = m_page[0U];
    }

    ~paged_vector() {
        for (uint32_t i = 0U,j = m_page_cnt;i < j;++i) {
            delete[] m_page[i];
        }
        delete[] m_page;
    }

    inline paged_vector& operator=(paged_vector& in) {
        base_t** p;
        uint32_t cnt,block;
        uint32_t i,j;

        if (&in == this) {
            return *this;
        }

        p = in.__get_page_block_private();
        cnt = in.size();
        clear();
        reserve(cnt);
        block = 0U;
        i = 0U;
        j = cnt;

        //Whole blocks ( l >> k_page_shr)
        for (;(i + k_elements_per_page) <= j;i += k_elements_per_page,++block,++m_page_ptr) {
            std::copy(&p[block][0U],&p[block][k_elements_per_page],m_page[m_page_ptr]);
        }

        //Mod remainder (l & k_page_mod)
        if (i < j) {
            std::copy(&p[block][0U],&p[block][(j & k_page_mod)],m_page[m_page_ptr]);
        }

        m_elements = cnt;
        return *this;
    }

    inline void clear() { //Memory is only cleared during dtor call
        m_page_ptr = 0U;
        m_elements = 0U;
        m_active_page = m_page[0U];
    }

    inline void erase(uint32_t offset) {
        if ((0U == m_elements) || (offset > m_elements)) {
            m_page_ptr = 0U;
            m_active_page = m_page[0U];
            return;
        } else if ((m_elements-1U) == offset) {  
            pop_back();
            return;
        }

        //TODO block-copy page + remainder
        register base_t** p = m_page;
        for (register uint32_t i = offset,j = m_elements - 1U;i < j;++i) {
            p[i >> k_page_shr][i & k_page_mod] = p[(i + 1U) >> k_page_shr][(i + 1U) & k_page_mod];
        }
        --m_elements;

        const uint32_t page = m_elements >> k_page_shr;
        if (page == m_page_ptr) {
            return;
        }
        m_page_ptr = page;
        m_active_page = m_page[m_page_ptr];
    }

    inline base_t& at(uint32_t offset) {
        return m_page[offset >> k_page_shr][offset & k_page_mod];
    }

    inline const base_t& at(uint32_t offset) const {
        return m_page[offset >> k_page_shr][offset & k_page_mod];
    }

    //Lock()'s are usefull for multi threading operations - ie each thread can only access an explicit set of pages
    inline base_t& at(uint32_t page,uint32_t offset) {
        const uint32_t cp = offset >> k_page_shr;
        return m_page[cp & ((page != 0U) ? page-1U : 0U)][offset & k_page_mod];
    }

    inline const base_t& at(uint32_t page,uint32_t offset) const {
        const uint32_t cp = offset >> k_page_shr;
        return m_page[cp & ((page != 0U) ? page-1U : 0U)][offset & k_page_mod];
    }

    inline const base_t& operator[](uint32_t offset) const {
        return m_page[offset >> k_page_shr][offset & k_page_mod];
    }

    inline base_t& operator[](uint32_t offset) {
        return m_page[offset >> k_page_shr][offset & k_page_mod];
    }

    inline void reserve(uint32_t elements) {
        const uint32_t r = (elements + k_elements_per_page) & (~k_page_mod);
        const uint32_t d = (r >> k_page_shr) + 1U + (r != 0U);
        const uint32_t sptr = m_page_ptr;

        if (m_page_cnt > d) {
            return;
        }

        for (uint32_t c = 0;c <= d;++c) {
            add_page();
        }

        m_page_ptr = sptr;
    }

    inline void push_back(const base_t& in) {
        const uint32_t page = m_elements >> k_page_shr;

        if (page != m_page_ptr) { //1% of the time
        	add_page();
        	m_active_page = m_page[m_page_ptr];
        }

		*(m_active_page++) = in;
		++m_elements;
    }

    inline base_t& back() {
        return (m_elements != 0U) ? this->at(m_elements - 1U) : this->at(0U);
    }

    inline const base_t& back() const {
        return (m_elements != 0U) ? this->at(m_elements - 1U) : this->at(0U);
    }

    inline void pop_back() {
        m_elements -= m_elements != 0U;
        const uint32_t page = m_elements >> k_page_shr;
        m_page_ptr = page;
        m_active_page = m_page[m_page_ptr];
    }

    inline const uint32_t size() const {
        return m_elements;
    }

    inline const uint32_t pages() const {
        return m_page_ptr + (1U - (uint32_t)empty());
    }

    inline const bool empty() const {
        return m_elements == 0U;
    }

    inline base_t* get_page_block(const uint32_t root = 0U) {
        return m_page[(root >= pages()) ? 0U : root];
    }

    private:

    inline base_t** __get_page_block_private() {
        return m_page;
    }

    inline void add_page() {
        const uint32_t scnt = m_page_cnt;
        base_t** temp;

        if ((++m_page_ptr) < m_page_cnt) {
            return;
        }

        m_page_cnt = m_page_ptr + k_min_page_count;
        temp = new base_t*[m_page_cnt];

        for (uint32_t i = 0U,j = scnt;i < j;++i) {
            temp[i] = m_page[i];
        }

        for (uint32_t i = scnt,j = m_page_cnt;i < j;++i) {
            temp[i] = new base_t[k_elements_per_page];
        }

        delete[] m_page;
        m_page = temp;
    }


    base_t** m_page;
    base_t* m_active_page;
    uint32_t m_page_cnt,m_page_ptr,m_elements;
};
#endif
