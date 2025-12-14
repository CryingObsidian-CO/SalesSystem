#include "manualadddialog.h"
#include "mainwindow.h"
#include "database.h"
#include <QMessageBox>
#include <QIntValidator>

ManualAddDialog::ManualAddDialog(MainWindow* parent)
    : QDialog(parent), m_mainWindow(parent)
{
    // 设置对话框标题
    setWindowTitle("手动添加商品");

    // 创建UI组件
    auto* productIdLabel = new QLabel("商品ID:");
    m_productIdEdit = new QLineEdit();
    m_productIdEdit->setValidator(new QIntValidator(1, 9999, this));
    m_productIdEdit->setPlaceholderText("请输入商品ID");

    auto* quantityLabel = new QLabel("数量:");
    m_quantityEdit = new QLineEdit();
    m_quantityEdit->setValidator(new QIntValidator(1, 9999, this));
    m_quantityEdit->setText("1"); // 默认数量为1

    m_okButton = new QPushButton("确认");
    m_cancelButton = new QPushButton("取消");

    // 创建布局
    auto* mainLayout = new QVBoxLayout(this);

    auto* idLayout = new QHBoxLayout();
    idLayout->addWidget(productIdLabel);
    idLayout->addWidget(m_productIdEdit);

    auto* quantityLayout = new QHBoxLayout();
    quantityLayout->addWidget(quantityLabel);
    quantityLayout->addWidget(m_quantityEdit);

    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(m_cancelButton);

    mainLayout->addLayout(idLayout);
    mainLayout->addLayout(quantityLayout);
    mainLayout->addLayout(buttonLayout);

    // 连接信号与槽
    connect(m_okButton, &QPushButton::clicked, this, &ManualAddDialog::onOkClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &ManualAddDialog::onCancelClicked);

    // 设置对话框大小
    resize(300, 150);
}

ManualAddDialog::~ManualAddDialog()
{
    // 不需要手动释放UI组件，Qt的布局会自动处理
}

void ManualAddDialog::onOkClicked()
{
    // 获取商品ID和数量
    int productId = m_productIdEdit->text().toInt();
    int quantity = m_quantityEdit->text().toInt();

    // 验证输入
    if (productId <= 0) {
        QMessageBox::warning(this, "警告", "请输入有效的商品ID");
        return;
    }

    if (quantity <= 0) {
        QMessageBox::warning(this, "警告", "请输入有效的数量");
        return;
    }

    // 查询商品是否存在
    Product product = query_product(productId);
    if (product.id == -1) {
        QMessageBox::warning(this, "警告", QString("商品ID %1 不存在").arg(productId));
        return;
    }

    // 检查数量是否超过库存
    if (quantity > product.stock) {
        QMessageBox::warning(this, "警告", QString("商品ID %1 的库存不足，当前库存为 %2").arg(productId).arg(product.stock));
        return;
    }

    // 添加到购物车
    if (m_mainWindow) {
        ShoppingCart& cart = m_mainWindow->getCart();

        // 检查购物车中是否已有该商品
        bool found = false;
        for (auto& cartItem : cart.items) {
            if (cartItem.product.id == productId) {
                // 更新已有商品的数量
                float oldSubtotal = cartItem.subtotal;
                cartItem.quantity += quantity;
                cartItem.subtotal = cartItem.product.price * cartItem.quantity;
                cart.total_price += (cartItem.subtotal - oldSubtotal);
                found = true;
                break;
            }
        }

        if (!found) {
            // 添加新商品到购物车
            CartItem cartItem;
            cartItem.product = product;
            cartItem.quantity = quantity;
            cartItem.subtotal = product.price * quantity;
            cart.items.push_back(cartItem);
            cart.total_price += cartItem.subtotal;
        }

        // 更新主窗口的购物车显示
        m_mainWindow->updateCartDisplay();

        // 关闭对话框
        accept();
    }
}

void ManualAddDialog::onCancelClicked()
{
    // 关闭对话框
    reject();
}