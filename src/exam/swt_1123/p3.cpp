// reference solution: dong2.trinh
#include <bits/stdc++.h>
#define int int64_t
using namespace std;

int32_t main(){
    // Input
    int n;
    cin >> n;
    string s;
    cin >> s;
    // Số areas được bao phủ bởi line tăng lên 1 khi: có 1 CẠNH MỚI nối đến 1 ĐIỂM đã TỪNG ĐI QUA
    // -> Cần phải xác định xem cạnh có phải là CẠNH MỚI hay không? -> sử dụng map<vector<int>, bool> để check, với vector<int> chứa thông tin của 2 node {x1, y1, x2, y2}
    // -> Cần phải xác định xem điểm đó đã đi qua chưa -> sử dụng map<pair<int,int>, bool> để check, với pair<int, int> chứa thông tin của node {x, y}
    int dx[4] = {0,0,1,-1};  // hướng theo chiều x
    int dy[4] = {1,-1,0,0};  // hướng theo chiều y
    int x0 = 0, y0 = 0;        // tọa độ hiện tại
    map<pair<int,int>, bool> mpNode;  // map check visited ĐIỂM
    mpNode[{0,0}] = true;
    map<vector<int>, bool> mpEdge; // map check visited CẠNH
    //
    int res = 0;
    for(auto & c : s) {
        // Toa do diem tiep theo
        int nx = x0 + dx[c-'0'];
        int ny = y0 + dy[c-'0'];
        // Kiem tra canh nay da tung di qua chua
        if(mpEdge.count({x0, y0, nx, ny})) { // da tung di qua
            // nothing
        } else { // chua di qua
            // ktra xem node tiep theo da tung di qua chua
            if(mpNode.count({nx,ny})) { // da tung di qua
                res++;
            } else { // chua di qua
                // cap nhat
                mpNode[{nx, ny}] = true;
            }
            // cap nhat mpEdge
            mpEdge[{x0,y0,nx,ny}]=true;
            mpEdge[{nx,ny,x0,y0}]=true;
        }
        x0 = nx;
        y0 = ny;
    }
    // OUTPUT
    cout<<res<<endl;

    return 0;
}