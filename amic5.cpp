#ifndef HASHMAP_H
#define HASHMAP_H

#include <algorithm>
#include <vector>
#include <list>
#include <memory>
#include <utility>
#include <list>
#include <initializer_list>
#include <iterator>

namespace hmap {


    static constexpr size_t TABLE_START_SZ = 1;

    template<class KeyType, class ValueType, class Hash = std::hash<KeyType> > class HashMap {
        struct MapElement {
            std::pair<const KeyType, ValueType> val;
            MapElement *next, *prev;
            size_t i;

            MapElement(std::pair<KeyType, ValueType> newVal, size_t index = 0, MapElement* nextEl = nullptr, MapElement* prevEl = nullptr) :
                val(newVal), next(nextEl), prev(prevEl), i(index) {}
        };

    public:

        template<typename T, typename ElT> class iteratorBase {

        public:
            T* val = nullptr;

            iteratorBase& operator++ () {
                val = val->next;
                return *this;
            }

            iteratorBase operator++ (int) {
                iteratorBase prevVal = *this;
                val = val->next;
                return prevVal;
            }

            ElT& operator* () const {
                return val->val;
            }

            ElT* operator-> () const {
                return &(val->val);
            }

            bool operator== (const iteratorBase& other) const noexcept {
                return val == other.val;
            }

            bool operator!= (const iteratorBase& other) const noexcept {
                return val != other.val;
            }

        };

        class const_iterator : public iteratorBase<const MapElement, const std::pair<const KeyType, ValueType>> {};
        class iterator : public iteratorBase<MapElement, std::pair<const KeyType, ValueType>> {};

    private:
        std::vector<std::list<MapElement>> hashArray;
        Hash hasher;
        size_t sz = 0;
        const_iterator cSt, cFin;
        iterator st, fin;
        size_t tableSize = TABLE_START_SZ;


    public:
        constexpr HashMap(const Hash& hasher_obj = Hash()) : hashArray(TABLE_START_SZ), hasher(hasher_obj) {}

        template<typename Ptr>
        HashMap(Ptr first, Ptr last, const Hash& hasher_obj = Hash()) :
            hasher(hasher_obj) {
            MapElement* prev = nullptr;

            Ptr curPtr = first;
            size_t cntElems = 0;
            while (curPtr != last) {
                ++cntElems;
                ++curPtr;
            }
            tableSize = 1;
            while (tableSize < cntElems) {
                tableSize *= 2;
            }
            hashArray.assign(tableSize);
            while (first != last) {
                size_t curHash = hasher(first->first) % tableSize;
                hashArray[curHash].push_back(MapElement(*first, curHash));
                if (sz > 0) {
                    hashArray[curHash].back().prev = prev;
                    prev->next = &hashArray[curHash].back();
                } else
                    st.val = &hashArray[curHash].back(), cSt.val = &hashArray[curHash].back();
                prev = &hashArray[curHash].back();
                ++sz;
                ++first;
            }
            fin.val = prev, cFin.val = prev;
        }

        HashMap(const std::initializer_list<std::pair<const KeyType, ValueType>>& elements, const Hash& hasher_obj = Hash()) :
            HashMap(elements.begin(), elements.end(), hasher_obj) {}

        HashMap& operator= (const HashMap& other) {
            for (auto cur = other.begin(); cur != other.end(); ++cur) {
                insert(*cur);
            }
            return *this;
        }


        void autoResize() {
            size_t tableSizeOld = tableSize;
            while (tableSize >= sz * 2) {
                tableSize /= 2;
            }
            while (tableSize < sz) {
                tableSize *= 2;
            }

            if (tableSize == tableSizeOld)
                return;

            std::vector<std::list<MapElement>> hashArrayOld = std::move(hashArray);
            st.val = fin.val = cSt.val = cFin.val = nullptr;
            hashArray.assign(tableSize);
            for (const auto& x : hashArrayOld) {
                for (const auto& y : x) {
                    *this->insert(y->val);
                }
            }
        }


        const size_t size() const noexcept {
            return sz;
        }

        const bool empty() const noexcept {
            return sz == 0;
        }

        void insert(std::pair<const KeyType, ValueType> elem) {
            size_t curHash = hasher(elem.first) % tableSize;
            for (const auto& x : hashArray[curHash]) {
                if (x.val.first == elem.first) {
                    return;
                }
            }
            ++sz;
            hashArray[curHash].push_back(MapElement(elem, curHash));
            if (fin.val != nullptr) {
                fin.val->next = &hashArray[curHash].back();
                hashArray[curHash].back().prev = fin.val;
            } else {
                st.val = &hashArray[curHash].back();
                cSt.val = &hashArray[curHash].back();
            }
            fin.val = &hashArray[curHash].back();
            cFin.val = &hashArray[curHash].back();

            autoResize();
        }

        void erase(const KeyType key) {
            size_t curHash = hasher(key) % tableSize;
            for (auto x = hashArray[curHash].begin(); x != hashArray[curHash].end(); ++x) {
                if (x->val.first == key) {
                    if (&(*x) == fin.val)
                        fin.val = x->prev;
                    cFin.val = x->prev;
                    if (x->prev != nullptr) {
                        x->prev->next = x->next;
                    }
                    if (x->next != nullptr) {
                        x->next->prev = x->prev;
                    }
                    if (&(*x) == st.val) {
                        st.val = nullptr;
                        cSt.val = nullptr;
                    }
                    hashArray[curHash].erase(x);
                    --sz;
                    break;
                }
            }

            autoResize();
        }

        const_iterator find(const KeyType key) const {
            size_t curHash = hasher(key) % tableSize;
            for (const auto& x : hashArray[curHash]) {
                if (x.val.first == key) {
                    const_iterator answ;
                    answ.val = &x;
                    return answ;
                }
            }
            return const_iterator();
        }

        iterator find(const KeyType key) {
            size_t curHash = hasher(key) % tableSize;
            for (auto& x : hashArray[curHash]) {
                if (x.val.first == key) {
                    iterator answ;
                    answ.val = &x;
                    return answ;
                }
            }
            return iterator();
        }

        ValueType& operator[] (const KeyType key) {
            size_t curHash = hasher(key) % tableSize;
            for (auto& x : hashArray[curHash]) {
                if (x.val.first == key) {
                    return x.val.second;
                }
            }
            insert({ key, ValueType() });
            return hashArray[curHash].back().val.second;
        }

        const ValueType& at(const KeyType key) const {
            size_t curHash = hasher(key) % tableSize;
            for (const auto& x : hashArray[curHash]) {
                if (x.val.first == key) {
                    return x.val.second;
                }
            }
            throw std::out_of_range("out of range");
        }

        iterator begin() {
            return st;
        }

        const_iterator begin() const {
            return cSt;
        }

        iterator end() {
            return iterator();
        }

        const_iterator end() const {
            return const_iterator();
        }

        void clear() {
            hashArray.assign(1);
            tableSize = 1;
            st.val = nullptr, fin.val = nullptr, cSt.val = nullptr, cFin.val = nullptr;
            sz = 0;
        }

        Hash hash_function() const {
            return hasher;
        }
    };

}

#endif // !HASHMAP_H