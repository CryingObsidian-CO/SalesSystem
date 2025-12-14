//
// Created by 24863 on 2025/12/14.
//

// You may need to build the project (run Qt uic code generator) to get "ui_simulate.h" resolved

#include "simulate.h"
#include "ui_simulate.h"
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QSpinBox>
#include "mainwindow.h"
#include "../sqlite/database.h"




simulate::simulate(QWidget* parent) :
    QWidget(parent), ui(new Ui::simulate), m_mainWindow(nullptr)
{
    ui->setupUi(this);

    // 获取主窗口指针
    if (parent)
    {
        m_mainWindow = dynamic_cast<MainWindow*>(parent);
    }

    // 初始化商品表格
    updateProductTable();
}

simulate::~simulate()
{
    delete ui;
}

void simulate::on_qx_clicked()
{
    // 取消按钮点击事件，关闭窗口
    this->close();
}

void simulate::updateProductTable()
{
    // 清空表格，显式释放资源
    ui->productTable->setRowCount(0);
    m_spinBoxMap.clear();
    m_stockMap.clear();

    // 从数据库获取所有商品
    const std::vector<Product> products = get_all_products();
    
    // 获取主窗口购物车中的商品数量信息
    QMap<int, int> cartItemQuantities;
    if (m_mainWindow) {
        const ShoppingCart& cart = m_mainWindow->getCart();
        for (const auto& cartItem : cart.items) {
            cartItemQuantities[cartItem.product.id] = cartItem.quantity;
        }
    }
    
    // 遍历商品列表，添加到表格
    for (const auto& [id, name, price,
             stock] : products)
    {
        const int row = ui->productTable->rowCount();
        ui->productTable->insertRow(row);

        // 商品ID
        auto* idItem = new QTableWidgetItem(QString::number(id));
        idItem->setFlags(idItem->flags() & ~Qt::ItemIsEditable);
        ui->productTable->setItem(row, 0, idItem);

        // 商品名称
        auto* nameItem = new QTableWidgetItem(QString::fromStdString(name));
        nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
        ui->productTable->setItem(row, 1, nameItem);

        // 单价
        auto* priceItem = new QTableWidgetItem(QString::number(price, 'f', 2));
        priceItem->setFlags(priceItem->flags() & ~Qt::ItemIsEditable);
        ui->productTable->setItem(row, 2, priceItem);

        // 库存
        auto* stockItem = new QTableWidgetItem(QString::number(stock));
        stockItem->setFlags(stockItem->flags() & ~Qt::ItemIsEditable);
        ui->productTable->setItem(row, 3, stockItem);

        // 数量（如果购物车中已有该商品，显示购物车中的数量，否则显示0）
        int quantity = 0;
        if (cartItemQuantities.contains(id)) {
            quantity = cartItemQuantities[id];
        }
        
        auto* spinBox = new QSpinBox();
        spinBox->setMinimum(0); // 数量不能为负
        spinBox->setMaximum(stock); // 最大数量限制为商品库存
        spinBox->setValue(quantity); // 设置初始数量
        spinBox->setAlignment(Qt::AlignCenter); // 居中显示
        ui->productTable->setCellWidget(row, 4, spinBox);
        
        // 存储商品ID与QSpinBox的映射
        m_spinBoxMap[id] = spinBox;
        
        // 存储商品库存信息
        m_stockMap[id] = stock;
        
        // 连接数量变化信号到槽函数
        connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), 
                this, [this, id](int value) { onQuantityChanged(value, id); });
    }
}

void simulate::checkQuantity(int productId, int quantity)
{
    // 检查数量是否超过库存
    if (m_stockMap.contains(productId)) {
        int stock = m_stockMap[productId];
        if (quantity > stock) {
            // 数量超过库存，设置为最大库存
            QSpinBox* spinBox = m_spinBoxMap[productId];
            if (spinBox) {
                spinBox->setValue(stock);
                QMessageBox::warning(this, "警告", QString("商品ID %1 的数量不能超过库存 %2，已自动调整为最大库存。").arg(productId).arg(stock));
            }
        }
    }
}

void simulate::onQuantityChanged(int value, int productId)
{
    // 当数量变化时检查是否超过库存
    checkQuantity(productId, value);
}

void simulate::on_qd_clicked()
{
    // 确定按钮点击事件，添加商品到购物车
    if (m_mainWindow) {
        // 获取主窗口的购物车
        ShoppingCart& cart = m_mainWindow->getCart();
        
        // 创建临时映射，存储所有商品的新数量
        QMap<int, int> newQuantities;
        QMap<int, Product> productMap;
        
        // 首先收集所有需要更新的商品及其数量
        for (int row = 0; row < ui->productTable->rowCount(); ++row) {
            // 获取商品ID
            QTableWidgetItem* idItem = ui->productTable->item(row, 0);
            if (!idItem) continue;
            
            int productId = idItem->text().toInt();
            
            // 获取数量
            QSpinBox* spinBox = dynamic_cast<QSpinBox*>(ui->productTable->cellWidget(row, 4));
            if (!spinBox) continue;
            
            int quantity = spinBox->value();
            
            // 如果数量大于0，记录下来
            if (quantity > 0) {
                // 从数据库获取商品信息
                Product product = query_product(productId);
                if (product.id != -1) {
                    newQuantities[productId] = quantity;
                    productMap[productId] = product;
                }
            }
        }
        
        // 更新购物车
        // 1. 先处理已有商品：更新数量或删除
        for (auto it = cart.items.begin(); it != cart.items.end(); ) {
            int productId = it->product.id;
            if (newQuantities.contains(productId)) {
                // 购物车中已有该商品，更新数量
                int newQuantity = newQuantities[productId];
                Product product = productMap[productId];
                
                // 计算旧小计和新小计
                float oldSubtotal = it->subtotal;
                float newSubtotal = product.price * newQuantity;
                
                // 更新购物车项
                it->quantity = newQuantity;
                it->subtotal = newSubtotal;
                
                // 更新总金额
                cart.total_price += (newSubtotal - oldSubtotal);
                
                // 从newQuantities中移除，剩下的就是需要新增的商品
                newQuantities.remove(productId);
                
                // 继续遍历
                ++it;
            } else {
                // 购物车中的商品不在新列表中，移除
                cart.total_price -= it->subtotal;
                it = cart.items.erase(it);
            }
        }
        
        // 2. 处理需要新增的商品
        for (auto it = newQuantities.begin(); it != newQuantities.end(); ++it) {
            int productId = it.key();
            int quantity = it.value();
            Product product = productMap[productId];
            
            // 创建购物车项
            CartItem cartItem;
            cartItem.product = product;
            cartItem.quantity = quantity;
            cartItem.subtotal = product.price * quantity;
            
            // 添加到购物车
            cart.items.push_back(cartItem);
            cart.total_price += cartItem.subtotal;
        }
        
        // 更新主窗口的购物车显示
        m_mainWindow->updateCartDisplay();
    }
    
    // 关闭模拟窗口
    this->close();
}
