#include "../header.hpp"

template<class _Key> class SetIterator;

template<class _Key> class Set {
private:
    using iterator = SetIterator<_Key>;
    using data_type = const _Key;
    struct node {
        data_type _key;
        node *_M_left, *_M_right, *_M_parent;
        node(const data_type& key) noexcept
            : _key(key), _M_left(nullptr), _M_right(nullptr), _M_parent(nullptr){}
        inline bool isRoot() const noexcept { return !_M_parent; }
        void rotate(const bool right) noexcept {
            node *p = _M_parent, *g = p->_M_parent;
            if(right){
                if((p->_M_left = _M_right)) _M_right->_M_parent = p;
                _M_right = p, p->_M_parent = this;
            }else{
                if((p->_M_right = _M_left)) _M_left->_M_parent = p;
                _M_left = p, p->_M_parent = this;
            }
            if(!(_M_parent = g)) return;
            if(g->_M_left == p) g->_M_left = this;
            if(g->_M_right == p) g->_M_right = this;
        }
    };
    friend SetIterator<_Key>;
    node *root, *_M_header, *start;
    size_t _M_node_count;
    node *splay(node *u) noexcept {
        while(!(u->isRoot())){
            node *p = u->_M_parent, *gp = p->_M_parent;
            if(p->isRoot()){
                u->rotate((u == p->_M_left));
            }else{
                bool flag = (u == p->_M_left);
                if((u == p->_M_left) == (p == gp->_M_left)){
                    p->rotate(flag), u->rotate(flag);
                }else{
                    u->rotate(flag), u->rotate(!flag);
                }
            }
        }
        return root = u;
    }
    static node *increment(node *ver) noexcept {
        if(ver->_M_right){
            ver = ver->_M_right;
            while(ver->_M_left) ver = ver->_M_left;
        }else{
            node *nx = ver->_M_parent;
            while(ver == nx->_M_right) ver = nx, nx = nx->_M_parent;
            ver = nx;
        }
        return ver;
    }
    static node *decrement(node *ver) noexcept {
        if(ver->_M_left){
            ver = ver->_M_left;
            while(ver->_M_right) ver = ver->_M_right;
        }else{
            node *nx = ver->_M_parent;
            while(ver == nx->_M_left) ver = nx, nx = nx->_M_parent;
            ver = nx;
        }
        return ver;
    }
    node *join(node *ver1, node *ver2, const node *ver) noexcept {
        while(ver2->_M_left) ver2 = ver2->_M_left;
        splay(ver2)->_M_left = ver1;
        return ver1 ? (ver1->_M_parent = ver2) : (start = ver2);
    }
    node *_find(const data_type& key) noexcept {
        node *cur = nullptr, *nx = root;
        do {
            cur = nx;
            if(cur == _M_header || cur->_key > key) nx = cur->_M_left;
            else if(cur->_key < key) nx = cur->_M_right;
            else return splay(cur);
        }while(nx);
        return _M_header;
    }
    node *_insert(const data_type& key) noexcept {
        node *cur = nullptr, *nx = root;
        do {
            cur = nx;
            if(cur == _M_header || cur->_key > key) nx = cur->_M_left;
            else if(cur->_key < key) nx = cur->_M_right;
            else return splay(cur);
        }while(nx);
        if(cur == _M_header || cur->_key > key){
            nx = new node(key);
            cur->_M_left = nx, nx->_M_parent = cur;
            if(cur == start) start = nx;
            return _M_node_count++, splay(nx);
        }else{
            nx = new node(key);
            cur->_M_right = nx, nx->_M_parent = cur;
            return _M_node_count++, splay(nx);
        }
    }
    node *_erase(node *root_ver){
        assert(root_ver != _M_header);
        if(root_ver->_M_left) root_ver->_M_left->_M_parent = nullptr;
        if(root_ver->_M_right) root_ver->_M_right->_M_parent = nullptr;
        node *res = join(root_ver->_M_left, root_ver->_M_right, root_ver);
        delete root_ver;
        return _M_node_count--, res;
    }
    node *_erase(const data_type& key){
        node *ver = _find(key);
        return _erase(ver);
    }
    node *_lower_bound(const data_type& key) noexcept {
        node *cur = nullptr, *nx = root, *res = nullptr;
        do {
            cur = nx;
            if(cur == _M_header){ nx = cur->_M_left; continue; }
            else if(cur->_key >= key){
                nx = cur->_M_left;
                if(!res || cur->_key <= res->_key) res = cur;
            }else nx = cur->_M_right;
        }while(nx);
        splay(cur);
        return res ? res : _M_header;
    }
    node *_upper_bound(const data_type& key) noexcept {
        node *cur = nullptr, *nx = root, *res = nullptr;
        do {
            cur = nx;
            if(cur == _M_header){ nx = cur->_M_left; continue; }
            else if(cur->_key > key){
                nx = cur->_M_left;
                if(!res || cur->_key <= res->_key) res = cur;
            }else nx = cur->_M_right;
        }while(nx);
        splay(cur);
        return res ? res : _M_header;
    }
    void clear_dfs(node *cur) noexcept {
        if(cur->_M_left) clear_dfs(cur->_M_left);
        if(cur->_M_right) clear_dfs(cur->_M_right);
        delete cur;
    }

public:
    Set() noexcept : _M_node_count(0){ root = _M_header = start = new node(_Key()); }
    // ~Set() noexcept { if(root) clear_dfs(root); }
    friend ostream& operator<< (ostream& os, Set& st) noexcept {
        for(auto& val : st) os << val << " ";
        return os;
    }
    size_t size() const noexcept { return _M_node_count; }
    bool empty() const noexcept { return size() == 0; }
    iterator begin() const noexcept { return iterator(start); }
    iterator end() const noexcept { return iterator(_M_header); }
    void clear() noexcept { clear_dfs(root), _M_node_count = 0, root = _M_header = start = new node(_Key()); }
    iterator find(const data_type& key) noexcept { return iterator(_find(key)); }
    size_t count(const _Key& key){ return (_find(key) != _M_header); }
    iterator insert(const data_type& key) noexcept { return iterator(_insert(key)); }
    iterator erase(const data_type& key){ return iterator(_erase(key)); }
    iterator erase(const iterator& itr){ return iterator(_erase(splay(itr.node_ptr))); }
    iterator lower_bound(const data_type& key) noexcept { return iterator(_lower_bound(key)); }
    iterator upper_bound(const data_type& key) noexcept { return iterator(_upper_bound(key)); }
};

