#include "returndialog.h"
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QDateTime>
#include <QApplication>
#include <climits>
#include "../sqlite/database.h"

ReturnDialog::ReturnDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("销售退货管理");
    resize(800, 600);
    initUI();
    updateTransactionList();
    updateProductList();
}

ReturnDialog::~ReturnDialog()
{
    // Qt的布局会自动处理UI组件的释放
}

void ReturnDialog::initUI()
{
    // 创建主布局
    auto* mainLayout = new QVBoxLayout(this);
    
    // 1. 交易信息区域
    auto* transactionGroup = new QGroupBox("交易信息");
    auto* transactionLayout = new QVBoxLayout(transactionGroup);
    
    auto* transactionTopLayout = new QHBoxLayout();
    auto* transactionIdLabel = new QLabel("交易ID:");
    m_transactionIdEdit = new QLineEdit();
    m_transactionIdEdit->setValidator(new QIntValidator(1, INT_MAX, this));
    m_transactionIdEdit->setPlaceholderText("请输入交易ID或从列表中选择");
    
    transactionTopLayout->addWidget(transactionIdLabel);
    transactionTopLayout->addWidget(m_transactionIdEdit);
    transactionTopLayout->addStretch();
    
    // 交易表格
    m_transactionTable = new QTableWidget();
    m_transactionTable->setColumnCount(6);
    m_transactionTable->setHorizontalHeaderLabels({"交易ID", "创建时间", "是否支付", "总金额", "实付金额", "找零"});
    m_transactionTable->horizontalHeader()->setStretchLastSection(true);
    m_transactionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_transactionTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    transactionLayout->addLayout(transactionTopLayout);
    transactionLayout->addWidget(m_transactionTable);
    
    // 2. 退货信息区域
    auto* returnGroup = new QGroupBox("退货信息");
    auto* returnLayout = new QVBoxLayout(returnGroup);
    
    // 退货信息表单
    auto* formLayout = new QGridLayout();
    
    // 商品选择
    auto* productLabel = new QLabel("商品:");
    m_productComboBox = new QComboBox();
    formLayout->addWidget(productLabel, 0, 0);
    formLayout->addWidget(m_productComboBox, 0, 1, 1, 2);
    
    // 退货数量
    auto* quantityLabel = new QLabel("退货数量:");
    m_returnQuantityEdit = new QLineEdit();
    m_returnQuantityEdit->setValidator(new QIntValidator(1, INT_MAX, this));
    m_returnQuantityEdit->setPlaceholderText("请输入退货数量");
    formLayout->addWidget(quantityLabel, 1, 0);
    formLayout->addWidget(m_returnQuantityEdit, 1, 1, 1, 2);
    
    // 退货原因
    auto* reasonLabel = new QLabel("退货原因:");
    m_returnReasonEdit = new QTextEdit();
    m_returnReasonEdit->setPlaceholderText("请输入退货原因");
    m_returnReasonEdit->setMaximumHeight(100);
    formLayout->addWidget(reasonLabel, 2, 0, Qt::AlignTop);
    formLayout->addWidget(m_returnReasonEdit, 2, 1, 1, 2);
    
    returnLayout->addLayout(formLayout);
    
    // 交易商品列表
    auto* productGroup = new QGroupBox("交易商品列表");
    auto* productLayout = new QVBoxLayout(productGroup);
    
    m_productTable = new QTableWidget();
    m_productTable->setColumnCount(4);
    m_productTable->setHorizontalHeaderLabels({"商品ID", "商品名称", "单价", "剩余可退货数量"});
    m_productTable->horizontalHeader()->setStretchLastSection(true);
    m_productTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_productTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    productLayout->addWidget(m_productTable);
    
    // 4. 按钮区域
    auto* buttonLayout = new QHBoxLayout();
    
    m_refreshButton = new QPushButton("刷新");
    m_returnButton = new QPushButton("确认退货");
    m_cancelButton = new QPushButton("取消");
    
    buttonLayout->addWidget(m_refreshButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_returnButton);
    buttonLayout->addWidget(m_cancelButton);
    
    // 添加所有组件到主布局
    mainLayout->addWidget(transactionGroup);
    mainLayout->addWidget(returnGroup);
    mainLayout->addWidget(productGroup);
    mainLayout->addLayout(buttonLayout);
    
    // 连接信号与槽
    connect(m_returnButton, &QPushButton::clicked, this, &ReturnDialog::onReturnClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &ReturnDialog::onCancelClicked);
    connect(m_refreshButton, &QPushButton::clicked, this, &ReturnDialog::onRefreshClicked);
    connect(m_transactionTable, &QTableWidget::clicked, this, &ReturnDialog::onTransactionTableClicked);
    connect(m_productTable, &QTableWidget::doubleClicked, this, &ReturnDialog::onProductTableDoubleClicked);
}

