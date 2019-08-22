#include "../header.hpp"

template<class _Key, class _Hash, bool DOWNSIZE> class UnorderedSetIterator;

template<class _Key, class _Hash = hash<_Key>, bool DOWNSIZE = false>
class UnorderedSet
{
private:
    using iterator = UnorderedSetIterator<_Key, _Hash, DOWNSIZE>;
    using aligned_pointer = typename aligned_storage<sizeof(_Key), alignof(_Key)>::type;
    friend UnorderedSetIterator<_Key, _Hash, DOWNSIZE>;
    struct bucket {
        unsigned int _hash;
        short int _dist;
        bool _last, _end;
        aligned_pointer _key_ptr;
        bucket() noexcept : _dist(-1), _last(false), _end(false){}
        ~bucket(){ if(!empty()) _delete(); }
        inline void clear() noexcept { _dist = -1; }
        inline void _delete(){ _dist = -1, key_ptr()->~_Key(); }
        inline bool empty() const noexcept { return (_dist == -1); }
        inline _Key& key() noexcept {
            return *reinterpret_cast<_Key*>(&_key_ptr);
        }
        inline _Key* key_ptr() noexcept {
            return reinterpret_cast<_Key*>(&_key_ptr);
        }
        inline void new_key(const _Key& key){
            new(&_key_ptr) _Key(key);
        }
    };
    inline static unsigned int ceilpow2(unsigned int u) noexcept {
        --u, u |= u >> 1, u |= u >> 2, u |= u >> 4, u |= u >> 8;
        return (u | (u >> 16)) + 1;
    }
    inline static bucket *increment(bucket *cur) noexcept {
        for(++cur; !cur->_end; ++cur){
            if(!cur->empty()) break;
        }
        return cur;
    }
    inline bucket *next_bucket(bucket*& cur) const noexcept {
        return cur->_last ? _buckets : cur + 1;
    }
    inline unsigned int make_hash(const _Key& key) const noexcept {
        return _Hash()(key);
    }
    inline float load_factor() const noexcept {
        return (float)_data_count / _bucket_count;
    }
    bucket *insert_impl(bucket *cur, unsigned int hash, short int dist, _Key key){
        bucket *ret = cur;
        bool flag = false;
        while(true){
            if(cur->empty()){
                cur->_hash = hash, cur->_dist = dist, cur->new_key(key);
                if(!flag) ret = cur, flag = true;
                break;
            }else if(dist > cur->_dist){
                swap(hash, cur->_hash), swap(dist, cur->_dist), swap(key, cur->key());
                if(!flag) ret = cur, flag = true;
            }
            ++dist;
            cur = next_bucket(cur);
        }
        return ret;
    }
    bucket *_find(const _Key& key){
        unsigned int hash = make_hash(key);
        bucket *cur = _buckets + (hash & _mask);
        short int dist = 0;
        while(dist <= cur->_dist){
            if(hash == cur->_hash && key == cur->key()){
                return cur;
            }
            ++dist;
            cur = next_bucket(cur);
        }
        return _buckets + _bucket_count;
    }
    bucket *_insert(const _Key& key){
        unsigned int hash = make_hash(key);
        bucket *cur = _buckets + (hash & _mask);
        short int dist = 0;
        while(dist <= cur->_dist){
            if(hash == cur->_hash && key == cur->key()){
                return cur;
            }
            ++dist;
            cur = next_bucket(cur);
        }
        ++_data_count;
        if(rehash_check()){
            hash = make_hash(key), cur = _buckets + (hash & _mask), dist = 0;
        }
        return insert_impl(cur, hash, dist, key);
    }
    bucket *backward_shift(bucket *cur, bool next_ret){
        bucket *next = next_bucket(cur), *ret = cur;
        if(next->_dist < 1) return next_ret ? increment(cur) : cur;
        do {
            cur->_hash = next->_hash;
            cur->_dist = next->_dist - 1;
            cur->_key_ptr = next->_key_ptr;
            cur = next, next = next_bucket(cur);
        }while(next->_dist >= 1);
        cur->clear();
        return ret;
    }
    bucket *erase_impl(bucket *cur, bool next_ret){
        assert(cur != _buckets + _bucket_count);
        cur->_delete();
        return backward_shift(cur, next_ret);
    }
    bucket *erase_itr(bucket *cur, bool next_ret = true){
        --_data_count;
        const _Key& key = cur->key();
        return erase_impl(rehash_check() ? _find(key) : cur, next_ret);
    }
    bucket *erase_key(const _Key& key, bool next_ret = true){
        --_data_count;
        rehash_check();
        return erase_impl(_find(key), next_ret);
    }
    bool rehash_check(){
        if(load_factor() >= MAX_LOAD_FACTOR){
            rehash(_bucket_count * 2);
            return true;
        }else if(DOWNSIZE){
            if(load_factor() <= MIN_LOAD_FACTOR && _bucket_count >= DOWNSIZE_THRESHOLD){
                rehash(_bucket_count / 2);
                return true;
            }else{
                return false;
            }
        }else{
            return false;
        }
    }
    void move_data(aligned_pointer key_ptr){
        unsigned int hash = make_hash(*reinterpret_cast<_Key*>(&key_ptr));
        bucket *cur = _buckets + (hash & _mask);
        short int dist = 0;
        while(true){
            if(cur->empty()){
                cur->_hash = hash, cur->_dist = dist, cur->_key_ptr = key_ptr;
                break;
            }else if(dist > cur->_dist){
                swap(hash, cur->_hash), swap(dist, cur->_dist), swap(key_ptr, cur->_key_ptr);
            }
            ++dist;
            cur = next_bucket(cur);
        }
    }
    void rehash(unsigned int new_bucket_count){
        UnorderedSet new_unordered_set(new_bucket_count);
        new_unordered_set._data_count = _data_count;
        for(bucket *cur = _buckets; !cur->_end; ++cur){
            if(!cur->empty()){
                new_unordered_set.move_data(cur->_key_ptr);
                cur->clear();
            }
        }
        swap(*this, new_unordered_set);
    }
    friend void swap(UnorderedSet& ust1, UnorderedSet& ust2){
        swap(ust1._bucket_count, ust2._bucket_count);
        swap(ust1._mask, ust2._mask);
        swap(ust1._data_count, ust2._data_count);
        swap(ust1._buckets, ust2._buckets);
    }

private:
    unsigned int _bucket_count, _mask, _data_count;
    bucket *_buckets;
public:
    const float MAX_LOAD_FACTOR = 0.5f;
    const float MIN_LOAD_FACTOR = 0.1f;
    const unsigned int DOWNSIZE_THRESHOLD = 16u;
    UnorderedSet(unsigned int bucket_size = 1u)
     : _bucket_count(ceilpow2(max(bucket_size, 1u))), _mask(_bucket_count - 1),
        _data_count(0u), _buckets(new bucket[_bucket_count + 1]){
        _buckets[_bucket_count - 1]._last = true, _buckets[_bucket_count]._end = true;
    }
    ~UnorderedSet(){ delete[] _buckets; }
    friend ostream& operator<< (ostream& os, UnorderedSet& ust) noexcept {
        for(_Key& val : ust) os << val << " ";
        return os;
    }
    void clear(){
        UnorderedSet new_unordered_set(1u);
        swap(*this, new_unordered_set);
    }
    size_t size() const noexcept { return _data_count; }
    size_t bucket_count() const noexcept { return _bucket_count; }
    bool empty() const noexcept { return (_data_count == 0); }
    iterator begin() const noexcept { return _buckets->empty() ? iterator(increment(_buckets)) : iterator(_buckets); }
    iterator end() const noexcept { return iterator(_buckets + _bucket_count); }
    iterator find(const _Key& key){ return iterator(_find(key)); }
    size_t count(const _Key& key){ return (_find(key) != _buckets + _bucket_count); }
    iterator insert(const _Key& key){ return iterator(_insert(key)); }
    iterator erase(const _Key& key){ return iterator(erase_key(key)); }
    iterator erase(const iterator& itr){ return iterator(erase_itr(itr.bucket_ptr)); }
    void simple_erase(const _Key& key){ erase_key(key, false); }
    void simple_erase(const iterator& itr){ erase_itr(itr.bucket_ptr, false); }