template<class _Key>
class SetIterator {
private:
    friend Set<_Key>;
    typename Set<_Key>::node *node_ptr;
    using iterator_category = bidirectional_iterator_tag;
    using value_type = const _Key;
    using difference_type = const _Key;
    using pointer = const _Key*;
    using reference = const _Key&;

private:
    SetIterator(typename Set<_Key>::node *st) noexcept : node_ptr(st){}

public:
    SetIterator() noexcept : node_ptr(){}
    SetIterator(const SetIterator& itr) noexcept : node_ptr(itr.node_ptr){}
    SetIterator& operator=(const SetIterator& itr) & noexcept { return node_ptr = itr.node_ptr, *this; }
    SetIterator& operator=(const SetIterator&& itr) & noexcept { return node_ptr = itr.node_ptr, *this; }
    reference operator*() const { return node_ptr->_key; }
    pointer operator->() const { return &(node_ptr->_key); }
    SetIterator& operator++() noexcept { return node_ptr = Set<_Key>::increment(node_ptr), *this; }
    SetIterator operator++(int) const noexcept { return SetIterator(Set<_Key>::increment(this->node_ptr)); }
    SetIterator& operator--() noexcept { return node_ptr = Set<_Key>::decrement(node_ptr), *this; }
    SetIterator operator--(int) const noexcept { return SetIterator(Set<_Key>::decrement(this->node_ptr)); }
    bool operator==(const SetIterator& itr) const noexcept { return !(*this != itr); };
    bool operator!=(const SetIterator& itr) const noexcept { return node_ptr != itr.node_ptr; }
};