void ReturnDialog::updateTransactionList()
{
    loadTransactions();
}

void ReturnDialog::updateProductList()
{
    // 清空商品下拉列表
    m_productComboBox->clear();
    
    // 加载所有商品
    const std::vector<Product> products = get_all_products();
    for (const auto& product : products)
    {
        m_productComboBox->addItem(QString::fromStdString(product.name), product.id);
    }
}

void ReturnDialog::loadTransactions() const
{
    // 清空表格
    m_transactionTable->setRowCount(0);
    
    // 从数据库获取所有交易记录
    const std::vector<Transaction> transactions = get_all_transactions();
    
    // 添加到表格
    for (const auto& transaction : transactions)
    {
        const int row = m_transactionTable->rowCount();
        m_transactionTable->insertRow(row);
        
        // 交易ID
        auto* idItem = new QTableWidgetItem(QString::number(transaction.transaction_id));
        idItem->setTextAlignment(Qt::AlignCenter);
        m_transactionTable->setItem(row, 0, idItem);
        
        // 创建时间
        QDateTime dateTime = QDateTime::fromSecsSinceEpoch(transaction.create_time);
        auto* timeItem = new QTableWidgetItem(dateTime.toString("yyyy-MM-dd HH:mm:ss"));
        timeItem->setTextAlignment(Qt::AlignCenter);
        m_transactionTable->setItem(row, 1, timeItem);
        
        // 是否支付
        auto* paidItem = new QTableWidgetItem(transaction.is_paid ? "已支付" : "未支付");
        paidItem->setTextAlignment(Qt::AlignCenter);
        m_transactionTable->setItem(row, 2, paidItem);
        
        // 总金额
        auto* totalItem = new QTableWidgetItem(QString::asprintf("%.2f", transaction.total_price));
        totalItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_transactionTable->setItem(row, 3, totalItem);
        
        // 实付金额
        auto* paidAmountItem = new QTableWidgetItem(QString::asprintf("%.2f", transaction.amount_paid));
        paidAmountItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_transactionTable->setItem(row, 4, paidAmountItem);
        
        // 找零
        auto* changeItem = new QTableWidgetItem(QString::asprintf("%.2f", transaction.change));
        changeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_transactionTable->setItem(row, 5, changeItem);
    }
    
    // 调整列宽
    for (int i = 0; i < m_transactionTable->columnCount() - 1; ++i)
    {
        m_transactionTable->resizeColumnToContents(i);
    }
}

void ReturnDialog::loadTransactionProducts(int transactionId) const
{
    // 清空表格
    m_productTable->setRowCount(0);
    
    // 从数据库获取交易商品
    const std::vector<CartItem> cartItems = get_cart_items_by_transaction_id(transactionId);
    
    // 添加到表格
    for (const auto& item : cartItems)
    {
        const int row = m_productTable->rowCount();
        m_productTable->insertRow(row);
        
        // 商品ID
        auto* idItem = new QTableWidgetItem(QString::number(item.product.id));
        idItem->setTextAlignment(Qt::AlignCenter);
        m_productTable->setItem(row, 0, idItem);
        
        // 商品名称
        auto* nameItem = new QTableWidgetItem(QString::fromStdString(item.product.name));
        nameItem->setTextAlignment(Qt::AlignCenter);
        m_productTable->setItem(row, 1, nameItem);
        
        // 单价
        auto* priceItem = new QTableWidgetItem(QString::asprintf("%.2f", item.product.price));
        priceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_productTable->setItem(row, 2, priceItem);
        
        // 剩余可退货数量
        int remainingQuantity = item.quantity - item.returned_quantity;
        auto* quantityItem = new QTableWidgetItem(QString::number(remainingQuantity));
        quantityItem->setTextAlignment(Qt::AlignCenter);
        m_productTable->setItem(row, 3, quantityItem);
    }
    
    // 调整列宽
    for (int i = 0; i < m_productTable->columnCount() - 1; ++i)
    {
        m_productTable->resizeColumnToContents(i);
    }
}

bool ReturnDialog::validateInput() const
{
    // 验证交易ID - 允许为空，在执行退货时再验证
    const int transactionId = m_transactionIdEdit->text().toInt();
    if (transactionId <= 0)
    {
        QMessageBox::warning(const_cast<ReturnDialog*>(this), "警告", "请输入有效的交易ID");
        return false;
    }
    
    // 验证商品选择
    const int productId = m_productComboBox->currentData().toInt();
    if (productId <= 0)
    {
        QMessageBox::warning(const_cast<ReturnDialog*>(this), "警告", "请选择要退货的商品");
        return false;
    }
    
    // 验证退货数量
    bool conversionOk;
    const long long returnQuantityLL = m_returnQuantityEdit->text().toLongLong(&conversionOk);
    if (!conversionOk || returnQuantityLL <= 0 || returnQuantityLL > INT_MAX)
    {
        QMessageBox::warning(const_cast<ReturnDialog*>(this), "警告", "请输入有效的退货数量");
        return false;
    }
    
    return true;
}

