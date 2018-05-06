#include "../header.hpp"
#include "segtree.hpp"

//領域木(フラクショナルカスケーディングは実装していません)
//2次元上の一点更新および長方形領域のクエリに答えるデータ構造
//遅延処理は多次元だとうまく回らない(多次元領域の更新をO(polylog(n))でできるデータ構造は知られていない)
//点数がn個の場合
//時間計算量:構築O(nlog(n)),クエリO(log^2(n))
//空間計算量:O(nlog(n))
//verifyはしていません(verify問題を知らない)

//座標の型, 値の型
template<typename CandidateType, typename ValueType> class RangeTree
{
private:
    using CT = CandidateType;
    using VT = ValueType;
    using pcc = pair<CT, CT>;
    using pci = pair<CT, int>;
    vector<segtree<VT> > seg;
    //座標, インデックス
    vector<pair<pcc, int> > sorted;
    //x座標
    vector<CT> xs;
    //y座標, インデックス
    vector<vector<pci> > ys;
    int n, sz;
    void update(int xid, CT y, const VT x) {
        xid += n-1;
        int yid = lower_bound(all(y[xid]),pci(y,-1)) - y[xid].begin();
        seg[xid].update(yid,x);
        while(xid>0){
            xid = (xid-1)/2;
            int yid = lower_bound(all(y[xid]),pci(y,-1)) - y[xid].begin();
            seg[xid].update(yid,x);
        }
    }
    VT query(int lxid, int rxid, CT ly, CT ry, int k, int l, int r) {
        if(r <= lxid || rxid <= l) return numeric_limits<VT>::max();
        if(lxid <= l && r <= rxid){
            int lyid = lower_bound(all(ys[k]),pci(ly,-1)) - ys[k].begin();
            int ryid = upper_bound(all(ys[k]),pci(ry,-1)) - ys[k].begin();
            if(lyid >= ryid) return numeric_limits<VT>::max();
            return seg[k].query(lyid,ryid);
        }else{
            return min(query(lxid,rxid,ly,ry,2*k+1,l,(l+r)/2), query(lxid,rxid,ly,ry,2*k+2,(l+r)/2,r));
        }
    }
public:
    // 座標, 点の値
    RangeTree(vector<pcc>& cand, vector<VT>& val){
        sz = (int)cand.size();
        n = 1;
        while(n < sz){
            n *= 2;
        }
        sorted.resize(sz);
        rep(i,sz){
            sorted[i] = make_pair(cand[i], i);
        }
        sort(sorted.begin(), sorted.end());
        xs.resize(sz), ys.resize(2*n-1), seg.resize(2*n-1);
        rep(i,sz){
            xs[i] = (sorted[i].first).first;
            ys[i+n-1] = {pci((sorted[i].first).second, sorted[i].second)};
            vector<VT> arg = {val[sorted[i].second]};
            seg[i+n-1].init(arg);
        }
        for(int i=n-2; i>=0; i--){
            ys[i].resize((int)ys[2*i+1].size() + (int)ys[2*i+2].size());
            merge(all(ys[2*i+1]),all(ys[2*i+2]),ys[i].begin(),[&](pci& a, pci& b){
                return a.first < b.first;
            });
            vector<VT> arg((int)ys[i].size());
            rep(j,(int)ys[i].size()){
                arg[j] = val[ys[i][j].second];
            }
            seg[i].init(arg);
        }
    }
    //[lx,rx)×[ly,ry)の長方形領域の更新を行う
    void update(CT x, CT y, const VT val){
        int xid = lower_bound(all(xs),x) - xs.begin();
        return update(xid,y,val,0,0,n);
    }
    //[lx,rx)×[ly,ry)の長方形領域のクエリに答える
    VT query(CT lx, CT ly, CT rx, CT ry){
        int lxid = lower_bound(all(xs),lx) - xs.begin();
        int rxid = upper_bound(all(xs),rx) - xs.begin();
        if(lxid >= rxid) return numeric_limits<VT>::max();
        return query(lxid,rxid,ly,ry,0,0,n);
    }
};