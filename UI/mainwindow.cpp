#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QStyledItemDelegate>
#include <QLineEdit>
#include <QIntValidator>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "BankerTest.h"//Lụm code dạo để test trước chừng nào có module khác thì đổi
#include "DeadlockMock.h"

//------------------------------------------------------------------------------------------

class IntegerOnlyDelegate : public QStyledItemDelegate {
public:
    explicit IntegerOnlyDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    // Hàm này tự động chạy khi người dùng đúp chuột vào ô để sửa
    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        QLineEdit *editor = new QLineEdit(parent);

        // Thiết lập bộ lọc: Chỉ cho phép nhập số nguyên từ 0 đến 999999 (Không cho nhập số âm và chữ)
        QIntValidator *validator = new QIntValidator(0, 999999, editor);
        editor->setValidator(validator);

        return editor;
    }
};

//------------------------------------------------------------------------------------------

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->Allocation_table->setItemDelegate(new IntegerOnlyDelegate(this));
    ui->Max_table->setItemDelegate(new IntegerOnlyDelegate(this));
    ui->Available_table->setItemDelegate(new IntegerOnlyDelegate(this));
    ui->Request_table->setItemDelegate(new IntegerOnlyDelegate(this));
}

//------------------------------------------------------------------------------------------

MainWindow::~MainWindow()
{
    delete ui;
}

//------------------------------------------------------------------------------------------

void MainWindow::on_ResizeTable_bt_clicked()//Nút "Tạo bảng/cập nhật"
{
    //Lấy giá trị n (số tiến trình) và m (số loại tài nguyên) từ QSpinBox.
    int n = ui->spinN->value();
    int m = ui->spinM->value();

    // Kiểm tra an toàn: nếu n hoặc m bằng 0 thì không làm gì cả.
    if (n <= 0 || m <= 0) {
        return;
    }

    //Cấu hình kích thước cho các ma trận n hàng x m cột (Allocation, Max, Request, Need)
    //Bảng Allocation
    ui->Allocation_table->setRowCount(n);
    ui->Allocation_table->setColumnCount(m);

    //Bảng Max
    ui->Max_table->setRowCount(n);
    ui->Max_table->setColumnCount(m);

    //Bảng Request
    ui->Request_table->setRowCount(n);
    ui->Request_table->setColumnCount(m);

    //Bảng Need
    ui->Need_table->setRowCount(n);
    ui->Need_table->setColumnCount(m);

    //Cấu hình cho vector Available (có kích thước 1 hàng x m cột)
    ui->Available_table->setRowCount(1);
    ui->Available_table->setColumnCount(m);

    //Tạo danh sách tiêu đề cột (R0, R1, ...)
    QStringList resourceHeaders;
    for (int j = 0; j < m; ++j) {
        resourceHeaders << "R" + QString::number(j);
    }

    //Tạo danh sách tiêu đề hàng tiến trình (P0, P1, ...)
    QStringList processHeaders;
    for (int i = 0; i < n; ++i) {
        processHeaders << "P" + QString::number(i);
    }

    //Cấu hình kích thước và tiêu đề cho các bảng n x m ---
    QTableWidget* tablesNxM[] = {
        ui->Allocation_table,
        ui->Max_table,
        ui->Request_table,
        ui->Need_table
    };

    for (auto* table : tablesNxM) {
        table->setRowCount(n);
        table->setColumnCount(m);
        table->setHorizontalHeaderLabels(resourceHeaders);
        table->setVerticalHeaderLabels(processHeaders);
    }

    //Cấu hình kích thước và tiêu đề cho bảng Available (1 x m) ---
    ui->Available_table->setRowCount(1);
    ui->Available_table->setColumnCount(m);
    ui->Available_table->setHorizontalHeaderLabels(resourceHeaders);
    ui->Available_table->setVerticalHeaderLabels(QStringList() << "Avail");

    //Tự động điền sẵn số "0" cho toàn bộ các ô trong các bảng ---
    for (auto* table : tablesNxM) {
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < m; ++j) {
                table->setItem(i, j, new QTableWidgetItem("0"));
            }
        }
    }

    //Điền sẵn số "0" cho bảng Available
    for (int j = 0; j < m; ++j) {
        ui->Available_table->setItem(0, j, new QTableWidgetItem("0"));
    }

    //In thông báo xác nhận ra khung log để kiểm tra
    ui->txtLog->append("Đã tạo/cập nhật bảng thành công: " + QString::number(n) + " tiến trình, " + QString::number(m) + " loại tài nguyên.");
}

//---------------------------------------------------------------------------------------------------------------------------------------------

