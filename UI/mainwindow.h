#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void on_ResizeTable_bt_clicked();

    void on_BankerSafety_bt_clicked();

    void on_DeadlockDetection_bt_clicked();

    void on_Recover_bt_clicked();

    void on_BankerRequest_bt_clicked();

    void on_LoadSample_bt_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
