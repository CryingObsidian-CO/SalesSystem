#include "historydialog.h"
#include "ui_historydialog.h"
#include "returndialog.h"
#include "../sqlite/database.h"
#include "../sale/saleStruct.h"
#include <QStandardItemModel>
#include <QMessageBox>
#include <QDateTime>

HistoryDialog::HistoryDialog(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::HistoryDialog)
{
    ui->setupUi(this);
    this->setWindowTitle("商家历史记录");

    // 设置表格模型
    QStandardItemModel* transactionModel = new QStandardItemModel(0, 6, this);
    transactionModel->setHorizontalHeaderLabels({"交易ID", "交易时间", "是否支付", "总金额", "支付金额", "找零"});
    ui->transactionTable->setModel(transactionModel);
    ui->transactionTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // 设置详情表格模型
    QStandardItemModel* detailModel = new QStandardItemModel(0, 5, this);
    detailModel->setHorizontalHeaderLabels({"商品ID", "商品名称", "单价", "数量", "小计"});
    ui->detailTable->setModel(detailModel);
    ui->detailTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // 手动连接信号和槽
    connect(ui->transactionTable, &QTableView::doubleClicked, this,
            &HistoryDialog::on_transactionTable_doubleClicked);

    // 加载交易记录
    loadTransactions();

    // 检查低库存
    checkLowStock();
}

HistoryDialog::~HistoryDialog()
{
    delete ui;
}

void HistoryDialog::loadTransactions()
{
    auto transactions = get_all_transactions();
    QStandardItemModel* model = static_cast<QStandardItemModel*>(ui->transactionTable->model());
    model->setRowCount(0);

    // 计算总交易金额
    double totalAmount = 0.0;

    for (const auto& transaction : transactions)
    {
        QList<QStandardItem*> row;

        // 交易ID
        row << new QStandardItem(QString::number(transaction.transaction_id));

        // 交易时间
        QDateTime dateTime = QDateTime::fromSecsSinceEpoch(transaction.create_time);
        row << new QStandardItem(dateTime.toString("yyyy-MM-dd hh:mm:ss"));

        // 是否支付
        row << new QStandardItem(transaction.is_paid ? "已支付" : "未支付");

        // 总金额
        row << new QStandardItem(QString::asprintf("%.2f", transaction.total_price));

        // 支付金额
        row << new QStandardItem(QString::asprintf("%.2f", transaction.amount_paid));

        // 找零
        row << new QStandardItem(QString::asprintf("%.2f", transaction.change));

        model->appendRow(row);

        // 累加总金额（只统计已支付的交易）
        if (transaction.is_paid) {
            totalAmount += transaction.total_price;
        }
    }

    // 显示总交易金额
    ui->totalAmountLabel->setText(QString::asprintf("¥%.2f", totalAmount));
}

void HistoryDialog::on_transactionTable_doubleClicked(const QModelIndex& index)
{
    if (index.isValid())
    {
        int row = index.row();
        QStandardItemModel* model = static_cast<QStandardItemModel*>(ui->transactionTable->model());
        int transactionId = model->item(row, 0)->text().toInt();
        showTransactionDetails(transactionId);
    }
}

void HistoryDialog::showTransactionDetails(const int transactionId) const
{
    auto cartItems = get_cart_items_by_transaction_id(transactionId);
    auto returnItems = get_returns_by_transaction_id(transactionId);
    QStandardItemModel* model = static_cast<QStandardItemModel*>(ui->detailTable->model());
    model->setRowCount(0);

    // 设置表格列标题，增加已退货数量和剩余数量列
    model->setHorizontalHeaderLabels({"商品ID", "商品名称", "单价", "购买数量", "已退货数量", "剩余数量", "小计", "退货时间", "退货原因"});

    // 显示商品基本信息
    for (const auto& item : cartItems)
    {
        QList<QStandardItem*> row;

        // 商品ID
        auto* idItem = new QStandardItem(QString::number(item.product.id));
        row << idItem;

        // 商品名称
        auto* nameItem = new QStandardItem(QString::fromStdString(item.product.name));
        row << nameItem;

        // 单价
        auto* priceItem = new QStandardItem(QString::asprintf("%.2f", item.product.price));
        row << priceItem;

        // 购买数量
        auto* quantityItem = new QStandardItem(QString::number(item.quantity));
        row << quantityItem;

        // 已退货数量
        auto* returnedItem = new QStandardItem(QString::number(item.returned_quantity));
        // 如果有已退货数量，设置为红色
        if (item.returned_quantity > 0) {
            returnedItem->setForeground(QBrush(Qt::red));
        }
        row << returnedItem;

        // 剩余数量
        int remainingQuantity = item.quantity - item.returned_quantity;
        auto* remainingItem = new QStandardItem(QString::number(remainingQuantity));
        row << remainingItem;

        // 小计
        auto* subtotalItem = new QStandardItem(QString::asprintf("%.2f", item.subtotal));
        row << subtotalItem;

        // 退货时间和原因（主商品行留空）
        row << new QStandardItem("");
        row << new QStandardItem("");

        // 将行添加到模型
        model->appendRow(row);

        // 为每条退货记录生成单独的行
        for (const auto& returnItem : returnItems) {
            if (returnItem.product_id == item.product.id) {
                QList<QStandardItem*> returnRow;
                returnRow << new QStandardItem("");
                returnRow << new QStandardItem(QString("→ 退货") + QString::fromStdString(item.product.name));
                returnRow << new QStandardItem(QString::asprintf("%.2f", item.product.price));
                returnRow << new QStandardItem("");
                returnRow << new QStandardItem("-"); // 已退货数量列显示"-"
                returnRow << new QStandardItem(QString::number(returnItem.quantity)); // 剩余数量列显示本次退货数量
                returnRow << new QStandardItem(QString::asprintf("-%.2f", item.product.price * returnItem.quantity));
                
                // 退货时间
                QDateTime returnTime = QDateTime::fromSecsSinceEpoch(returnItem.return_time);
                returnRow << new QStandardItem(returnTime.toString("yyyy-MM-dd HH:mm:ss"));
                
                // 退货原因
                returnRow << new QStandardItem(QString::fromStdString(returnItem.reason));
                
                // 设置退货行的样式
                for (auto* itemWidget : returnRow) {
                    itemWidget->setForeground(QBrush(Qt::red));
                    QFont font = itemWidget->font();
                    font.setItalic(true);
                    itemWidget->setFont(font);
                }
                
                model->appendRow(returnRow);
            }
        }
    }
}

