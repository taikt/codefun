#include <bits/stdc++.h>
using namespace std;
typedef long long ll;

int main() {
    freopen("input.txt","r",stdin);

	int n;
	int k;
	cin >> n >> k;
	vector<int> arr(n);
	for (int i = 0; i < n; i++) { cin >> arr[i]; }

	int left = 0;
	int right = 0;
	ll ans = 0;
	int distinct = 0;
	map<int, int> freq;
	while (left < n) {
		while (right < n) {
			if (freq.count(arr[right]) == 0 && distinct == k) { break; }

			// Put the new array element in the map.
			if (freq.count(arr[right]) == 0) {
				freq[arr[right]] = 1;
				distinct++;
			} else {
				freq[arr[right]]++;
			}
			right++;
		}

		// Add the new subarrays.
        
		ans += (right - left);
        cout<<"ans="<<ans<<"left="<<left<<", right="<<right<<"\n";

		// Slide the window rightwards.
		if (freq[arr[left]] == 1) {
			distinct--;
			freq.erase(arr[left]);
		} else {
			freq[arr[left]] -= 1;
		}
		left++;
	}

	cout << ans << endl;
}