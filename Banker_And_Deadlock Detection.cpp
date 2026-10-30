#include <iostream>
#include <vector>
#include <string>  // Thu vien utility cung cap pair de luu hai dinh cua mot canh trong do thi cho.
#include <utility>

using namespace std;

// Struct State: Luu toan bo du lieu va trang thai hien tai cua he thong.
struct State {
    int n = 0;                         // So luong tien trinh
    int m = 0;                         // So luong loai tai nguyen

    vector<int> available;             // So tai nguyen con trong cua moi loai
    vector<vector<int>> allocation;   // Ma tran tai nguyen da cap phat cho cac tien trinh
    vector<vector<int>> maximum;      // Ma tran nhu cau tai nguyen toi da cua cac tien trinh
    vector<vector<int>> request;      // Ma tran yeu cau tai nguyen dang cho cua cac tien trinh

    int bankerPid = -1;                // ID cua tien trinh dang gui yeu cau trong Banker
    vector<int> bankerRequest;         // Vector tai nguyen ma tien trinh do dang yeu cau
    vector<bool> terminated;           // Danh dau tien trinh da bi ket thuc trong mo phong phuc hoi
};

// Struct SafetyResult: Luu ket qua cua thuat toan Banker Safety.
// Cung cap thong tin trang thai an toan, chuoi an toan va qua trinh thuc hien thuat toan.
struct SafetyResult {
    bool safe = false;                 // true neu he thong an toan, false neu khong an toan
    vector<int> sequence;              // Luu chuoi an toan (Safe Sequence) neu tim duoc
    vector<string> trace;              // Luu nhat ky cac buoc thuc hien thuat toan
    vector<int> unfinished;            // Luu danh sach tien trinh chua hoan thanh
};

// Struct DetectionResult: Luu ket qua cua thuat toan Deadlock Detection.
// Cho biet nhung tien trinh bi deadlock va ghi lai cac buoc kiem tra.
struct DetectionResult {
    vector<int> deadlocked;            // Danh sach ID cac tien trinh bi deadlock
    vector<string> trace;              // Luu nhat ky cac buoc phat hien deadlock
};

// ==================== THANH VIEN 2: Tran Duy Khang (BANKER) ====================
// Tinh ma tran Need tu Max va Allocation cho tung tien trinh va loai tai nguyen.
vector<vector<int>> calculateNeed(const State& s) {
    return {};
}

// Kiem tra trang thai an toan bang Banker Safety va tra ve safe sequence neu co.
SafetyResult bankerSafety(const State& s) {
    SafetyResult result;
    return result;
}

// Thu cap phat tai nguyen cho mot tien trinh va chi chap nhan neu trang thai sau cap phat van an toan.
bool bankerRequestAccepted(const State& s, string& explanation) {
    explanation = "Chua cai dat Banker Resource-Request.";
    return false;
}

// ==================== THANH VIEN 3: Tran Hoang Thien Loc (DEADLOCK + PHUC HOI) ====================
// Phat hien cac tien trinh bi deadlock dua tren Available, Allocation va Request.
DetectionResult deadlockDetection(const State& s) {
    DetectionResult result;
    return result;
}

// Tao cac canh Pi -> Pj de bieu dien quan he Pi dang cho tai nguyen do Pj giu.
vector<pair<int, int>> buildWaitForEdges(const State& s) {
    return {};
}

// Mo phong ket thuc mot tien trinh va tra lai Available cac tai nguyen tien trinh do dang giu.
void recoverTerminate(State& s, int processId) {
}

// ==================== THANH VIEN 4: Pham La Trung Nam (GUI) ====================

// ========================================

// ==================== THANH VIEN 5: Cao Le Minh Đang (CSV + TEST) ====================
// Ham khoi dong giao dien desktop; thanh vien phu trach GUI se trien khai than ham.
void runApp();

// Doc du lieu tu file CSV vao State va tra ve thong bao neu du lieu khong hop le.
bool loadCsv(const string& fileName, State& s, string& error) {
    error = "Chua cai dat chuc nang doc CSV.";
    return false;
}

// Ghi nhat ky cac buoc thuc thi ra mot file CSV.
bool exportTraceCsv(const string& fileName, const vector<string>& trace) {
    return false;
}

// Chay cac ca kiem thu va so sanh ket qua thuc te voi ket qua mong doi.
void runTests() {
    cout << "Chua cai dat cac ca kiem thu.\n";
}

int main() {
    cout << "BO KHUNG BAI TAP BANKER VA DEADLOCK DETECTION\n";
    return 0;
}
