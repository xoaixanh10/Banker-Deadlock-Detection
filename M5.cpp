#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <chrono>

using namespace std;

struct State {
    int n = 0;
    int m = 0;
    vector<int> available;
    vector<vector<int>> maximum;
    vector<vector<int>> allocation;
    vector<vector<int>> request;
    vector<bool> terminated;
};

struct SafetyResult {
    bool safe = false;
    vector<int> sequence;
};

struct DetectionResult {
    vector<int> deadlocked;
};

static bool canFinish(const State& s, int processId, const vector<int>& work) {
    if (processId < 0 || processId >= s.n) return false;
    if (processId >= (int)s.maximum.size()) return false;

    const vector<int>& need = s.maximum[processId];
    const vector<int>& alloc = s.allocation[processId];

    for (int j = 0; j < s.m; ++j) {
        int needValue = (j < (int)need.size()) ? need[j] - alloc[j] : 0;
        if (needValue < 0) needValue = 0;
        if (needValue > work[j]) {
            return false;
        }
    }
    return true;
}

SafetyResult bankerSafety(const State& s) {
    SafetyResult res;
    if (s.n <= 0 || s.m <= 0) {
        return res;
    }

    vector<int> work = s.available;
    vector<bool> finish(s.n, false);
    vector<int> sequence;

    bool progress = true;
    while (progress) {
        progress = false;
        for (int i = 0; i < s.n; ++i) {
            if (finish[i]) continue;
            if (canFinish(s, i, work)) {
                for (int j = 0; j < s.m; ++j) {
                    if (j < (int)s.allocation[i].size()) {
                        work[j] += s.allocation[i][j];
                    }
                }
                finish[i] = true;
                sequence.push_back(i);
                progress = true;
            }
        }
    }

    res.safe = true;
    for (bool f : finish) {
        if (!f) {
            res.safe = false;
            break;
        }
    }
    res.sequence = sequence;
    return res;
}

DetectionResult deadlockDetection(const State& s) {
    DetectionResult result;
    if (s.n <= 0) {
        return result;
    }

    vector<int> work = s.available;
    for (int i = 0; i < s.n; ++i) {
        if (i < (int)s.terminated.size() && s.terminated[i]) continue;

        bool hasRequest = false;
        bool blocked = false;
        for (int j = 0; j < s.m; ++j) {
            int req = (j < (int)s.request[i].size()) ? s.request[i][j] : 0;
            int alloc = (j < (int)s.allocation[i].size()) ? s.allocation[i][j] : 0;
            if (req > 0 || alloc > 0) {
                hasRequest = true;
            }
            if (req > work[j]) {
                blocked = true;
            }
        }

        if (hasRequest && blocked) {
            result.deadlocked.push_back(i);
        }
    }

    return result;
}

void recoverTerminate(State& s, int processId) {
    if (processId < 0 || processId >= s.n) return;
    if (processId >= (int)s.terminated.size()) {
        s.terminated.resize(s.n, false);
    }

    for (int j = 0; j < s.m; ++j) {
        if (j < (int)s.allocation[processId].size()) {
            s.available[j] += s.allocation[processId][j];
            s.allocation[processId][j] = 0;
        }
        if (j < (int)s.request[processId].size()) {
            s.request[processId][j] = 0;
        }
    }
    s.terminated[processId] = true;
}

// ==========================================
// - THANH VIEN 5: CSV, TEST SUITE & BENCHMARK -
// ==========================================

// Helper: Cắt chuỗi theo ký tự phân cách (dùng cho doc CSV)
static vector<string> splitString(const string& str, char delimiter) {
    vector<string> tokens;
    string token;
    stringstream ss(str);
    while (getline(ss, token, delimiter)) {
        // Loai bo khoang trang o dau va cuoi chuoi
        size_t start = token.find_first_not_of(" \t\r\n");
        size_t end = token.find_last_not_of(" \t\r\n");
        if (start != string::npos && end != string::npos) {
            tokens.push_back(token.substr(start, end - start + 1));
        } else if (start != string::npos) {
            tokens.push_back(token.substr(start));
        } else {
            tokens.push_back("");
        }
    }
    return tokens;
}

