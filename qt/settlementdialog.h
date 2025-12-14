#ifndef SETTLEMENTDIALOG_H
#define SETTLEMENTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QIntValidator>
#include <QDoubleValidator>
#include "saleStruct.h"

class MainWindow;

class SettlementDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettlementDialog(MainWindow* parent = nullptr, double totalPrice = 0.0);
    ~SettlementDialog() override;

private:
    MainWindow* m_mainWindow;
    double m_totalPrice;
    QLabel* m_totalPriceLabel;
    QLineEdit* m_amountPaidEdit;
    QLabel* m_changeLabel;
    QPushButton* m_cashButton;
    QPushButton* m_cancelButton;

private slots:
    void onCashButtonClicked();
    void onCancelButtonClicked();
    void onAmountPaidChanged(const QString& text);
};

#endif // SETTLEMENTDIALOG_H