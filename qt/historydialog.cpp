#include "historydialog.h"
#include "ui_historydialog.h"
#include "../sqlite/database.h"
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
    QStandardItemModel* model = static_cast<QStandardItemModel*>(ui->detailTable->model());
    model->setRowCount(0);

    for (const auto& item : cartItems)
    {
        QList<QStandardItem*> row;

        // 商品ID
        row << new QStandardItem(QString::number(item.product.id));

        // 商品名称
        row << new QStandardItem(QString::fromStdString(item.product.name));

        // 单价
        row << new QStandardItem(QString::asprintf("%.2f", item.product.price));

        // 数量
        row << new QStandardItem(QString::number(item.quantity));

        // 小计
        row << new QStandardItem(QString::asprintf("%.2f", item.subtotal));

        model->appendRow(row);
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
