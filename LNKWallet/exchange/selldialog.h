#ifndef SELLDIALOG_H
#define SELLDIALOG_H

#include <QDialog>

namespace Ui {
class SellDialog;
}
class FeeChooseWidget;
class SellDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SellDialog(QWidget *parent = 0);
    ~SellDialog();

    void  pop();

    void init();

    void setSellAsset(QString _assetSymbol);
    void setBuyAsset(QString _assetSymbol);

private slots:
    void jsonDataUpdated(QString id);

    void on_okBtn_clicked();

    void on_cancelBtn_clicked();

    void on_assetComboBox_currentIndexChanged(const QString &arg1);

    void on_assetComboBox2_currentIndexChanged(const QString &arg1);

    void on_closeBtn_clicked();

    void on_sellAmountLineEdit_textChanged(const QString &arg1);

    void on_buyAmountLineEdit_textChanged(const QString &arg1);

private:
    Ui::SellDialog *ui;
    int stepCount = 0;      // 合约执行步数

    void estimateContractFee();
    FeeChooseWidget *feeChoose;
};

#endif // SELLDIALOG_H
