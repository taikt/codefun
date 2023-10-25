// https://leetcode.com/problems/network-delay-time/description/

class Solution {
public:
    // BFS method
    int networkDelayTime(vector<vector<int>>& times, int N, int K) {
        
        // graph Adjacency list containing connected node and delay time
        vector<vector<pair<int,int>>>graph(N+1);
        
        // Populate graph Adjacency list 
        for(int i=0;i<times.size();i++)
        {
            graph[times[i][0]].push_back(make_pair(times[i][1],times[i][2]));
        }
        
        // time cost vector of nodes
        vector<int>timeCost(N+1,INT_MAX);
        
        queue<pair<int,int>>Q;

        // insert start node and time 0 in to queue
        Q.push(make_pair(K,0));
        
        
        while(!Q.empty())
        {
            int node = Q.front().first;
            int time = Q.front().second;

            // update minimum time of current node
            timeCost[node] = min(time,timeCost[node]);

            Q.pop();                

            for(int i=0;i<graph[node].size();i++)
            {
                // Push neighbour in to queue if updated time cost is less than current time cost of neighbour
                // This condition stops graph loop and also updates nodes time cost if there is shorter time available 
                if(graph[node][i].second+time<timeCost[graph[node][i].first])
                    Q.push(make_pair(graph[node][i].first,graph[node][i].second+time));
            }
        }
        
        // remove 0th index value
        timeCost.erase(timeCost.begin());
        
        // find max timeCost of nodes
        int result = *max_element(timeCost.begin(),timeCost.end());
        
        // If max timeCost is INT_MAX then all nodes are not explored and return -1
        return result==INT_MAX?-1:result;        
    }
};

#if 0
class Solution {
public:
    int networkDelayTime(vector<vector<int>>& times, int n, int k) {
        vector<vector<pair<int,int>>> adj(n,vector<pair<int,int>>());
        // [u,v] = w
        // Adj[u].push_back(make_pair(v,w));
        for (auto time: times) {
            // time[0] = u, time[1] = v, time[2] = w
            adj[time[0]-1].push_back(make_pair(time[1]-1, time[2]));
        }

        // or adj[u][v] = w
        // vector<vector<int>> adj;

        // dist la vector luu khoang cach cac dinh toi source
        vector<int> dist(n,101);
        dist[k-1] = 0;
        //for (int i=0; i<n;i++)
        //printf("i=%d, disk=%d\n",i,dist[i]);

        vector<int> visited(n,0);
        // tim dinh co khoang cach nho nhat toi source


        // tim dinh ke voi dinh tren, update khoang cach toi dinh do (neu nho hon)
        for (int i=0; i<n; i++) {
            int x = minVertex(n,visited,dist);
            printf("x=%d\n",x);
            visited[x] = 1;
            // tim cac dinh ke cua x: u -> W
            for (auto ed: adj[x]) {
                int u = ed.first, w = ed.second;   
                printf("u=%d, w=%d\n",u,w);
                if (!visited[u] && dist[u] > dist[x] + w) {
                    dist[u] = dist[x] + w;

                }        
            }
        }

        // tim gia tri max trong dist
        int max = 0;
        for (int i=0; i<n; i++) {
            printf("final d=%d, i=%d\n",dist[i],i);
            if (dist[i] == 101) return -1;
            if (max < dist[i]) {
                max = dist[i];
            }
        }
        return max;
    }
    
    int minVertex(int n, vector<int>& visited, vector<int>& dist) {
        int min = INT_MAX, index;
        for (int i=0; i<n; i++) {
            //printf("min=%d, dist=%d,i=%d, visted=%d\n",min,dist[i],i,visited[i]);
            if (min > dist[i] && !visited[i]) {
                //printf("min=%d\n",min);
                min = dist[i];
                index = i;
            }
        }
        return index;
    }
};



//  distra using priority queue
typedef pair<int,int> pii;
class Solution {
public:
    int networkDelayTime(vector<vector<int>>& times, int n, int k) {
       
        vector<vector<pair<int,int>>> adj(n,vector<pair<int,int>>());
        // [u,v] = w, 0 <= wi <= 100
        // Adj[u].push_back(make_pair(v,w));
        for (auto time: times) {
            // time[0] = u, time[1] = v, time[2] = w
            adj[time[0]-1].push_back(make_pair(time[1]-1, time[2]));
        }

        vector<int> dist(n,101); // 0 <= wi <= 100
        dist[k-1] = 0;
        priority_queue<pii, vector<pii>, greater<pii>> pq; // min-heap
        pq.push({0,k-1}); // w, vertice
        while (!pq.empty()) {
            pii x = pq.top(); pq.pop();
            int u = x.second; // u la dinh co khoang cach toi src nho nhat trong cac dinh chua tham (thuoc priority queue)
            // tim canh ke cua u
            for (auto ed : adj[u]) {
                int v = ed.first, w = ed.second;// u -> v = w
                if (dist[v] > dist[u] + w) {
                    dist[v] = dist[u] + w;
                    pq.push({dist[v],v});
                }
            } 
        }

        // tim gia tri max trong dist
        int max = 0;
        for (int i=0; i<n; i++) {
            //printf("final d=%d, i=%d\n",dist[i],i);
            if (dist[i] == 101) return -1;
            if (max < dist[i]) {
                max = dist[i];
            }
        }
        return max;
    }
};
#endif