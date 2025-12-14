#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "simulate.h"
#include "saleStruct.h"

class ManualAddDialog;
class SettlementDialog;

QT_BEGIN_NAMESPACE

namespace Ui
{
    class MainWindow;
}

QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    // 获取购物车实例
    ShoppingCart& getCart();

    // 更新购物车显示
    void updateCartDisplay();

private:
    Ui::MainWindow* ui;
    ShoppingCart m_cart; // 购物车实例


private slots:
    void onMngmClicked();
    void on_sdtj_clicked();
    void on_qk_clicked();
    void on_jiesuan_clicked();
};
#endif // MAINWINDOW_H