// TODO M5-1: Doc file CSV va nap du lieu vao struct State
bool loadCsv(const string& fileName, State& s, string& error) {
    ifstream file(fileName);
    if (!file.is_open()) {
        error = "Loi: Khong the mo file CSV tu duong dan: " + fileName;
        return false;
    }

    string line;
    string section = "";

    State tempState;
    bool hasAvailable = false;

    while (getline(file, line)) {
        if (line.empty()) continue;

        // Xoa ky tu \r neu doc tu file Windows
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        // Kiem tra tieu de section
        if (line.find("[Available]") != string::npos) { section = "Available"; continue; }
        if (line.find("[Max]") != string::npos) { section = "Max"; continue; }
        if (line.find("[Allocation]") != string::npos) { section = "Allocation"; continue; }
        if (line.find("[Request]") != string::npos) { section = "Request"; continue; }

        vector<string> tokens = splitString(line, ',');
        if (tokens.empty() || tokens[0].empty()) continue;

        try {
            if (section == "Available") {
                for (const string& t : tokens) {
                    if (!t.empty()) tempState.available.push_back(stoi(t));
                }
                hasAvailable = true;
            }
            else if (section == "Max") {
                vector<int> row;
                for (const string& t : tokens) {
                    if (!t.empty()) row.push_back(stoi(t));
                }
                if (!row.empty()) tempState.maximum.push_back(row);
            }
            else if (section == "Allocation") {
                vector<int> row;
                for (const string& t : tokens) {
                    if (!t.empty()) row.push_back(stoi(t));
                }
                if (!row.empty()) tempState.allocation.push_back(row);
            }
            else if (section == "Request") {
                vector<int> row;
                for (const string& t : tokens) {
                    if (!t.empty()) row.push_back(stoi(t));
                }
                if (!row.empty()) tempState.request.push_back(row);
            }
        } catch (...) {
            error = "Loi: Dinh dang du lieu trong file CSV khong hop le (co ky tu khong phai so).";
            file.close();
            return false;
        }
    }
    file.close();

    // Kiem tra va validate du lieu nap vao
    if (!hasAvailable || tempState.available.empty()) {
        error = "Loi: Thieu du lieu Available trong file CSV.";
        return false;
    }

    tempState.m = tempState.available.size();
    tempState.n = tempState.allocation.size();

    if (tempState.maximum.size() != (size_t)tempState.n) {
        error = "Loi: So luong tien trinh giua Allocation va Max khong dong nhat.";
        return false;
    }

    // Khoi tao marray terminated ban dau
    tempState.terminated.assign(tempState.n, false);

    // Gan vao State goc sau khi da kiem tra hop le
    s = tempState;
    error = "Doc file CSV thanh cong.";
    return true;
}

// TODO M5-2: Xuat nhat ky (trace) thuat toan ra file CSV
bool exportTraceCsv(const string& fileName, const vector<string>& trace) {
    ofstream file(fileName);
    if (!file.is_open()) {
        return false;
    }

    file << "Step,Description\n";
    for (size_t i = 0; i < trace.size(); ++i) {
        // Xu ly dau ngoac kep neu chuoi trace co chua dau phay
        string escaped = trace[i];
        if (escaped.find(',') != string::npos) {
            escaped = "\"" + escaped + "\"";
        }
        file << (i + 1) << "," << escaped << "\n";
    }

    file.close();
    return true;
}

// Helper: In ket qua kiem thu
static void printTestResult(const string& testName, bool passed) {
    cout << "[TEST] " << testName << " -> " << (passed ? "PASSED (DAT)" : "FAILED (KHONG DAT)") << "\n";
}