void MainWindow::on_BankerSafety_bt_clicked()//Nút "Kiểm tra Banker safety"
{
    //Khởi tạo đối tượng State theo chuẩn của nhóm
    State s;
    s.n = ui->spinN->value(); //Theo số dòng hiện tại của bảng
    s.m = ui->spinM->value(); //Theo số cột

    if (s.n <= 0 || s.m <= 0) {
        ui->txtLog->append("Vui lòng nhập số tiến trình và tài nguyên hợp lệ!");
        return;
    }

    //Cấp phát kích thước ma trận cho State
    s.allocation.assign(s.n, std::vector<int>(s.m, 0));
    s.maximum.assign(s.n, std::vector<int>(s.m, 0));
    s.available.assign(s.m, 0);

    //Đọc dữ liệu từ bảng Allocation
    for (int i = 0; i < s.n; ++i) {
        for (int j = 0; j < s.m; ++j) {
            QTableWidgetItem *item = ui->Allocation_table->item(i, j);
            s.allocation[i][j] = item ? item->text().toInt() : 0;
        }
    }

    //Đọc dữ liệu từ bảng Max (maximum)
    for (int i = 0; i < s.n; ++i) {
        for (int j = 0; j < s.m; ++j) {
            QTableWidgetItem *item = ui->Max_table->item(i, j);
            s.maximum[i][j] = item ? item->text().toInt() : 0;
        }
    }

    //Đọc dữ liệu từ bảng Available (1 hàng, m cột)
    for (int j = 0; j < s.m; ++j) {
        QTableWidgetItem *item = ui->Available_table->item(0, j);
        s.available[j] = item ? item->text().toInt() : 0;
    }

    //Tính toán ma trận Need và tự động điền kết quả vào bảng Need trên UI
    std::vector<std::vector<int>> needMatrix = calculateNeed(s);
    for (int i = 0; i < s.n; ++i) {
        for (int j = 0; j < s.m; ++j) {
            ui->Need_table->setItem(i, j, new QTableWidgetItem(QString::number(needMatrix[i][j])));
        }
    }

    //Gọi thuật toán kiểm tra Banker Safety
    SafetyResult res = bankerSafety(s);

    //In nhật ký (trace) ra khung txtLog của giao diện
    ui->txtLog->append("--- BẮT ĐẦU KIỂM TRA BANKER SAFETY ---");
    for (const std::string& stepMsg : res.trace) {
        ui->txtLog->append(QString::fromStdString(stepMsg));
    }

    if (res.safe) {
        QString seqStr = "==> Chuỗi an toàn: ";
        for (size_t i = 0; i < res.sequence.size(); ++i) {
            seqStr += "P" + QString::number(res.sequence[i]);
            if (i < res.sequence.size() - 1) seqStr += " -> ";
        }
        ui->txtLog->append(seqStr);
    } else {
        QString unfinStr = "==> Các tiến trình chưa hoàn thành: ";
        for (size_t i = 0; i < res.unfinished.size(); ++i) {
            unfinStr += "P" + QString::number(res.unfinished[i]) + " ";
        }
        ui->txtLog->append(unfinStr);
    }
}

//--------------------------------------------------------------------------------------------------------------------

void MainWindow::on_DeadlockDetection_bt_clicked()
{
    State s;
    s.n = ui->spinN->value();
    s.m = ui->spinM->value();

    if (s.n <= 0 || s.m <= 0) {
        ui->txtLog->append("Vui lòng nhập số tiến trình và tài nguyên hợp lệ!");
        return;
    }

    s.allocation.assign(s.n, std::vector<int>(s.m, 0));
    s.request.assign(s.n, std::vector<int>(s.m, 0)); // Deadlock dùng Request
    s.available.assign(s.m, 0);

    //Đọc dữ liệu từ bảng Allocation
    for (int i = 0; i < s.n; ++i) {
        for (int j = 0; j < s.m; ++j) {
            QTableWidgetItem *item = ui->Allocation_table->item(i, j);
            s.allocation[i][j] = item ? item->text().toInt() : 0;
        }
    }

    //Đọc dữ liệu từ bảng Request
    for (int i = 0; i < s.n; ++i) {
        for (int j = 0; j < s.m; ++j) {
            QTableWidgetItem *item = ui->Request_table->item(i, j);
            s.request[i][j] = item ? item->text().toInt() : 0;
        }
    }

    //Đọc dữ liệu từ bảng Available
    for (int j = 0; j < s.m; ++j) {
        QTableWidgetItem *item = ui->Available_table->item(0, j);
        s.available[j] = item ? item->text().toInt() : 0;
    }

    //Gọi thuật toán phát hiện Deadlock từ file DeadlockMock.h
    DetectionResult res = deadlockDetection(s);

    //In kết quả ra khung txtLog để kiểm tra
    ui->txtLog->append("--- BẮT ĐẦU QUÉT PHÁT HIỆN DEADLOCK ---");
    for (const std::string& logMsg : res.trace) {
        ui->txtLog->append(QString::fromStdString(logMsg));
    }

    if (!res.deadlocked.empty()) {
        QString deadStr = "==> Danh sách tiến trình bị kẹt: ";
        for (int pid : res.deadlocked) {
            deadStr += "P" + QString::number(pid) + " ";
        }
        ui->txtLog->append(deadStr);
    }
}

