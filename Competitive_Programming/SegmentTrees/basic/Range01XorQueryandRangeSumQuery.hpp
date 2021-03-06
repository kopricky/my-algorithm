#include "../../header.hpp"

// 配列の持つ値が01の場合
// rangeで10を逆転(xor),queryで和を取る
// O(log(n))

template<typename T> class segtree {
private:
    int n,sz;
public:
    vector<T> node, lazy;
    segtree(const vector<T>& v) : n(1), sz((int)v.size()){
        while(n < sz) n *= 2;
        node.resize(2*n-1);
        lazy.resize(2*n-1, 0);
        for(int i = 0; i < sz; i++){
            node[i+n-1] = v[i];
        }
        for(int i=n-2; i>=0; i--){
            node[i] = node[i*2+1] + node[i*2+2];
        }
    }
    void eval(int k, int l, int r) {
        if(lazy[k] != 0) {
            node[k] = (r-l) - node[k];
            if(r - l > 1) {
                lazy[2*k+1] ^= lazy[k];
                lazy[2*k+2] ^= lazy[k];
            }
            lazy[k] = 0;
        }
    }
    void range(int a, int b, int k=0, int l=0, int r=-1) {
        if(r < 0) r = n;
        eval(k, l, r);
        if(b <= l || r <= a){
            return;
        }
        if(a <= l && r <= b) {
            lazy[k] ^= 1;
            eval(k, l, r);
        }
        else {
            range(a, b, 2*k+1, l, (l+r)/2);
            range(a, b, 2*k+2, (l+r)/2, r);
            node[k] = node[2*k+1] + node[2*k+2];
        }
    }
    T query(int a, int b, int k=0, int l=0, int r=-1) {
        if(r < 0) r = n;
        eval(k, l, r);
        if(b <= l || r <= a){
            return 0;
        }
        if(a <= l && r <= b){
            return node[k];
        }
        T vl = query(a, b, 2*k+1, l, (l+r)/2);
        T vr = query(a, b, 2*k+2, (l+r)/2, r);
        return vl + vr;
    }
    void print()
    {
        for(int i = 0; i < sz; i++){
            cout << query(i,i+1) << " ";
        }
        cout << endl;
    }
};
