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
    static constexpr int TABLESZ_DIV_SZ = 2;

    template<class KeyType, class ValueType, class Hash = std::hash<KeyType> > class HashMap {
        typedef std::pair<const KeyType, ValueType> MapElement;

    public:
        using const_iterator = typename std::list<MapElement>::const_iterator;
        using iterator = typename std::list<MapElement>::iterator;

    private:
        std::vector<std::list<std::pair<const_iterator, iterator>>> hashArray;
        Hash hasher;
        size_t sz = 0;
        std::list<MapElement> elemArray;
        size_t tableSize = TABLE_START_SZ;

    public:
        constexpr explicit HashMap(const Hash& hasherObj = Hash()) : hashArray(TABLE_START_SZ), hasher(hasherObj) {}

        template<typename ForwardIt>
        HashMap(ForwardIt first, ForwardIt last, const Hash& hasherObj = Hash()) :
            HashMap(hasherObj) {
            while (first != last) {
                this->insert(*first);
                ++first;
            }
        }

        HashMap(const std::initializer_list<std::pair<const KeyType, ValueType>>& elements, const Hash& hasherObj = Hash()) :
            HashMap(elements.begin(), elements.end(), hasherObj) {}


        void autoResize() {
            size_t tableSizeOld = tableSize;
            while (tableSize >= TABLESZ_DIV_SZ * 2 && tableSize >= sz * TABLESZ_DIV_SZ * 2) {
                tableSize /= 2;
            }
            while (tableSize < sz * TABLESZ_DIV_SZ / 2) {
                tableSize *= 2;
            }

            if (tableSize == tableSizeOld)
                return;
            hashArray.assign(tableSize, std::list<std::pair<const_iterator, iterator>>());
            std::list<MapElement> elemArrayOld = std::move(elemArray);
            elemArray.clear();
            sz = 0;
            for (const auto& x : elemArrayOld) {
                this->insert(x, false);
            }
        }


        const size_t size() const noexcept {
            return sz;
        }

        const bool empty() const noexcept {
            return sz == 0;
        }

        void insert(std::pair<const KeyType, ValueType> elem, bool needAR = true) {
            size_t curHash = hasher(elem.first) % tableSize;
            iterator it = this->find(elem.first);
            if (it == elemArray.end()) {
                ++sz;
                elemArray.push_back(elem);
                hashArray[curHash].push_back({ --elemArray.cend(), --elemArray.end() });
                if (needAR)
                    autoResize();
            }
        }

        void erase(const KeyType key) {
            size_t curHash = hasher(key) % tableSize;
            for (auto xIt = hashArray[curHash].begin(); xIt != hashArray[curHash].end(); ++xIt) {
                if (xIt->first->first == key) {
                    elemArray.erase(xIt->second);
                    hashArray[curHash].erase(xIt);
                    --sz;

                    autoResize();
                    return;
                }
            }
        }

        const_iterator find(const KeyType key) const {
            size_t curHash = hasher(key) % tableSize;
            for (const auto& x : hashArray[curHash]) {
                if (x.first->first == key) {
                    return x.first;
                }
            }
            return elemArray.cend();
        }

        iterator find(const KeyType key) {
            size_t curHash = hasher(key) % tableSize;
            for (const auto& x : hashArray[curHash]) {
                if (x.first->first == key) {
                    return x.second;
                }
            }
            return elemArray.end();
        }

        ValueType& operator[] (const KeyType key) {
            iterator curIt = this->find(key);
            if (curIt != elemArray.end()) {
                return curIt->second;
            }

            this->insert(std::make_pair(key, ValueType()));
            return this->find(key)->second;
        }

        const ValueType& at(const KeyType key) const {
            const_iterator curIt = this->find(key);
            if (curIt != elemArray.cend()) {
                return curIt->second;
            }
            throw std::out_of_range("out of range");
        }

        iterator begin() noexcept {
            return elemArray.begin();
        }

        const_iterator begin() const noexcept {
            return elemArray.cbegin();
        }

        iterator end() noexcept {
            return elemArray.end();
        }

        const_iterator end() const noexcept {
            return elemArray.cend();
        }

        void clear() {
            hashArray.assign(TABLE_START_SZ, std::list<std::pair<const_iterator, iterator>>());
            tableSize = TABLE_START_SZ;
            elemArray.clear();
            sz = 0;
        }

        Hash hash_function() const {
            return hasher;
        }
    };

}

#endif // !HASHMAP_H