#ifndef MANUAL_ADD_DIALOG_H
#define MANUAL_ADD_DIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIntValidator>

class MainWindow;

class ManualAddDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit ManualAddDialog(MainWindow* parent = nullptr);
    ~ManualAddDialog() override;

private:
    MainWindow* m_mainWindow;
    QLineEdit* m_productIdEdit;
    QLineEdit* m_quantityEdit;
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;

private slots:
    void onOkClicked();
    void onCancelClicked();
};

#endif // MANUAL_ADD_DIALOG_H
