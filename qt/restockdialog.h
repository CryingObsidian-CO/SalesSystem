#ifndef RESTOCKDIALOG_H
#define RESTOCKDIALOG_H

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

class RestockDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit RestockDialog(QWidget* parent = nullptr);
    ~RestockDialog() override;

private:
    QComboBox* m_productComboBox;
    QLineEdit* m_restockQuantityEdit;
    QTableWidget* m_productTable;
    QPushButton* m_restockButton;
    QPushButton* m_cancelButton;
    QPushButton* m_refreshButton;

    // 更新商品下拉列表和表格
    void updateProductList();
    // 加载所有商品信息
    void loadAllProducts() const;
    // 补货操作
    void restockProduct();

private slots:
    void onRestockClicked();
    void onCancelClicked();
    void onRefreshClicked();
    void onProductTableDoubleClicked(const QModelIndex& index);
};

#endif // RESTOCKDIALOG_H