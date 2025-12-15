#ifndef RETURNDALOG_H
#define RETURNDALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QComboBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QTextEdit>
#include <QDateEdit>
#include <QGroupBox>

class ReturnDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit ReturnDialog(QWidget* parent = nullptr);
    ~ReturnDialog() override;
    
    // 设置交易ID，用于从历史记录中传递
    void setTransactionId(int transactionId);

private:
    // UI组件
    QLineEdit* m_transactionIdEdit;
    QComboBox* m_productComboBox;
    QLineEdit* m_returnQuantityEdit;
    QTextEdit* m_returnReasonEdit;
    QTableWidget* m_transactionTable;
    QTableWidget* m_productTable;
    QPushButton* m_returnButton;
    QPushButton* m_cancelButton;
    QPushButton* m_refreshButton;

    // 初始化UI
    void initUI();
    // 更新交易列表
    void updateTransactionList();
    // 更新商品下拉列表
    void updateProductList();
    // 加载交易记录
    void loadTransactions() const;
    // 加载交易商品
    void loadTransactionProducts(int transactionId) const;
    // 执行退货操作
    void processReturn();
    // 验证输入
    bool validateInput() const;

private slots:
    void onReturnClicked();
    void onCancelClicked();
    void onRefreshClicked();
    void onTransactionTableClicked(const QModelIndex& index);
    void onProductTableDoubleClicked(const QModelIndex& index);
};

#endif // RETURNDALOG_H