void HistoryDialog::on_refreshButton_clicked()
{
    loadTransactions();
    checkLowStock();
    QMessageBox::information(this, "刷新成功", "交易记录已刷新");
}

void HistoryDialog::checkLowStock()
{
    const auto lowStockProducts = get_low_stock_products();
    if (!lowStockProducts.empty())
    {
        showLowStockWarning();
    }
}

void HistoryDialog::showLowStockWarning()
{
    const auto lowStockProducts = get_low_stock_products();
    if (lowStockProducts.empty())
        return;

    QString warningText = "以下商品库存过低，需要补货：\n\n";
    for (const auto& product : lowStockProducts)
    {
        warningText += QString::fromStdString(product.name) + ": 库存 " + QString::number(
            product.stock) + " (预警阈值: " + QString::number(get_product_alert_threshold(product.id)) + ")\n";
    }

    QMessageBox::warning(this, "库存警告", warningText);
}

void HistoryDialog::on_returnButton_clicked()
{
    // 获取当前选中的交易ID（可选）
    QModelIndexList selectedRows = ui->transactionTable->selectionModel()->selectedRows();
    int transactionId = 0;
    
    // 如果有选中的交易记录，则获取交易ID
    if (!selectedRows.isEmpty())
    {
        int selectedRow = selectedRows.first().row();
        QStandardItemModel* model = static_cast<QStandardItemModel*>(ui->transactionTable->model());
        transactionId = model->item(selectedRow, 0)->text().toInt();
    }

    // 创建并显示退货对话框
    ReturnDialog dialog(this);
    if (transactionId > 0)
    {
        dialog.setTransactionId(transactionId);
    }
    dialog.exec();

    // 刷新交易记录
    loadTransactions();
}