void ReturnDialog::processReturn()
{
    if (!validateInput())
    {
        return;
    }
    
    // 获取输入数据
    const int transactionId = m_transactionIdEdit->text().toInt();
    const int productId = m_productComboBox->currentData().toInt();
    const int returnQuantity = m_returnQuantityEdit->text().toInt();
    const std::string reason = m_returnReasonEdit->toPlainText().toStdString();
    
    // 检查交易是否存在
    const std::vector<Transaction> transactions = get_all_transactions();
    const auto transactionIt = std::find_if(transactions.begin(), transactions.end(),
        [transactionId](const Transaction& t) { return t.transaction_id == transactionId; });
    
    if (transactionIt == transactions.end())
    {
        QMessageBox::warning(this, "警告", "交易不存在");
        return;
    }
    
    // 检查该商品是否在交易中
    const std::vector<CartItem> cartItems = get_cart_items_by_transaction_id(transactionId);
    const auto cartItemIt = std::find_if(cartItems.begin(), cartItems.end(),
        [productId](const CartItem& item) { return item.product.id == productId; });
    
    if (cartItemIt == cartItems.end())
    {
        QMessageBox::warning(this, "警告", "该商品不在所选交易中");
        return;
    }
    
    // 计算剩余可退货数量
    int remainingReturnable = cartItemIt->quantity - cartItemIt->returned_quantity;
    
    // 检查退货数量是否超过剩余可退货数量
    if (returnQuantity > remainingReturnable)
    {
        QMessageBox::warning(this, "警告",
            QString("退货数量不能超过剩余可退货数量！\n购买数量: %1\n已退货: %2\n剩余可退货: %3")
            .arg(cartItemIt->quantity)
            .arg(cartItemIt->returned_quantity)
            .arg(remainingReturnable));
        return;
    }
    
    // 执行退货操作
    if (add_return(transactionId, productId, returnQuantity, reason))
    {
        // 退货成功
        QMessageBox::information(this, "提示",
            QString("商品 '%1' 退货成功！\n退货数量: %2")
            .arg(QString::fromStdString(cartItemIt->product.name))
            .arg(returnQuantity));
        
        // 关闭窗口
        accept();
    }
    else
    {
        // 退货失败
        QMessageBox::critical(this, "错误", "商品退货失败，请检查数据库连接");
    }
}

void ReturnDialog::onReturnClicked()
{
    processReturn();
}

void ReturnDialog::onCancelClicked()
{
    // 关闭对话框
    reject();
}

void ReturnDialog::onRefreshClicked()
{
    // 刷新交易列表和商品列表
    updateTransactionList();
    updateProductList();
    QMessageBox::information(this, "提示", "列表已刷新");
}

void ReturnDialog::onTransactionTableClicked(const QModelIndex& index)
{
    // 获取选中的交易ID
    const int row = index.row();
    QTableWidgetItem* idItem = m_transactionTable->item(row, 0);
    if (idItem)
    {
        const int transactionId = idItem->text().toInt();
        m_transactionIdEdit->setText(QString::number(transactionId));
        
        // 加载该交易的商品
        loadTransactionProducts(transactionId);
    }
}

void ReturnDialog::onProductTableDoubleClicked(const QModelIndex& index)
{
    // 获取双击的行号
    const int row = index.row();
    
    // 获取商品ID
    QTableWidgetItem* idItem = m_productTable->item(row, 0);
    if (!idItem) return;
    
    const int productId = idItem->text().toInt();
    
    // 获取商品名称
    QTableWidgetItem* nameItem = m_productTable->item(row, 1);
    if (!nameItem) return;
    
    const QString productName = nameItem->text();
    
    // 在商品下拉列表中选择对应的商品
    const int comboIndex = m_productComboBox->findText(productName);
    if (comboIndex != -1)
    {
        m_productComboBox->setCurrentIndex(comboIndex);
        
        // 将焦点设置到退货数量输入框
        m_returnQuantityEdit->setFocus();
        m_returnQuantityEdit->selectAll();
    }
}

void ReturnDialog::setTransactionId(int transactionId)
{
    // 设置交易ID到输入框
    m_transactionIdEdit->setText(QString::number(transactionId));
    
    // 自动加载交易商品
    loadTransactionProducts(transactionId);
}