//
// Created by 24863 on 2025/12/14.
//

#ifndef SALESSYSTEM_SIMULATE_H
#define SALESSYSTEM_SIMULATE_H

#include <QSpinBox>
#include <QMap>


QT_BEGIN_NAMESPACE

namespace Ui
{
    class simulate;
}

QT_END_NAMESPACE

class MainWindow;

class simulate : public QWidget
{
    Q_OBJECT

public:
    explicit simulate(QWidget* parent = nullptr);
    ~simulate() override;

private:
    Ui::simulate* ui;
    MainWindow* m_mainWindow;

    // 存储每个商品对应的QSpinBox
    QMap<int, QSpinBox*> m_spinBoxMap;

    // 存储每个商品的库存信息
    QMap<int, int> m_stockMap;

    // 更新商品表格
    void updateProductTable();

    // 检查数量是否超过库存
    void checkQuantity(int productId, int quantity);

private slots:
    void on_qx_clicked();
    void on_qd_clicked();
    void onQuantityChanged(int value, int productId);
    void on_addProductButton_clicked();
    void on_restockButton_clicked();
};


#endif //SALESSYSTEM_SIMULATE_H
