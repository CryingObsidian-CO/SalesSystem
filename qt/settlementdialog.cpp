#include "settlementdialog.h"
#include "mainwindow.h"
#include "../sqlite/database.h"
#include <QMessageBox>
#include <QDoubleValidator>

SettlementDialog::SettlementDialog(MainWindow* parent, double totalPrice)
    : QDialog(parent), m_mainWindow(parent), m_totalPrice(totalPrice)
{
    // 设置对话框标题
    setWindowTitle("结算");
    
    // 创建UI组件
    auto* titleLabel = new QLabel("购物结算");
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont font = titleLabel->font();
    font.setBold(true);
    font.setPointSize(12);
    titleLabel->setFont(font);
    
    auto* totalPriceDescLabel = new QLabel("总计金额:");
    m_totalPriceLabel = new QLabel(QString::number(m_totalPrice, 'f', 2) + " 元");
    m_totalPriceLabel->setStyleSheet("font-weight: bold; color: red;");
    
    auto* amountPaidDescLabel = new QLabel("收到现金:");
    m_amountPaidEdit = new QLineEdit();
    auto* validator = new QDoubleValidator(0.0, 999999.99, 2, this);
    m_amountPaidEdit->setValidator(validator);
    m_amountPaidEdit->setPlaceholderText("请输入收到的现金金额");
    
    auto* changeDescLabel = new QLabel("找零金额:");
    m_changeLabel = new QLabel("0.00 元");
    m_changeLabel->setStyleSheet("font-weight: bold; color: green;");
    
    m_cashButton = new QPushButton("现金结算");
    m_cancelButton = new QPushButton("取消");
    m_cashButton->setEnabled(false); // 初始状态禁用
    
    // 创建布局
    auto* mainLayout = new QVBoxLayout(this);
    auto* contentLayout = new QGridLayout();
    auto* buttonLayout = new QHBoxLayout();
    
    contentLayout->addWidget(totalPriceDescLabel, 0, 0, 1, 1);
    contentLayout->addWidget(m_totalPriceLabel, 0, 1, 1, 1);
    contentLayout->addWidget(amountPaidDescLabel, 1, 0, 1, 1);
    contentLayout->addWidget(m_amountPaidEdit, 1, 1, 1, 1);
    contentLayout->addWidget(changeDescLabel, 2, 0, 1, 1);
    contentLayout->addWidget(m_changeLabel, 2, 1, 1, 1);
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_cashButton);
    buttonLayout->addWidget(m_cancelButton);
    
    mainLayout->addWidget(titleLabel);
    mainLayout->addLayout(contentLayout);
    mainLayout->addLayout(buttonLayout);
    
    // 连接信号与槽
    connect(m_cashButton, &QPushButton::clicked, this, &SettlementDialog::onCashButtonClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &SettlementDialog::onCancelButtonClicked);
    connect(m_amountPaidEdit, &QLineEdit::textChanged, this, &SettlementDialog::onAmountPaidChanged);
    
    // 设置对话框大小
    resize(300, 200);
}

SettlementDialog::~SettlementDialog()
{
    // 不需要手动释放UI组件，Qt的布局会自动处理
}

void SettlementDialog::onAmountPaidChanged(const QString& text)
{
    // 实时计算找零
    double amountPaid = text.toDouble();
    double change = amountPaid - m_totalPrice;
    m_changeLabel->setText(QString::number(change, 'f', 2) + " 元");
    
    // 如果收到的钱大于等于总金额，启用结算按钮
    m_cashButton->setEnabled(amountPaid >= m_totalPrice);
}

void SettlementDialog::onCashButtonClicked()
{
    // 获取收到的金额
    double amountPaid = m_amountPaidEdit->text().toDouble();
    double change = amountPaid - m_totalPrice;
    
    // 生成交易记录
    if (m_mainWindow) {
        // 获取主窗口的购物车
        ShoppingCart& cart = m_mainWindow->getCart();
        
        // 创建交易记录
        Transaction transaction;
        transaction.create_time = time(nullptr);
        transaction.is_paid = true;
        transaction.total_price = m_totalPrice;
        transaction.amount_paid = amountPaid;
        transaction.change = change;
        transaction.cart = cart;
        
        // 保存交易记录到数据库
        if (save_transaction(transaction)) {
            // 交易记录保存成功
        } else {
            QMessageBox::warning(this, "警告", "保存交易记录失败");
            return;
        }
        
        // 清空购物车
        cart.items.clear();
        cart.total_price = 0.0;
        
        // 更新主窗口的购物车显示
        m_mainWindow->updateCartDisplay();
        
        // 显示结算成功信息
        QMessageBox::information(this, "结算成功", 
                                QString("总计金额：%1 元\n收到现金：%2 元\n找零金额：%3 元\n结算成功！").arg(
                                    QString::number(m_totalPrice, 'f', 2),
                                    QString::number(amountPaid, 'f', 2),
                                    QString::number(change, 'f', 2)));
        
        // 关闭对话框
        accept();
    }
}

void SettlementDialog::onCancelButtonClicked()
{
    // 关闭对话框
    reject();
}