#ifndef HISTORYDIALOG_H
#define HISTORYDIALOG_H

#include <QDialog>
#include "saleStruct.h"

QT_BEGIN_NAMESPACE
namespace Ui {
    class HistoryDialog;
}
QT_END_NAMESPACE

class HistoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HistoryDialog(QWidget *parent = nullptr);
    ~HistoryDialog() override;

private slots:
    void on_transactionTable_doubleClicked(const QModelIndex &index);
    void on_refreshButton_clicked();
    void on_returnButton_clicked();
    void on_returnRecordButton_clicked();
    void checkLowStock();

private:
    Ui::HistoryDialog *ui;
    void loadTransactions();
    void showTransactionDetails(int transactionId) const;
    void showLowStockWarning();
};
#endif // HISTORYDIALOG_H