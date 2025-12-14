#include "editproductdialog.h"
#include <QMessageBox>

EditProductDialog::EditProductDialog(QWidget* parent)
    : QDialog(parent), m_productId(0), m_productAlertThreshold(10)
{
    // 设置对话框标题
    setWindowTitle("修改商品");

    // 创建UI组件
    auto* productIdLabel = new QLabel("商品ID:");
    m_productIdEdit = new QLineEdit();
    m_productIdEdit->setEnabled(false); // ID不可编辑
    m_productIdEdit->setPlaceholderText("商品ID");
    
    auto* productNameLabel = new QLabel("商品名称:");
    m_productNameEdit = new QLineEdit();
    m_productNameEdit->setPlaceholderText("请输入商品名称");

    auto* productPriceLabel = new QLabel("商品单价:");
    m_productPriceEdit = new QLineEdit();
    // 设置只能输入正数，最多两位小数
    auto* priceValidator = new QDoubleValidator(0.01, 999999.99, 2, this);
    priceValidator->setNotation(QDoubleValidator::StandardNotation);
    m_productPriceEdit->setValidator(priceValidator);
    m_productPriceEdit->setPlaceholderText("请输入商品单价，最多两位小数");

    auto* productStockLabel = new QLabel("商品库存:");
    m_productStockEdit = new QLineEdit();
    m_productStockEdit->setValidator(new QIntValidator(0, 999999, this));
    m_productStockEdit->setPlaceholderText("请输入商品库存");
    
    auto* productAlertThresholdLabel = new QLabel("预警阈值:");
    m_productAlertThresholdEdit = new QLineEdit();
    m_productAlertThresholdEdit->setValidator(new QIntValidator(1, 999999, this));
    m_productAlertThresholdEdit->setPlaceholderText("请输入预警阈值，库存低于此值时会报警");
    m_productAlertThresholdEdit->setText("10"); // 默认阈值为10

    m_okButton = new QPushButton("确认修改");
    m_cancelButton = new QPushButton("取消");

    // 创建布局
    auto* mainLayout = new QVBoxLayout(this);

    auto* idLayout = new QHBoxLayout();
    idLayout->addWidget(productIdLabel);
    idLayout->addWidget(m_productIdEdit);

    auto* nameLayout = new QHBoxLayout();
    nameLayout->addWidget(productNameLabel);
    nameLayout->addWidget(m_productNameEdit);

    auto* priceLayout = new QHBoxLayout();
    priceLayout->addWidget(productPriceLabel);
    priceLayout->addWidget(m_productPriceEdit);

    auto* stockLayout = new QHBoxLayout();
    stockLayout->addWidget(productStockLabel);
    stockLayout->addWidget(m_productStockEdit);
    
    auto* alertThresholdLayout = new QHBoxLayout();
    alertThresholdLayout->addWidget(productAlertThresholdLabel);
    alertThresholdLayout->addWidget(m_productAlertThresholdEdit);

    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(m_cancelButton);

    mainLayout->addLayout(idLayout);
    mainLayout->addLayout(nameLayout);
    mainLayout->addLayout(priceLayout);
    mainLayout->addLayout(stockLayout);
    mainLayout->addLayout(alertThresholdLayout);
    mainLayout->addLayout(buttonLayout);

    // 连接信号与槽
    connect(m_okButton, &QPushButton::clicked, this, &EditProductDialog::onOkClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &EditProductDialog::onCancelClicked);

    // 设置对话框大小
    resize(350, 250);
}

EditProductDialog::~EditProductDialog()
{
    // 不需要手动释放UI组件，Qt的布局会自动处理
}

void EditProductDialog::setProductInfo(const Product& product)
{
    m_productId = product.id;
    m_productIdEdit->setText(QString::number(product.id));
    m_productNameEdit->setText(QString::fromStdString(product.name));
    m_productPriceEdit->setText(QString::number(product.price, 'f', 2));
    m_productStockEdit->setText(QString::number(product.stock));
    // 预警阈值会通过单独的函数设置
}

void EditProductDialog::setProductAlertThreshold(int threshold)
{
    m_productAlertThreshold = threshold;
    m_productAlertThresholdEdit->setText(QString::number(threshold));
}

std::string EditProductDialog::getProductName() const
{
    return m_productNameEdit->text().toStdString();
}

double EditProductDialog::getProductPrice() const
{
    return m_productPriceEdit->text().toDouble();
}

int EditProductDialog::getProductStock() const
{
    return m_productStockEdit->text().toInt();
}

int EditProductDialog::getProductAlertThreshold() const
{
    return m_productAlertThresholdEdit->text().toInt();
}

int EditProductDialog::getProductId() const
{
    return m_productId;
}

void EditProductDialog::onOkClicked()
{
    // 验证输入
    if (m_productNameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入商品名称");
        return;
    }

    if (m_productPriceEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入商品单价");
        return;
    }

    if (m_productStockEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入商品库存");
        return;
    }
    
    if (m_productAlertThresholdEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入预警阈值");
        return;
    }

    // 输入验证通过，关闭对话框
    accept();
}

void EditProductDialog::onCancelClicked()
{
    // 关闭对话框
    reject();
}