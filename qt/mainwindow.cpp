#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPushButton>
#include <QTableWidgetItem>
#include <QMessageBox>
#include "manualadddialog.h"
#include "settlementdialog.h"

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 初始化购物车
    m_cart.total_price = 0.0;
    m_cart.items.clear();

    connect(ui->mngm, &QPushButton::clicked, this, &MainWindow::onMngmClicked);

    // 更新购物车显示
    updateCartDisplay();
}

MainWindow::~MainWindow()
{
    // 释放UI资源
    delete ui;
}

ShoppingCart& MainWindow::getCart()
{
    return m_cart;
}


void MainWindow::updateCartDisplay()
{
    ui->productTable->setRowCount(0);
    // 遍历购物车中的商品
    for (const auto& item : m_cart.items)
    {
        const auto& product = item.product;
        const auto quantity = item.quantity;
        const auto subtotal = item.subtotal;

        // 添加新行
        const int row = ui->productTable->rowCount();
        ui->productTable->insertRow(row);

        // 商品ID
        auto* idItem = new QTableWidgetItem(QString::number(product.id));
        idItem->setFlags(idItem->flags() & ~Qt::ItemIsEditable);
        ui->productTable->setItem(row, 0, idItem);

        // 商品名称
        auto* nameItem = new
            QTableWidgetItem(QString::fromStdString(product.name));
        nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
        ui->productTable->setItem(row, 1, nameItem);

        // 单价
        auto* priceItem = new QTableWidgetItem(
            QString::number(product.price, 'f', 2));
        priceItem->setFlags(priceItem->flags() & ~Qt::ItemIsEditable);
        ui->productTable->setItem(row, 2, priceItem);

        // 小计
        auto* subtotalItem = new QTableWidgetItem(
            QString::number(subtotal, 'f', 2));
        subtotalItem->setFlags(subtotalItem->flags() & ~Qt::ItemIsEditable);
        ui->productTable->setItem(row, 3, subtotalItem);

        // 数量
        auto* quantityItem = new QTableWidgetItem(QString::number(quantity));
        quantityItem->setFlags(quantityItem->flags() & ~Qt::ItemIsEditable);
        ui->productTable->setItem(row, 4, quantityItem);
    }

    // 更新总计金额
    ui->label_totalMoney->setText(QString::number(m_cart.total_price, 'f', 2));
}

void MainWindow::onMngmClicked()
{
    // 创建并显示模拟窗口，设置为独立窗口
    auto* simWindow = new simulate(this);
    simWindow->setAttribute(Qt::WA_DeleteOnClose); // 关闭时自动删除，防止内存泄漏
    simWindow->setWindowFlags(simWindow->windowFlags() | Qt::Window); // 设置为独立窗口
    simWindow->show();
}

void MainWindow::on_sdtj_clicked()
{
    // 弹出手动添加商品对话框
    ManualAddDialog dialog(this);
    dialog.exec();
}

void MainWindow::on_qk_clicked()
{
    // 清空购物车
    m_cart.items.clear();
    m_cart.total_price = 0.0;
    updateCartDisplay();
    QMessageBox::information(this, "提示", "购物车已清空");
}

void MainWindow::on_jiesuan_clicked()
{
    // 结算购物车的逻辑
    if (m_cart.items.empty())
    {
        QMessageBox::warning(this, "提示", "购物车为空，无法结算");
        return;
    }

    // 弹出结算对话框
    SettlementDialog dialog(this, m_cart.total_price);
    dialog.exec();
}

void MainWindow::on_historyButton_clicked()
{
    // 打开历史记录窗口
    HistoryDialog dialog(this);
    dialog.exec();
}
