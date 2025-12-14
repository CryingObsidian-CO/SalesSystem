#ifndef ADD_PRODUCT_DIALOG_H
#define ADD_PRODUCT_DIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDoubleValidator>
#include <QIntValidator>

class AddProductDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit AddProductDialog(QWidget* parent = nullptr);
    ~AddProductDialog() override;

    // 获取输入的商品信息
    std::string getProductName() const;
    double getProductPrice() const;
    int getProductStock() const;

private:
    QLineEdit* m_productNameEdit;
    QLineEdit* m_productPriceEdit;
    QLineEdit* m_productStockEdit;
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;

private slots:
    void onOkClicked();
    void onCancelClicked();
};

#endif // ADD_PRODUCT_DIALOG_H