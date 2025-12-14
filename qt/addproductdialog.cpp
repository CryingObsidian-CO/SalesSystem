#include "addproductdialog.h"
#include <QMessageBox>

AddProductDialog::AddProductDialog(QWidget* parent)
    : QDialog(parent)
{
    // 设置对话框标题
    setWindowTitle("添加商品");

    // 创建UI组件
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

    m_okButton = new QPushButton("确认");
    m_cancelButton = new QPushButton("取消");

    // 创建布局
    auto* mainLayout = new QVBoxLayout(this);

    auto* nameLayout = new QHBoxLayout();
    nameLayout->addWidget(productNameLabel);
    nameLayout->addWidget(m_productNameEdit);

    auto* priceLayout = new QHBoxLayout();
    priceLayout->addWidget(productPriceLabel);
    priceLayout->addWidget(m_productPriceEdit);

    auto* stockLayout = new QHBoxLayout();
    stockLayout->addWidget(productStockLabel);
    stockLayout->addWidget(m_productStockEdit);

    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(m_cancelButton);

    mainLayout->addLayout(nameLayout);
    mainLayout->addLayout(priceLayout);
    mainLayout->addLayout(stockLayout);
    mainLayout->addLayout(buttonLayout);

    // 连接信号与槽
    connect(m_okButton, &QPushButton::clicked, this, &AddProductDialog::onOkClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &AddProductDialog::onCancelClicked);

    // 设置对话框大小
    resize(350, 180);
}

AddProductDialog::~AddProductDialog()
{
    // 不需要手动释放UI组件，Qt的布局会自动处理
}

std::string AddProductDialog::getProductName() const
{
    return m_productNameEdit->text().toStdString();
}

double AddProductDialog::getProductPrice() const
{
    return m_productPriceEdit->text().toDouble();
}

int AddProductDialog::getProductStock() const
{
    return m_productStockEdit->text().toInt();
}

void AddProductDialog::onOkClicked()
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

    // 输入验证通过，关闭对话框
    accept();
}

void AddProductDialog::onCancelClicked()
{
    // 关闭对话框
    reject();
}