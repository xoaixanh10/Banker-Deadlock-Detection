#include <iostream>
#include <vector>
#include <string>

using namespace std;

struct State {
    vector<vector<int>> Max;
    vector<vector<int>> Allocation;
    vector<int> Available;
    
    int requestProcessId; 
    vector<int> Request;  
};

struct SafetyResult {
    bool isSafe;
    vector<int> safeSequence;
};

vector<vector<int>> calculateNeed(const State& s) {
    int n = s.Max.size(), m = s.Available.size();
    vector<vector<int>> Need(n, vector<int>(m));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            Need[i][j] = s.Max[i][j] - s.Allocation[i][j];
        }
    }
    return Need;
}

SafetyResult bankerSafety(const State& s) {
    int n = s.Allocation.size(), m = s.Available.size();
    vector<vector<int>> Need = calculateNeed(s);
    vector<int> Work = s.Available;
    vector<bool> Finish(n, false);
    SafetyResult res;
    
    int count = 0;
    bool found;
    do {
        found = false;
        for (int p = 0; p < n; p++) {
            if (!Finish[p]) {
                bool canRun = true;
                for (int j = 0; j < m; j++) {
                    if (Need[p][j] > Work[j]) { canRun = false; break; }
                }

                if (canRun) { 
                    for (int j = 0; j < m; j++) Work[j] += s.Allocation[p][j];
                    res.safeSequence.push_back(p);
                    Finish[p] = true;
                    found = true;
                    count++;
                }
            }
        }
    } while (found); 

    res.isSafe = (count == n); 
    return res;
}

bool bankerRequestAccepted(const State& s, string& explanation) {
    int p = s.requestProcessId;
    int m = s.Available.size();
    vector<vector<int>> Need = calculateNeed(s);

    for (int j = 0; j < m; j++) {
        if (s.Request[j] > Need[p][j]) { explanation = "Lỗi: Vượt quá Need"; return false; }
        if (s.Request[j] > s.Available[j]) { explanation = "Đợi: Không đủ Available"; return false; }
    }

    State s_copy = s;
    for (int j = 0; j < m; j++) {
        s_copy.Available[j] -= s.Request[j];
        s_copy.Allocation[p][j] += s.Request[j];
    }

    if (bankerSafety(s_copy).isSafe) {
        explanation = "Chấp nhận: Hệ thống an toàn";
        return true;
    } 
    
    explanation = "Từ chối: Gây mất an toàn (Deadlock)";
    return false;
}

int main() {
    State s;
    
    s.Available = {3, 3, 2};
    s.Max = {{7, 5, 3}, {3, 2, 2}, {9, 0, 2}, {2, 2, 2}, {4, 3, 3}};
    s.Allocation = {{0, 1, 0}, {2, 0, 0}, {3, 0, 2}, {2, 1, 1}, {0, 0, 2}};

    SafetyResult res = bankerSafety(s);
    if (res.isSafe) {
        cout << "He thong an toan.\n";
    } else {
        cout << "He thong khong an toan (Deadlock).\n";
    }
    
    return 0;
}