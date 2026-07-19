#include <bits/stdc++.h>
using namespace std;
// gfg potd

vector<bool> processQueries(vector<int> &arr,
                            vector<vector<int>> &queries) {
    int n = arr.size();

    // dec[i] stores the farthest index reachable
    // from i while maintaining a non-decreasing sequence.
    vector<int> dec(n);
    dec[n - 1] = n - 1;

    for (int i = n - 2; i >= 0; i--) {
        if (arr[i] <= arr[i + 1]) {
            dec[i] = dec[i + 1];
        } else {
            dec[i] = i;
        }
    }

    // inc[i] stores the leftmost index reachable
    // from i while maintaining a non-increasing sequence.
    vector<int> inc(n);
    inc[0] = 0;

    for (int i = 1; i < n; i++) {
        if (arr[i] <= arr[i - 1]) {
            inc[i] = inc[i - 1];
        } else {
            inc[i] = i;
        }
    }

    vector<bool> ans;

    for (auto &query : queries) {
        int l = query[0];
        int r = query[1];

        ans.push_back(dec[l] >= inc[r]);
    }

    return ans;
}

int main() {
    vector<int> arr = {2, 3, 2, 4, 4, 6, 3, 2};

    vector<vector<int>> queries = {
        {0, 2},
        {1, 3}
    };

    vector<bool> ans = processQueries(arr, queries);

    for (bool x : ans) {
        cout << (x ? "true" : "false") << " ";
    }

    return 0;
}
