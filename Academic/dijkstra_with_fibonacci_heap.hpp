#include "./header.hpp"

template<typename _Key, typename _Tp> class FibonacciHeap
{
public:
    using data_type = pair<_Key, _Tp>;
    class node
    {
    public:
        data_type _data;
        unsigned short int _child;
        bool _mark;
        node *_par, *_prev, *_next, *_ch_last;
        node(data_type&& data) : _data(move(data)), _child(0), _mark(false),
            _par(nullptr), _prev(nullptr), _next(nullptr), _ch_last(nullptr){}
        inline const _Key& get_key() const noexcept { return _data.first; }
        void insert(node *cur){
            if(_ch_last) insert_impl(cur, _ch_last);
            else _ch_last = cur, _ch_last->_prev = _ch_last->_next = _ch_last;
            ++_child, cur->_par = this;
        }
        void erase(node *cur){
            if(cur == cur->_prev){
                _ch_last = nullptr;
            }else{
                erase_impl(cur);
                if(cur == _ch_last) _ch_last = cur->_prev;
            }
            --_child, cur->_par = nullptr;
        }
    };

private:
    size_t _size;
    node *_minimum;
    vector<node*> rank;

    static void insert_impl(node *cur, node *next){
        cur->_prev = next->_prev, cur->_next = next;
        cur->_prev->_next = cur, next->_prev = cur;
    }
    static void erase_impl(node *cur){
        cur->_prev->_next = cur->_next, cur->_next->_prev = cur->_prev;
    }
    void root_insert(node *cur){
        if(_minimum){
            insert_impl(cur, _minimum);
            if(cur->get_key() < _minimum->get_key()) _minimum = cur;
        }else{
            _minimum = cur, _minimum->_prev = _minimum->_next = _minimum;
        }
    }
    void root_erase(node *cur){
        if(cur == cur->_prev) _minimum = nullptr;
        else erase_impl(cur);
    }
    void _delete(node *cur){
        root_erase(cur);
        delete cur;
    }
    template<typename Key, typename Data>
    node *_push(Key&& key, Data&& data){
        ++_size;
        data_type new_data(forward<Key>(key), forward<Data>(data));
        node* new_node = new node(move(new_data));
        root_insert(new_node);
        return new_node;
    }
    void _pop(){
        assert(_size > 0);
        --_size;
        if(_size == 0){
            _delete(_minimum);
            return;
        }
        if(_minimum->_ch_last){
            for(node *cur = _minimum->_ch_last->_next;;){
                node *next = cur->_next;
                _minimum->erase(cur), root_insert(cur);
                if(!_minimum->_ch_last) break;
                cur = next;
            }
        }
        node *next_minimum = _minimum->_next;
        for(node*& cur : rank) cur = nullptr;
        for(node *cur = next_minimum; cur != _minimum;){
            if(cur->get_key() < next_minimum->get_key()) next_minimum = cur;
            node *next = cur->_next;
            unsigned int deg = cur->_child;
            if(rank.size() <= deg) rank.resize(deg + 1, nullptr);
            while(rank[deg]){
                if(cur->get_key() < rank[deg]->get_key() || cur == next_minimum){
                    root_erase(rank[deg]), cur->insert(rank[deg]);
                }else{
                    root_erase(cur), rank[deg]->insert(cur);
                    cur = rank[deg];
                }
                rank[deg++] = nullptr;
                if(rank.size() <= deg) rank.resize(deg + 1, nullptr);
            }
            rank[deg] = cur;
            cur = next;
        }
        _delete(_minimum);
        _minimum = next_minimum;
    }
    void _decrease_key(node *cur, const _Key& key){
        assert(!(key < (_Key)0));
        node *change = ((cur->_data.first -= key) < _minimum->get_key()) ? cur : nullptr;
        if(!cur->_par || !(cur->get_key() < cur->_par->get_key())){
            if(change) _minimum = change;
            return;
        }
        while(true){
            node *next = cur->_par;
            next->erase(cur), root_insert(cur);
            cur->_mark = false, cur = next;
            if(!cur->_par) break;
            if(!cur->_mark){
                cur->_mark = true;
                break;
            }
        }
        if(change) _minimum = change;
    }
    void clear_dfs(node *cur){
        if(cur->_ch_last){
            for(node *_cur = cur->_ch_last->_next;;){
                node *next = _cur->_next;
                if(_cur == cur->_ch_last){
                    clear_dfs(_cur);
                    break;
                }else{
                    clear_dfs(_cur);
                }
                _cur = next;
            }
        }
        delete cur;
        return;
    }
    void _clear(){
        if(!_minimum) return;
        for(node *cur = _minimum->_next;;){
            node *next = cur->_next;
            if(cur == _minimum){
                clear_dfs(cur);
                break;
            }else{
                clear_dfs(cur);
            }
            cur = next;
        }
    }

public:
    FibonacciHeap() noexcept : _size(0u), _minimum(nullptr){}
    // ~FibonacciHeap(){ _clear(); }
    inline bool empty() const noexcept { return (_size == 0); }
    inline size_t size() const noexcept { return _size; }
    inline const data_type& top() const noexcept { return _minimum->_data; }
    template<typename Key, typename Data>
    node *push(Key&& key, Data&& data){ return _push(forward<Key>(key), forward<Data>(data)); }
    void pop(){ _pop(); }
    void decrease_key(node *cur, const _Key& key){ _decrease_key(cur, key); }
    void clear(){ _clear(); _size = 0; rank.~vector<node*>(); }
};

template<typename T> class Dijkstra {
public:
	struct edge{
		int to; T cost;
	};
    const int V;
	vector<vector<edge> > G;
	vector<T> d;
    FibonacciHeap<T, int> fheap;
    typename FibonacciHeap<T, int>::node** keep;
	Dijkstra(int node_size) : V(node_size), G(V), d(V, numeric_limits<T>::max()),
        keep(new typename FibonacciHeap<T, int>::node*[V]){}
    // ~Dijkstra(){
        // delete[] keep;
    // }
	// 有向グラフの場合
	void add_edge(int u,int v,T cost){
		G[u].pb((edge){v, cost});
	}
	void solve(int s){
		d[s] = 0;
		keep[s] = fheap.push(0, s);
		while(!fheap.empty()){
            int v = fheap.top().second;
            fheap.pop();
			for(auto& w : G[v]){
				if(d[w.to] > d[v] + w.cost){
                    if(d[w.to] == numeric_limits<T>::max()){
                        keep[w.to] = fheap.push(d[v] + w.cost, w.to);
                    }else{
                        fheap.decrease_key(keep[w.to], d[w.to] - (d[v] + w.cost));
                    }
                    d[w.to] = d[v] + w.cost;
				}
			}
		}
	}
};