// TODO M5-3: Khung kiem thu tu dong (Test Suite Oracle)
void runTests() {
    cout << "\n==========================================\n";
    cout << "   BAT DAU CHAY BO KIEM THU (RUN TESTS)   \n";
    cout << "==========================================\n\n";

    // Ca kiem thu 1: Banker Safety - Trang thai An toan (Safe)
    {
        State s;
        s.n = 5; s.m = 3;
        s.available = {3, 3, 2};
        s.maximum = {{7, 5, 3}, {3, 2, 2}, {9, 0, 2}, {2, 2, 2}, {4, 3, 3}};
        s.allocation = {{0, 1, 0}, {2, 0, 0}, {3, 0, 2}, {2, 1, 1}, {0, 0, 2}};
        s.terminated.assign(s.n, false);

        SafetyResult res = bankerSafety(s);
        printTestResult("Banker Safety (Safe State)", res.safe == true && res.sequence.size() == 5);
    }

    // Ca kiem thu 2: Banker Safety - Trang thai Mất an toan (Unsafe)
    {
        State s;
        s.n = 3; s.m = 2;
        s.available = {1, 0};
        s.maximum = {{3, 2}, {2, 2}, {2, 1}};
        s.allocation = {{1, 1}, {1, 1}, {0, 0}};
        s.terminated.assign(s.n, false);

        SafetyResult res = bankerSafety(s);
        printTestResult("Banker Safety (Unsafe State)", res.safe == false);
    }

    // Ca kiem thu 3: Deadlock Detection - Phat hien Deadlock
    {
        State s;
        s.n = 3; s.m = 2;
        s.available = {0, 0};
        s.allocation = {{1, 0}, {0, 1}, {1, 1}};
        s.request = {{0, 1}, {1, 0}, {0, 0}};
        s.terminated.assign(s.n, false);

        DetectionResult res = deadlockDetection(s);
        printTestResult("Deadlock Detection (Has Deadlock)", res.deadlocked.size() == 2);
    }

    // Ca kiem thu 4: Recovery - Mo phong ket thuc tien trinh giai phong tai nguyen
    {
        State s;
        s.n = 3; s.m = 2;
        s.available = {0, 0};
        s.allocation = {{1, 0}, {0, 1}, {1, 1}};
        s.request = {{0, 1}, {1, 0}, {0, 0}};
        s.terminated.assign(s.n, false);

        recoverTerminate(s, 0); // Huy P0
        DetectionResult res = deadlockDetection(s);
        printTestResult("Recovery (Terminate P0 & Clear Deadlock)", res.deadlocked.empty());
    }

    cout << "\n==========================================\n";
    cout << "       HOAN THANH KHIEM THU TU DONG       \n";
    cout << "==========================================\n\n";
}

// TODO M5-4: Do thoi gian thuc thi (Benchmark performance)
void runBenchmark() {
    cout << "\n==========================================\n";
    cout << "   BAT DAU DO HIEU NANG (RUN BENCHMARK)   \n";
    cout << "==========================================\n\n";

    // Khoi tao du lieu gia lap kich thuoc lon
    State s;
    s.n = 500;
    s.m = 50;
    s.available = vector<int>(s.m, 100);
    s.maximum = vector<vector<int>>(s.n, vector<int>(s.m, 10));
    s.allocation = vector<vector<int>>(s.n, vector<int>(s.m, 1));
    s.request = vector<vector<int>>(s.n, vector<int>(s.m, 1));
    s.terminated.assign(s.n, false);

    auto start = chrono::high_resolution_clock::now();
    SafetyResult res = bankerSafety(s);
    auto end = chrono::high_resolution_clock::now();

    chrono::duration<double, milli> duration = end - start;
    cout << "Benchmark Banker Safety (N=" << s.n << ", M=" << s.m << "): " 
         << duration.count() << " ms\n";

    start = chrono::high_resolution_clock::now();
    DetectionResult detRes = deadlockDetection(s);
    end = chrono::high_resolution_clock::now();

    duration = end - start;
    cout << "Benchmark Deadlock Detection (N=" << s.n << ", M=" << s.m << "): " 
         << duration.count() << " ms\n\n";
}

int main() {
    runTests();
    runBenchmark();
    return 0;
}
