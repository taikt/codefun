#include <bits/stdc++.h>
#define int long long

using namespace std;

const int maxn = 16;
int f, r, n, m;
int val[maxn][maxn], all[maxn];
int dp[1 << maxn];
pair < int, int > tv[1 << maxn];

bool mini(int &a, int b) 
{
    if (a > b) 
    {
        a = b;
        return true;
    }

    return false;
}

void solution()
{
    memset(dp, 0x3f3f3f, sizeof(dp));
    int inf = dp[0];
    dp[0] = 0;
    int mx = 1 << n;
    for (int mask = 0; mask < mx; ++mask) 
    {
        if (dp[mask] == inf) 
            continue;

        for (int i = 0; i < n; ++i) 
        {
            if (mask & (1 << i)) 
                continue;

            for (int j = 0; j < n; ++j) 
            {
                if (i == j || (mask & (1 << j))) 
                    continue;

                int nw = mask | (1 << i) | (1 << j);
                if (mini(dp[nw], dp[mask] + 2 * val[i][j] * f + (all[i] - val[i][j]) * r + (all[j] - val[j][i]) * r)) 
                    tv[nw] = {i,j};
            }
        }
    }

    cout << dp[mx - 1] / 2 << '\n';

    // Truy vết.
    int mask = mx - 1;
    while (mask) 
    {
        pair < int, int > t = tv[mask];
        cout << t.first + 1 << ' ' << t.second + 1 << '\n';
        mask ^= (1 << t.first);
        mask ^= (1 << t.second);
    }
}

main() 
{
    cin >> f >> r >> n >> m;

    for (int i = 1; i <= m; i++) 
    {
        int u,v,c; 
        cin >> u >> v >> c; 
        
        u--; 
        v--;
        if (u > v) 
            swap(u,v);

        val[u][v] += c;
        val[v][u] += c;
        all[u] += c;
        all[v] += c;
    }

    solution();

    return 0;
}
