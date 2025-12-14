#include "restockdialog.h"
#include <QMessageBox>
#include <QTableWidgetItem>
#include "../sqlite/database.h"

RestockDialog::RestockDialog(QWidget* parent)
    : QDialog(parent)
{
    // 设置对话框标题
    setWindowTitle("商品补货");
    // 设置对话框大小
    resize(600, 400);

    // 创建UI组件
    auto* productLabel = new QLabel("选择商品:");
    m_productComboBox = new QComboBox();
    m_productComboBox->setMinimumWidth(200);

    auto* restockQuantityLabel = new QLabel("补货数量:");
    m_restockQuantityEdit = new QLineEdit();
    m_restockQuantityEdit->setValidator(new QIntValidator(1, 999999, this));
    m_restockQuantityEdit->setPlaceholderText("请输入补货数量");
    m_restockQuantityEdit->setMinimumWidth(150);

    // 创建商品表格
    m_productTable = new QTableWidget();
    m_productTable->setColumnCount(5);
    m_productTable->setHorizontalHeaderLabels({"商品ID", "商品名称", "单价", "当前库存", "状态"});
    m_productTable->horizontalHeader()->setStretchLastSection(true);
    m_productTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_productTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 创建按钮
    m_restockButton = new QPushButton("确认补货");
    m_cancelButton = new QPushButton("取消");
    m_refreshButton = new QPushButton("刷新列表");

    // 创建布局
    auto* mainLayout = new QVBoxLayout(this);

    // 补货操作布局
    auto* restockLayout = new QHBoxLayout();
    restockLayout->addWidget(productLabel);
    restockLayout->addWidget(m_productComboBox);
    restockLayout->addWidget(restockQuantityLabel);
    restockLayout->addWidget(m_restockQuantityEdit);
    restockLayout->addWidget(m_restockButton);
    restockLayout->addWidget(m_refreshButton);
    restockLayout->addWidget(m_cancelButton);

    // 将布局添加到主布局
    mainLayout->addLayout(restockLayout);
    mainLayout->addWidget(m_productTable);

    // 连接信号与槽
    connect(m_restockButton, &QPushButton::clicked, this, &RestockDialog::onRestockClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &RestockDialog::onCancelClicked);
    connect(m_refreshButton, &QPushButton::clicked, this, &RestockDialog::onRefreshClicked);
    connect(m_productTable, &QTableWidget::doubleClicked, this, &RestockDialog::onProductTableDoubleClicked);

    // 初始化商品列表
    updateProductList();
}

RestockDialog::~RestockDialog()
{
    // 不需要手动释放UI组件，Qt的布局会自动处理
}

void RestockDialog::updateProductList()
{
    // 清空商品下拉列表
    m_productComboBox->clear();

    // 加载所有商品
    loadAllProducts();
}

void RestockDialog::loadAllProducts() const
{
    // 清空表格
    m_productTable->setRowCount(0);

    // 从数据库获取所有商品
    const std::vector<Product> products = get_all_products();

    // 添加到表格和下拉列表
    for (const auto& product : products)
    {
        // 添加到下拉列表
        m_productComboBox->addItem(QString::fromStdString(product.name), product.id);

        // 添加到表格
        const int row = m_productTable->rowCount();
        m_productTable->insertRow(row);

        // 商品ID
        auto* idItem = new QTableWidgetItem(QString::number(product.id));
        idItem->setTextAlignment(Qt::AlignCenter);
        m_productTable->setItem(row, 0, idItem);

        // 商品名称
        auto* nameItem = new QTableWidgetItem(QString::fromStdString(product.name));
        nameItem->setTextAlignment(Qt::AlignCenter);
        m_productTable->setItem(row, 1, nameItem);

        // 单价
        auto* priceItem = new QTableWidgetItem(QString::number(product.price, 'f', 2));
        priceItem->setTextAlignment(Qt::AlignCenter);
        m_productTable->setItem(row, 2, priceItem);

        // 当前库存
        auto* stockItem = new QTableWidgetItem(QString::number(product.stock));
        stockItem->setTextAlignment(Qt::AlignCenter);
        m_productTable->setItem(row, 3, stockItem);

        // 库存状态
        QString status;
        if (product.stock <= 10)
        {
            status = "低库存";
        }
        else if (product.stock <= 50)
        {
            status = "正常";
        }
        else
        {
            status = "充足";
        }
        auto* statusItem = new QTableWidgetItem(status);
        statusItem->setTextAlignment(Qt::AlignCenter);
        m_productTable->setItem(row, 4, statusItem);
    }

    // 调整列宽
    for (int i = 0; i < m_productTable->columnCount() - 1; ++i)
    {
        m_productTable->resizeColumnToContents(i);
    }
}

void RestockDialog::restockProduct()
{
    // 获取选中的商品ID
    int productId = m_productComboBox->currentData().toInt();
    if (productId <= 0)
    {
        QMessageBox::warning(this, "警告", "请选择要补货的商品");
        return;
    }

    // 获取补货数量
    int restockQuantity = m_restockQuantityEdit->text().toInt();
    if (restockQuantity <= 0)
    {
        QMessageBox::warning(this, "警告", "请输入有效的补货数量");
        return;
    }

    // 获取商品当前库存
    Product product = query_product(productId);
    if (product.id == -1)
    {
        QMessageBox::warning(this, "警告", "选中的商品不存在");
        return;
    }

    // 计算新库存
    int newStock = product.stock + restockQuantity;

    // 更新库存
    if (update_stock(productId, newStock))
    {
        // 补货成功
        QMessageBox::information(this, "提示",
                                 QString("商品 '%1' 补货成功！\n当前库存: %2 → %3").arg(
                                     QString::fromStdString(product.name))
                                 .arg(product.stock).arg(newStock));

        // 清空输入
        m_restockQuantityEdit->clear();

        // 更新商品列表
        updateProductList();
    }
    else
    {
        // 补货失败
        QMessageBox::critical(this, "错误", "商品补货失败，请检查数据库连接");
    }
}

void RestockDialog::onRestockClicked()
{
    restockProduct();
}

void RestockDialog::onCancelClicked()
{
    // 关闭对话框
    reject();
}

void RestockDialog::onRefreshClicked()
{
    // 刷新商品列表
    updateProductList();
    QMessageBox::information(this, "提示", "商品列表已刷新");
}

void RestockDialog::onProductTableDoubleClicked(const QModelIndex& index)
{
    // 获取双击的行号
    int row = index.row();
    
    // 获取该行的商品ID
    QTableWidgetItem* idItem = m_productTable->item(row, 0);
    if (!idItem) return;
    
    int productId = idItem->text().toInt();
    
    // 获取该行的商品名称
    QTableWidgetItem* nameItem = m_productTable->item(row, 1);
    if (!nameItem) return;
    
    QString productName = nameItem->text();
    
    // 在商品下拉列表中选择对应的商品
    int comboIndex = m_productComboBox->findText(productName);
    if (comboIndex != -1) {
        m_productComboBox->setCurrentIndex(comboIndex);
        
        // 将焦点设置到补货数量输入框，方便用户直接输入
        m_restockQuantityEdit->setFocus();
        m_restockQuantityEdit->selectAll();
    }
}