//------------------------------------------------------------------------------------------------------

void MainWindow::on_Recover_bt_clicked()
{
    State s;
    s.n = ui->spinN->value();
    s.m = ui->spinM->value();

    s.allocation.assign(s.n, std::vector<int>(s.m, 0));
    s.request.assign(s.n, std::vector<int>(s.m, 0));
    s.available.assign(s.m, 0);

    for (int i = 0; i < s.n; ++i) {
        for (int j = 0; j < s.m; ++j) {
            if (auto *item = ui->Allocation_table->item(i, j))
                s.allocation[i][j] = item->text().toInt();
            if (auto *item = ui->Request_table->item(i, j))
                s.request[i][j] = item->text().toInt();
        }
    }
    for (int j = 0; j < s.m; ++j) {
        if (auto *item = ui->Available_table->item(0, j))
            s.available[j] = item->text().toInt();
    }

    //Chạy phát hiện deadlock trước
    DetectionResult res = deadlockDetection(s);

    if (res.deadlocked.empty()) {
        ui->txtLog->append("Hệ thống không có Deadlock, không cần thu hồi!");
        return;
    }

    //Tiến hành thu hồi tiến trình bị kẹt đầu tiên tìm thấy
    int victimPid = res.deadlocked[0];
    recoverTerminate(s, victimPid);

    ui->txtLog->append("Đã thu hồi/terminate tiến trình P" + QString::number(victimPid) + " để giải cứu hệ thống!");

    //Cập nhật ngược lại dữ liệu mới lên QTableWidget để người dùng nhìn thấy trực quan
    for (int j = 0; j < s.m; ++j) {
        // Cập nhật bảng Available lên dòng 0, cột j
        if (auto *item = ui->Available_table->item(0, j)) {
            item->setText(QString::number(s.available[j]));
        } else {
            ui->Available_table->setItem(0, j, new QTableWidgetItem(QString::number(s.available[j])));
        }
    }

    for (int i = 0; i < s.n; ++i) {
        for (int j = 0; j < s.m; ++j) {
            //Cập nhật bảng Allocation
            if (auto *item = ui->Allocation_table->item(i, j)) {
                item->setText(QString::number(s.allocation[i][j]));
            } else {
                ui->Allocation_table->setItem(i, j, new QTableWidgetItem(QString::number(s.allocation[i][j])));
            }
        }
    }

    ui->txtLog->append("Đã cập nhật lại trạng thái các bảng trên giao diện sau khi thu hồi!");
}

//-----------------------------------------------------------------------------------------------------------------------

void MainWindow::on_BankerRequest_bt_clicked()
{
    State s;
    s.n = ui->spinN->value();
    s.m = ui->spinM->value();

    //Cấp phát kích thước
    s.allocation.assign(s.n, std::vector<int>(s.m, 0));
    s.maximum.assign(s.n, std::vector<int>(s.m, 0));
    s.available.assign(s.m, 0);

    //Đọc dữ liệu từ bảng Allocation và Max
    for (int i = 0; i < s.n; ++i) {
        for (int j = 0; j < s.m; ++j) {
            if (auto *item = ui->Allocation_table->item(i, j))
                s.allocation[i][j] = item->text().toInt();
            if (auto *item = ui->Max_table->item(i, j))
                s.maximum[i][j] = item->text().toInt();
        }
    }

    //Đọc dữ liệu từ bảng Available
    for (int j = 0; j < s.m; ++j) {
        if (auto *item = ui->Available_table->item(0, j))
            s.available[j] = item->text().toInt();
    }

    //Tự động xác định tiến trình đang Request
    //Ưu tiên lấy dòng đang được click chọn trên bảng Request_table,
    //nếu không chọn thì tự động tìm dòng có nhập giá trị khác 0
    int targetPid = 0;
    QModelIndexList selectedIndexes = ui->Request_table->selectionModel()->selectedIndexes();
    if (!selectedIndexes.isEmpty()) {
        targetPid = selectedIndexes.first().row();
    } else {
        // Tự quét xem dòng nào có giá trị request > 0
        bool found = false;
        for (int i = 0; i < s.n; ++i) {
            for (int j = 0; j < s.m; ++j) {
                if (auto *item = ui->Request_table->item(i, j)) {
                    if (item->text().toInt() > 0) {
                        targetPid = i;
                        found = true;
                        break;
                    }
                }
            }
            if (found) break;
        }
    }

    //Đọc mảng request của tiến trình targetPid vừa tìm được
    std::vector<int> req(s.m, 0);
    for (int j = 0; j < s.m; ++j) {
        if (auto *item = ui->Request_table->item(targetPid, j)) {
            req[j] = item->text().toInt();
        }
    }

    ui->txtLog->append("--- XỬ LÝ BANKER REQUEST CHO P" + QString::number(targetPid) + " ---");

    RequestResult res = requestResources(s, targetPid, req);

    for (const std::string &msg : res.trace) {
        ui->txtLog->append(QString::fromStdString(msg));
    }
}