void HistoryDialog::on_returnRecordButton_clicked()
{
    // 获取所有退货记录
    const auto returns = get_all_returns();
    
    // 创建退货记录对话框
    QDialog* returnDialog = new QDialog(this);
    returnDialog->setWindowTitle("退货记录");
    returnDialog->resize(800, 600);
    
    // 创建布局
    auto* layout = new QVBoxLayout(returnDialog);
    
    // 创建表格视图
    QTableView* returnTable = new QTableView(returnDialog);
    returnTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    returnTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    returnTable->horizontalHeader()->setStretchLastSection(true);
    
    // 创建模型
    QStandardItemModel* model = new QStandardItemModel(0, 7, returnDialog);
    model->setHorizontalHeaderLabels({"退货ID", "交易ID", "商品ID", "商品名称", "退货数量", "退货金额", "退货时间"});
    
    // 添加数据
    for (const auto& returnItem : returns)
    {
        // 获取商品信息
        Product product = query_product(returnItem.product_id);
        if (product.id == -1)
            continue;
        
        // 计算退货金额
        float returnAmount = product.price * returnItem.quantity;
        
        QList<QStandardItem*> row;
        
        // 退货ID
        row << new QStandardItem(QString::number(returnItem.return_id));
        // 交易ID
        row << new QStandardItem(QString::number(returnItem.transaction_id));
        // 商品ID
        row << new QStandardItem(QString::number(returnItem.product_id));
        // 商品名称
        row << new QStandardItem(QString::fromStdString(product.name));
        // 退货数量
        row << new QStandardItem(QString::number(returnItem.quantity));
        // 退货金额
        row << new QStandardItem(QString::asprintf("%.2f", returnAmount));
        // 退货时间
        QDateTime returnTime = QDateTime::fromSecsSinceEpoch(returnItem.return_time);
        row << new QStandardItem(returnTime.toString("yyyy-MM-dd HH:mm:ss"));
        
        model->appendRow(row);
    }
    
    returnTable->setModel(model);
    layout->addWidget(returnTable);
    
    // 添加刷新按钮
    auto* buttonLayout = new QHBoxLayout();
    auto* refreshButton = new QPushButton("刷新", returnDialog);
    buttonLayout->addStretch();
    buttonLayout->addWidget(refreshButton);
    layout->addLayout(buttonLayout);
    
    // 连接信号槽
    connect(refreshButton, &QPushButton::clicked, [=]() {
        // 重新加载退货记录
        model->setRowCount(0);
        const auto updatedReturns = get_all_returns();
        for (const auto& returnItem : updatedReturns)
        {
            Product product = query_product(returnItem.product_id);
            if (product.id == -1)
                continue;
            
            float returnAmount = product.price * returnItem.quantity;
            
            QList<QStandardItem*> row;
            row << new QStandardItem(QString::number(returnItem.return_id));
            row << new QStandardItem(QString::number(returnItem.transaction_id));
            row << new QStandardItem(QString::number(returnItem.product_id));
            row << new QStandardItem(QString::fromStdString(product.name));
            row << new QStandardItem(QString::number(returnItem.quantity));
            row << new QStandardItem(QString::asprintf("%.2f", returnAmount));
            QDateTime returnTime = QDateTime::fromSecsSinceEpoch(returnItem.return_time);
            row << new QStandardItem(returnTime.toString("yyyy-MM-dd HH:mm:ss"));
            
            model->appendRow(row);
        }
    });
    
    // 双击查看交易详情
    connect(returnTable, &QTableView::doubleClicked, [=](const QModelIndex& index) {
        if (index.isValid())
        {
            int row = index.row();
            int transactionId = model->item(row, 1)->text().toInt();
            
            // 显示交易详情
            QDialog* detailDialog = new QDialog(returnDialog);
            detailDialog->setWindowTitle(QString("交易详情 - ID: %1").arg(transactionId));
            detailDialog->resize(800, 600);
            
            auto* detailLayout = new QVBoxLayout(detailDialog);
            
            // 显示交易基本信息
            auto* basicInfoGroup = new QGroupBox("交易基本信息", detailDialog);
            auto* basicInfoLayout = new QVBoxLayout(basicInfoGroup);
            
            const auto transactions = get_all_transactions();
            const auto transactionIt = std::find_if(transactions.begin(), transactions.end(),
                [transactionId](const Transaction& t) { return t.transaction_id == transactionId; });
            
            if (transactionIt != transactions.end())
            {
                QDateTime transactionTime = QDateTime::fromSecsSinceEpoch(transactionIt->create_time);
                
                QString basicInfo = QString("交易ID: %1\n交易时间: %2\n是否支付: %3\n总金额: %.2f\n支付金额: %.2f\n找零: %.2f")
                    .arg(transactionIt->transaction_id)
                    .arg(transactionTime.toString("yyyy-MM-dd HH:mm:ss"))
                    .arg(transactionIt->is_paid ? "已支付" : "未支付")
                    .arg(transactionIt->total_price)
                    .arg(transactionIt->amount_paid)
                    .arg(transactionIt->change);
                
                auto* infoLabel = new QLabel(basicInfo, basicInfoGroup);
                basicInfoLayout->addWidget(infoLabel);
            }
            detailLayout->addWidget(basicInfoGroup);
            
            // 显示交易商品详情
            auto* productGroup = new QGroupBox("交易商品详情", detailDialog);
            auto* productLayout = new QVBoxLayout(productGroup);
            
            auto* productTable = new QTableView(productGroup);
            auto* productModel = new QStandardItemModel(0, 7, productGroup);
            productModel->setHorizontalHeaderLabels({"商品ID", "商品名称", "单价", "购买数量", "已退货数量", "剩余数量", "小计"});
            
            const auto cartItems = get_cart_items_by_transaction_id(transactionId);
            for (const auto& item : cartItems)
            {
                QList<QStandardItem*> productRow;
                productRow << new QStandardItem(QString::number(item.product.id));
                productRow << new QStandardItem(QString::fromStdString(item.product.name));
                productRow << new QStandardItem(QString::asprintf("%.2f", item.product.price));
                productRow << new QStandardItem(QString::number(item.quantity));
                productRow << new QStandardItem(QString::number(item.returned_quantity));
                productRow << new QStandardItem(QString::number(item.quantity - item.returned_quantity));
                productRow << new QStandardItem(QString::asprintf("%.2f", item.subtotal));
                
                productModel->appendRow(productRow);
            }
            
            productTable->setModel(productModel);
            productTable->horizontalHeader()->setStretchLastSection(true);
            productLayout->addWidget(productTable);
            detailLayout->addWidget(productGroup);
            
            detailDialog->exec();
            delete detailDialog;
        }
    });
    
    returnDialog->exec();
    delete returnDialog;
}
