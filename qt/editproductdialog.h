#ifndef EDITPRODUCTDIALOG_H
#define EDITPRODUCTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDoubleValidator>
#include <QIntValidator>
#include "saleStruct.h"

class EditProductDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit EditProductDialog(QWidget* parent = nullptr);
    ~EditProductDialog() override;

    // 设置要修改的商品信息
    void setProductInfo(const Product& product);
    
    // 获取修改后的商品信息
    std::string getProductName() const;
    double getProductPrice() const;
    int getProductStock() const;
    int getProductId() const;
    int getProductAlertThreshold() const;
    
    // 设置商品预警阈值
    void setProductAlertThreshold(int threshold);

private:
    QLineEdit* m_productIdEdit;
    QLineEdit* m_productNameEdit;
    QLineEdit* m_productPriceEdit;
    QLineEdit* m_productStockEdit;
    QLineEdit* m_productAlertThresholdEdit;
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
    
    int m_productId;
    int m_productAlertThreshold;

private slots:
    void onOkClicked();
    void onCancelClicked();
};

#endif // EDITPRODUCTDIALOG_H