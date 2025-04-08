#include <bits/stdc++.h>
using namespace std;
#define int int64_t
//start your code

const int  mod =1e9+7;

bool check(int &x,int &y,int &nx,int &ny,int &direction,int &k,vector<vector<int>> &a)
{
    if(direction==0) //trai
    {
        int c1 = y-1,c2 = y;
        for(int r= x;r<x+k;r++) if(a[r][c1]!=a[r][c2]) return false;
        
    }
    else if(direction==1) //phai
    {
        int c1 = ny-1,c2 = ny;
        for(int r= nx;r<nx+k;r++) if(a[r][c1]!=a[r][c2]) return false;
        
    }
    else if(direction==2)//duoi
    {
        int r1 = nx-1,r2 =nx;
        for(int c = ny;c<ny+k;c++) if(a[r1][c]!=a[r2][c]) return false;
    }
    else //tren
    {
        int r1 = x-1,r2 =x;
        for(int c = y;c<y+k;c++) if(a[r1][c]!=a[r2][c]) return false;
    }
    return true;
}
void solve()
{
    int n;
    cin>>n;
    vector<vector<int>> a(n,vector<int>(n));
    for(auto&x:a) for(auto&y:x) cin>>y;
    int res = 1;
    int direct[4][2]= {{0,-1},{0,1},{1,0},{-1,0}}; //trai,phai, duoi, tren
    for(int k=2;k<n;k++)
    {
        if(n%k) continue;
        //bfs
        vector<vector<bool>> visited(n,vector<bool>(n));
        visited[0][0] =  true;
        queue<pair<int,int>> q;
        q.push({0,0});
        int cnt= 0;
        while(!q.empty())
        {
            int x =q.front().first,y = q.front().second;
            q.pop();
            cnt++;
            for(int i=0;i<4;i++)
            {
                int nx = x+ direct[i][0]*k;
                int ny = y+direct[i][1]*k;
                if(nx<0||nx>=n || ny<0 ||ny>=n) continue;
                if(!check(x,y,nx,ny,i,k,a)) continue;
                if(visited[nx][ny]) continue;
                visited[nx][ny]=true;
                q.push({nx,ny});
            }
        }
        res = max(res,cnt);
    }
    cout<<res<<endl;
}

int32_t main()
{
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);
    cout.tie(NULL);
    int t=1;
    // cin>>t;
    for(int i=1;i<=t;i++)
    {
        // cout<<"Case #"<<i<<": ";
        solve();
    }
    return 0;
}