#include "selldialog.h"
#include "ui_selldialog.h"

#include "wallet.h"
#include "commondialog.h"
#include "FeeChooseWidget.h"
#include "dialog/ErrorResultDialog.h"
#include "dialog/TransactionResultDialog.h"

SellDialog::SellDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SellDialog)
{
    ui->setupUi(this);

    setParent(UBChain::getInstance()->mainFrame);

    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlags(Qt::FramelessWindowHint);

    ui->widget->setObjectName("widget");
    ui->widget->setStyleSheet("#widget {background-color:rgba(10, 10, 10,100);}");
    ui->containerWidget->setObjectName("containerwidget");
    ui->containerWidget->setStyleSheet(CONTAINERWIDGET_STYLE);

    ui->okBtn->setStyleSheet(OKBTN_STYLE);
    ui->cancelBtn->setStyleSheet(CANCELBTN_STYLE);
    ui->closeBtn->setStyleSheet(CLOSEBTN_STYLE);

    feeChoose = new FeeChooseWidget(0,UBChain::getInstance()->feeType);
    ui->stackedWidget->addWidget(feeChoose);
    feeChoose->resize(ui->stackedWidget->size());
    ui->stackedWidget->setCurrentIndex(0);

    connect( UBChain::getInstance(), SIGNAL(jsonDataUpdated(QString)), this, SLOT(jsonDataUpdated(QString)));

    ui->sellAmountLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
    ui->buyAmountLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);

    init();
}

SellDialog::~SellDialog()
{
    delete ui;
}

void SellDialog::pop()
{
    move(0,0);
    exec();
}

void SellDialog::init()
{
    ui->accountNameLabel->setText(UBChain::getInstance()->currentAccount);

    QStringList assetIds = UBChain::getInstance()->assetInfoMap.keys();
    foreach (QString assetId, assetIds)
    {
        ui->assetComboBox->addItem(UBChain::getInstance()->assetInfoMap.value(assetId).symbol, assetId);
        ui->assetComboBox2->addItem(UBChain::getInstance()->assetInfoMap.value(assetId).symbol, assetId);
    }
}

void SellDialog::setSellAsset(QString _assetSymbol)
{
    ui->assetComboBox->setCurrentText(_assetSymbol);
}

void SellDialog::setBuyAsset(QString _assetSymbol)
{
    ui->assetComboBox2->setCurrentText(_assetSymbol);
}

void SellDialog::jsonDataUpdated(QString id)
{
    if( id == "id-invoke_contract-putOnSellOrder")
    {
        QString result = UBChain::getInstance()->jsonDataValue(id);
        qDebug() << id << result;

        if(result.startsWith("\"result\":"))
        {
            close();

            TransactionResultDialog transactionResultDialog;
            transactionResultDialog.setInfoText(tr("Transaction of sell-order has been sent out!"));
            transactionResultDialog.setDetailText(result);
            transactionResultDialog.pop();
        }
        else if(result.startsWith("\"error\":"))
        {            
            ErrorResultDialog errorResultDialog;
            errorResultDialog.setInfoText(tr("Fail to create sell-order!"));
            errorResultDialog.setDetailText(result);
            errorResultDialog.pop();
        }

        return;
    }

    if( id == "id-invoke_contract_testing-putOnSellOrder")
    {
        QString result = UBChain::getInstance()->jsonDataValue(id);
        qDebug() << id << result;

        if(result.startsWith("\"result\":"))
        {
            UBChain::TotalContractFee totalFee = UBChain::getInstance()->parseTotalContractFee(result);
            stepCount = totalFee.step;
            unsigned long long totalAmount = totalFee.baseAmount + ceil(totalFee.step * UBChain::getInstance()->contractFee / 100.0);

            feeChoose->updateFeeNumberSlots(getBigNumberString(totalAmount, ASSET_PRECISION).toDouble());
            feeChoose->updateAccountNameSlots(ui->accountNameLabel->text());
        }

        return;
    }

    if( id.startsWith( "id-unlock-SellDialog") )
    {
        QString result = UBChain::getInstance()->jsonDataValue(id);
        qDebug() << id << result;

        if( result == "\"result\":null")
        {
            QString contractAddress = UBChain::getInstance()->getExchangeContractAddress(ui->accountNameLabel->text());

            AssetInfo assetInfo = UBChain::getInstance()->assetInfoMap.value(ui->assetComboBox->currentData().toString());
            AssetInfo assetInfo2 = UBChain::getInstance()->assetInfoMap.value(ui->assetComboBox2->currentData().toString());

            QString params = QString("%1,%2,%3,%4").arg(ui->assetComboBox->currentText()).arg(decimalToIntegerStr(ui->sellAmountLineEdit->text(), assetInfo.precision))
                   .arg(ui->assetComboBox2->currentText()).arg(decimalToIntegerStr(ui->buyAmountLineEdit->text(), assetInfo2.precision));
            feeChoose->updatePoundageID();
            UBChain::getInstance()->postRPC( "id-invoke_contract-putOnSellOrder", toJsonFormat( "invoke_contract",
                                                                                   QJsonArray() << ui->accountNameLabel->text()
                                                                                   << UBChain::getInstance()->currentContractFee() << stepCount
                                                                                   << contractAddress
                                                                                   << "putOnSellOrder"  << params));
        }
        else if(result.startsWith("\"error\":"))
        {
            ui->tipLabel->setText("<body><font style=\"font-size:12px\" color=#ff224c>" + tr("Wrong password!") + "</font></body>" );
        }

        return;
    }
}