    // DEBUG 用
    short int maximum_distance() const noexcept {
        short int ret = -1;
        for(bucket *cur = _buckets; !cur->_end; ++cur){
            ret = max(ret, cur->_dist);
        }
        return ret;
    }
};

template<class _Key, class _Hash, bool DOWNSIZE>
class UnorderedSetIterator {
private:
    friend UnorderedSet<_Key, _Hash, DOWNSIZE>;
    typename UnorderedSet<_Key, _Hash, DOWNSIZE>::bucket *bucket_ptr;
    using iterator_category = forward_iterator_tag;
    using value_type = _Key;
    using difference_type = ptrdiff_t;
    using pointer = _Key*;
    using reference = _Key&;

private:
    UnorderedSetIterator(typename UnorderedSet<_Key, _Hash, DOWNSIZE>::bucket *_bucket_ptr)
        noexcept : bucket_ptr(_bucket_ptr){}
public:
    UnorderedSetIterator() noexcept : bucket_ptr(){}
    UnorderedSetIterator(const UnorderedSetIterator& itr) noexcept : bucket_ptr(itr.bucket_ptr){}
    UnorderedSetIterator& operator=(const UnorderedSetIterator& itr)
        & noexcept { return bucket_ptr = itr.bucket_ptr, *this; }
    UnorderedSetIterator& operator=(const UnorderedSetIterator&& itr)
        & noexcept { return bucket_ptr = itr.bucket_ptr, *this; }
    reference operator*() const noexcept { return bucket_ptr->key(); }
    pointer operator->() const noexcept { return bucket_ptr->key_ptr(); }
    UnorderedSetIterator& operator++() noexcept {
        return bucket_ptr = UnorderedSet<_Key, _Hash, DOWNSIZE>::increment(bucket_ptr), *this;
    }
    UnorderedSetIterator operator++(int) const noexcept {
        return UnorderedSetIterator(UnorderedSet<_Key, _Hash, DOWNSIZE>::increment(this->bucket_ptr));
    }
    bool operator==(const UnorderedSetIterator& itr) const noexcept { return !(*this != itr); };
    bool operator!=(const UnorderedSetIterator& itr) const noexcept { return bucket_ptr != itr.bucket_ptr; }
};