//-----------------------------------------------------------------------------------------------

void MainWindow::on_LoadSample_bt_clicked()
{
    //Mở hộp thoại cho phép người dùng chọn file CSV
    QString filePath = QFileDialog::getOpenFileName(this,
                                                    "Chọn file dữ liệu CSV", "", "CSV Files (*.csv);;All Files (*)");

    if (filePath.isEmpty()) {
        return; // Người dùng bấm Cancel
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Lỗi", "Không thể mở file CSV vừa chọn!");
        return;
    }

    QTextStream in(&file);

    //Đọc số tiến trình (n) và số tài nguyên (m) từ dòng đầu tiên của file (ví dụ: định dạng "n,m")
    if (in.atEnd()) return;
    QString headerLine = in.readLine();
    QStringList headerParts = headerLine.split(',');
    if (headerParts.size() < 2) {
        QMessageBox::warning(this, "Lỗi định dạng", "Dòng đầu tiên của file CSV phải chứa số tiến trình và số tài nguyên (ví dụ: 3,3)");
        file.close();
        return;
    }

    int n = headerParts[0].toInt();
    int m = headerParts[1].toInt();

    //Cập nhật giá trị lên giao diện
    ui->spinN->setValue(n);
    ui->spinM->setValue(m);

    //Cập nhật kích thước các bảng
    ui->Allocation_table->setRowCount(n);
    ui->Allocation_table->setColumnCount(m);
    ui->Max_table->setRowCount(n);
    ui->Max_table->setColumnCount(m);
    ui->Request_table->setRowCount(n);
    ui->Request_table->setColumnCount(m);
    ui->Available_table->setRowCount(1);
    ui->Available_table->setColumnCount(m);

    //Đọc ma trận Allocation (n dòng tiếp theo)
    for (int i = 0; i < n; ++i) {
        if (in.atEnd()) break;
        QString line = in.readLine();
        QStringList vals = line.split(',');
        for (int j = 0; j < m && j < vals.size(); ++j) {
            ui->Allocation_table->setItem(i, j, new QTableWidgetItem(vals[j].trimmed()));
        }
    }

    //Đọc ma trận Max (n dòng tiếp theo)
    for (int i = 0; i < n; ++i) {
        if (in.atEnd()) break;
        QString line = in.readLine();
        QStringList vals = line.split(',');
        for (int j = 0; j < m && j < vals.size(); ++j) {
            ui->Max_table->setItem(i, j, new QTableWidgetItem(vals[j].trimmed()));
        }
    }

    //Đọc ma trận Request (n dòng tiếp theo)
    for (int i = 0; i < n; ++i) {
        if (in.atEnd()) break;
        QString line = in.readLine();
        QStringList vals = line.split(',');
        for (int j = 0; j < m && j < vals.size(); ++j) {
            ui->Request_table->setItem(i, j, new QTableWidgetItem(vals[j].trimmed()));
        }
    }

    //Đọc mảng Available (1 dòng cuối)
    if (!in.atEnd()) {
        QString line = in.readLine();
        QStringList vals = line.split(',');
        for (int j = 0; j < m && j < vals.size(); ++j) {
            ui->Available_table->setItem(0, j, new QTableWidgetItem(vals[j].trimmed()));
        }
    }

    file.close();
    ui->txtLog->append("\n Đã đọc thành công dữ liệu từ file CSV: " + filePath);
}