void SellDialog::on_okBtn_clicked()
{
    if(ui->assetComboBox->currentText() == ui->assetComboBox2->currentText())
    {
        CommonDialog commonDialog(CommonDialog::OkOnly);
        commonDialog.setText( tr("Assets can not be the same!") );
        commonDialog.pop();

        return;
    }

    if(ui->sellAmountLineEdit->text().toDouble() <= 0 || ui->buyAmountLineEdit->text().toDouble() <= 0)
    {
        CommonDialog commonDialog(CommonDialog::OkOnly);
        commonDialog.setText(tr("The amount can not be 0!"));
        commonDialog.pop();
        return;
    }

    UBChain::getInstance()->postRPC( "id-unlock-SellDialog", toJsonFormat( "unlock", QJsonArray() << ui->pwdLineEdit->text()
                                               ));
}

void SellDialog::on_cancelBtn_clicked()
{
    close();
}


void SellDialog::on_assetComboBox_currentIndexChanged(const QString &arg1)
{
    ExchangeContractBalances balances = UBChain::getInstance()->accountExchangeContractBalancesMap.value(ui->accountNameLabel->text());

    unsigned long long balanceAmount = 0;
    if(balances.contains(ui->assetComboBox->currentText()))
    {
        balanceAmount = balances.value(ui->assetComboBox->currentText());
    }

    AssetInfo assetInfo = UBChain::getInstance()->assetInfoMap.value(UBChain::getInstance()->getAssetId(ui->assetComboBox->currentText()));

    QString balanceStr = getBigNumberString(balanceAmount,assetInfo.precision);

    ui->sellAmountLineEdit->setPlaceholderText(tr("Max: %1 %2").arg(balanceStr).arg(ui->assetComboBox->currentText()));


    QRegExp rx1(QString("^([0]|[1-9][0-9]{0,10})(?:\\.\\d{0,%1})?$|(^\\t?$)").arg(assetInfo.precision));
    QRegExpValidator *pReg1 = new QRegExpValidator(rx1, this);
    ui->sellAmountLineEdit->setValidator(pReg1);
    ui->sellAmountLineEdit->clear();
}

void SellDialog::on_assetComboBox2_currentIndexChanged(const QString &arg1)
{
    AssetInfo assetInfo = UBChain::getInstance()->assetInfoMap.value(UBChain::getInstance()->getAssetId(ui->assetComboBox2->currentText()));

    QRegExp rx1(QString("^([0]|[1-9][0-9]{0,10})(?:\\.\\d{0,%1})?$|(^\\t?$)").arg(assetInfo.precision));
    QRegExpValidator *pReg1 = new QRegExpValidator(rx1, this);
    ui->buyAmountLineEdit->setValidator(pReg1);
    ui->buyAmountLineEdit->clear();
}

void SellDialog::on_closeBtn_clicked()
{
    close();
}

void SellDialog::estimateContractFee()
{
    feeChoose->updateFeeNumberSlots(0);
    if(ui->sellAmountLineEdit->text().toDouble() <= 0)  return;
    if(ui->buyAmountLineEdit->text().toDouble() <= 0)  return;

    QString contractAddress = UBChain::getInstance()->getExchangeContractAddress(ui->accountNameLabel->text());

    AssetInfo assetInfo = UBChain::getInstance()->assetInfoMap.value(ui->assetComboBox->currentData().toString());
    AssetInfo assetInfo2 = UBChain::getInstance()->assetInfoMap.value(ui->assetComboBox2->currentData().toString());

    QString params = QString("%1,%2,%3,%4").arg(ui->assetComboBox->currentText()).arg(decimalToIntegerStr(ui->sellAmountLineEdit->text(), assetInfo.precision))
            .arg(ui->assetComboBox2->currentText()).arg(decimalToIntegerStr(ui->buyAmountLineEdit->text(), assetInfo2.precision));
    UBChain::getInstance()->postRPC( "id-invoke_contract_testing-putOnSellOrder", toJsonFormat( "invoke_contract_testing",
                                                                           QJsonArray() << ui->accountNameLabel->text()
                                                                           << contractAddress
                                                                           << "putOnSellOrder"  << params));
}

void SellDialog::on_sellAmountLineEdit_textChanged(const QString &arg1)
{
    ExchangeContractBalances balances = UBChain::getInstance()->accountExchangeContractBalancesMap.value(ui->accountNameLabel->text());
    unsigned long long balanceAmount = 0;
    if(balances.contains(ui->assetComboBox->currentText()))
    {
        balanceAmount = balances.value(ui->assetComboBox->currentText());
    }

    AssetInfo assetInfo = UBChain::getInstance()->assetInfoMap.value(UBChain::getInstance()->getAssetId(ui->assetComboBox->currentText()));
    QString balanceStr = getBigNumberString(balanceAmount,assetInfo.precision);
    if(ui->sellAmountLineEdit->text().toDouble() > balanceStr.toDouble())
    {
        ui->sellAmountLineEdit->setText(balanceStr);
        return;
    }

    estimateContractFee();
}

void SellDialog::on_buyAmountLineEdit_textChanged(const QString &arg1)
{
    estimateContractFee();
}
