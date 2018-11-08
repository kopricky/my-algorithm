#include "../header.hpp"

// 有向グラフを強連結成分分解する(O(n))

class SCC{
public:
	int V,cnt;
	vector<vector<int> > G,rG,graph;
	vector<int> vs,cmp;
	vector<bool> used;
	SCC(int node_size) : V(node_size), G(V), rG(V), cmp(V), used(V, false){}
	void add_edge(int from,int to){
		G[from].push_back(to);
		rG[to].push_back(from);
	}
	void dfs(int u){
		used[u] = true;
		for(int v : G[u]){
			if(!used[v]){
				dfs(v);
			}
		}
		vs.push_back(u);
	}
	void dfs(int u,const int k){
		used[u] = true;
		cmp[u] = k;
		for(int v : rG[u]){
			if(!used[v]){
				dfs(v,k);
			}
		}
	}
	int solve(){ //強連結成分の数を返す
		for(int i = 0; i < V; i++){
			if(!used[i]){
				dfs(i);
			}
		}
		fill(used.begin(),used.end(),false);
		cnt = 0;
		for(int i=V-1;i>=0;i--){
			if(!used[vs[i]]){
				dfs(vs[i],cnt++);
			}
		}
		return cnt;
	}
	void make_graph(){
		graph.resize(cnt);
		for(int i = 0; i < V; i++){
			for(int v : G[i]){
				if(cmp[i] != cmp[v]){
					graph[cmp[i]].push_back(cmp[v]);
				}
			}
		}
	}
};
