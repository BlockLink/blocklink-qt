#include "ColdHotTransferPage.h"
#include "ui_ColdHotTransferPage.h"

#include "wallet.h"
#include "ColdHotTransferDialog.h"
#include "ToolButtonWidget.h"
#include "dialog/TransactionResultDialog.h"
#include "dialog/ErrorResultDialog.h"
#include "dialog/checkpwddialog.h"
#include "commondialog.h"
#include "showcontentdialog.h"
#include "ColdHotInfoDialog.h"

ColdHotTransferPage::ColdHotTransferPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ColdHotTransferPage)
{
    ui->setupUi(this);

    connect( UBChain::getInstance(), SIGNAL(jsonDataUpdated(QString)), this, SLOT(jsonDataUpdated(QString)));

    ui->coldHotTransactionTableWidget->installEventFilter(this);
    ui->coldHotTransactionTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->coldHotTransactionTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->coldHotTransactionTableWidget->setFocusPolicy(Qt::NoFocus);
    ui->coldHotTransactionTableWidget->setMouseTracking(true);
    ui->coldHotTransactionTableWidget->setShowGrid(false);//隐藏表格线

    ui->coldHotTransactionTableWidget->horizontalHeader()->setSectionsClickable(true);
    ui->coldHotTransactionTableWidget->horizontalHeader()->setVisible(true);
    ui->coldHotTransactionTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    ui->coldHotTransactionTableWidget->setColumnWidth(0,120);
    ui->coldHotTransactionTableWidget->setColumnWidth(1,100);
    ui->coldHotTransactionTableWidget->setColumnWidth(2,100);
    ui->coldHotTransactionTableWidget->setColumnWidth(3,100);
    ui->coldHotTransactionTableWidget->setColumnWidth(4,80);
    ui->coldHotTransactionTableWidget->setColumnWidth(5,80);
    ui->coldHotTransactionTableWidget->setColumnWidth(6,80);
    ui->coldHotTransactionTableWidget->setStyleSheet(TABLEWIDGET_STYLE_1);

    ui->transferBtn->setStyleSheet(TOOLBUTTON_STYLE_1);

    ui->typeCurrentBtn->setCheckable(true);
    ui->typeWaitingBtn->setCheckable(true);

    setStyleSheet("QToolButton{background:transparent;border:none;color: rgb(51,51,51);font:12px \"微软雅黑\";padding:0px 0px 1px 0px;}"
                  "QToolButton::checked{background-color:rgb(84,116,235);border-radius:12px;color:white;}");

    init();
}

ColdHotTransferPage::~ColdHotTransferPage()
{
    delete ui;
}

void ColdHotTransferPage::init()
{
    inited = false;

    ui->accountComboBox->clear();
    QStringList accounts = UBChain::getInstance()->getMyFormalGuards();
    ui->accountComboBox->addItems(accounts);

    if(accounts.contains(UBChain::getInstance()->currentAccount))
    {
        ui->accountComboBox->setCurrentText(UBChain::getInstance()->currentAccount);
    }

    UBChain::getInstance()->mainFrame->installBlurEffect(ui->coldHotTransactionTableWidget);

    ui->typeCurrentBtn->setChecked(true);

    fetchColdHotTransaction();

    inited = true;
}

void ColdHotTransferPage::refresh()
{
    fetchColdHotTransaction();
}

void ColdHotTransferPage::jsonDataUpdated(QString id)
{
    if( id.startsWith("id-get_coldhot_transaction-") )
    {
        QString result = UBChain::getInstance()->jsonDataValue(id);
        qDebug() << id << result;

        int type = id.mid(QString("id-get_coldhot_transaction-").size()).toInt();

        if(result.startsWith("\"result\":"))
        {
            result.prepend("{");
            result.append("}");

            QJsonDocument parse_doucment = QJsonDocument::fromJson(result.toLatin1());
            QJsonObject jsonObject = parse_doucment.object();
            QJsonArray array = jsonObject.take("result").toArray();

            if(type == 1)
            {
                coldHotTransactionMap.clear();
                coldHotCrosschainTransactionMap.clear();
            }
            else if(type == 2)
            {
                coldHotSignTransactionMap.clear();
            }
            else if(type == 0)
            {
                pendingColdHotTransactionMap.clear();
            }

            foreach (QJsonValue v, array)
            {
                QJsonArray array2 = v.toArray();
                QJsonObject object = array2.at(1).toObject();
                QJsonArray operationArray = object.take("operations").toArray();
                QJsonArray array3 = operationArray.at(0).toArray();
                int operationType = array3.at(0).toInt();
                QJsonObject operationObject = array3.at(1).toObject();

                if(operationType == TRANSACTION_TYPE_COLDHOT_CROSSCHAIN)
                {
                    ColdHotCrosschainTransaction chct;
                    chct.trxId = array2.at(0).toString();
                    chct.coldHotTrxId = operationObject.take("coldhot_trx_id").toString();

                    coldHotCrosschainTransactionMap.insert(chct.trxId, chct);
                }
                else if(operationType == TRANSACTION_TYPE_COLDHOT)
                {
                    ColdHotTransaction cht;
                    cht.trxId           = array2.at(0).toString();
                    cht.expirationTime  = object.take("expiration").toString();
                    cht.withdrawAddress = operationObject.take("multi_account_withdraw").toString();
                    cht.depositAddress  = operationObject.take("multi_account_deposit").toString();
                    cht.amount          = operationObject.take("amount").toString();
                    cht.assetSymbol     = operationObject.take("asset_symbol").toString();
                    cht.assetId         = operationObject.take("asset_id").toString();
                    cht.guard           = operationObject.take("guard").toString();
                    cht.guardId         = operationObject.take("guard_id").toString();
                    cht.memo            = operationObject.take("memo").toString();

                    if(type == 0)
                    {
                        pendingColdHotTransactionMap.insert(cht.trxId, cht);
                    }
                    else
                    {
                        coldHotTransactionMap.insert(cht.trxId, cht);
                    }
                }
                else if(operationType == TRANSACTION_TYPE_COLDHOT_SIGN)
                {
                    ColdHotSignTransaction chst;
                    chst.trxId          = array2.at(0).toString();
                    chst.coldHotCrosschainTrxId = operationObject.take("coldhot_trx_id").toString();
                    chst.guardAddress   = operationObject.take("guard_address").toString();

                    coldHotSignTransactionMap.insert(chst.trxId, chst);
                }
            }


            if(type != 2)
            {
                showColdHotTransactions();
            }
            else
            {
                refreshColdHotTtransactionsState();
            }
        }

        return;
    }

    if(id == "id-guard_sign_coldhot_transaction")
    {
        QString result = UBChain::getInstance()->jsonDataValue(id);
        qDebug() << id << result;

        if(result.startsWith("\"result\":null"))
        {
            CommonDialog commonDialog(CommonDialog::OkOnly);
            commonDialog.setText(tr("Transaction of guard-sign-coldhot has been sent out!"));
            commonDialog.pop();
        }
        else if(result.startsWith("\"error\":"))
        {
            ErrorResultDialog errorResultDialog;
            errorResultDialog.setInfoText(tr("Failed!"));
            errorResultDialog.setDetailText(result);
            errorResultDialog.pop();
        }

        return;
    }
}

void ColdHotTransferPage::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(QPen(QColor(248,249,253),Qt::SolidLine));
    painter.setBrush(QBrush(QColor(248,249,253),Qt::SolidPattern));

    painter.drawRect(rect());
}

void ColdHotTransferPage::on_transferBtn_clicked()
{
    ColdHotTransferDialog coldHotTransferDialog;
    coldHotTransferDialog.pop();
}

void ColdHotTransferPage::fetchColdHotTransaction()
{
    UBChain::getInstance()->postRPC( "id-get_coldhot_transaction-" + QString::number(0), toJsonFormat( "get_coldhot_transaction",
                                                                                     QJsonArray() << 0));

    UBChain::getInstance()->postRPC( "id-get_coldhot_transaction-" + QString::number(1), toJsonFormat( "get_coldhot_transaction",
                                                                                     QJsonArray() << 1));

    UBChain::getInstance()->postRPC( "id-get_coldhot_transaction-" + QString::number(2), toJsonFormat( "get_coldhot_transaction",
                                                                                     QJsonArray() << 2));

}

void ColdHotTransferPage::showColdHotTransactions()
{
    if(currentType == 1)
    {
        QStringList keys = coldHotTransactionMap.keys();
        int size = keys.size();
        ui->coldHotTransactionTableWidget->setRowCount(0);
        ui->coldHotTransactionTableWidget->setRowCount(size);

        for(int i = 0; i < size; i++)
        {
            ui->coldHotTransactionTableWidget->setRowHeight(i,40);

            ColdHotTransaction cht = coldHotTransactionMap.value(keys.at(i));

            QDateTime time = QDateTime::fromString(cht.expirationTime, "yyyy-MM-ddThh:mm:ss");
    //                time = time.addSecs(-600);       // 时间减10分钟
            QString currentDateTime = time.toString("yyyy-MM-dd\nhh:mm:ss");
            ui->coldHotTransactionTableWidget->setItem(i, 0, new QTableWidgetItem(currentDateTime));
            ui->coldHotTransactionTableWidget->item(i,0)->setData(Qt::UserRole, cht.trxId);

            ui->coldHotTransactionTableWidget->setItem(i, 1, new QTableWidgetItem(cht.amount + " " + cht.assetSymbol));

            ui->coldHotTransactionTableWidget->setItem(i, 2, new QTableWidgetItem(cht.withdrawAddress));

            ui->coldHotTransactionTableWidget->setItem(i, 3, new QTableWidgetItem(cht.depositAddress));

            ui->coldHotTransactionTableWidget->setItem(i, 4, new QTableWidgetItem(tr("checking")));

            ui->coldHotTransactionTableWidget->setItem(i, 5, new QTableWidgetItem(tr("check")));
            ToolButtonWidget *toolButton = new ToolButtonWidget();
            toolButton->setText(ui->coldHotTransactionTableWidget->item(i,5)->text());
    //            toolButton->setBackgroundColor(itemColor);
            ui->coldHotTransactionTableWidget->setCellWidget(i,5,toolButton);
            connect(toolButton,&ToolButtonWidget::clicked,std::bind(&ColdHotTransferPage::on_coldHotTransactionTableWidget_cellClicked,this,i,5));


            ui->coldHotTransactionTableWidget->setItem(i, 6, new QTableWidgetItem(tr("sign")));
            ToolButtonWidget *toolButton2 = new ToolButtonWidget();
            toolButton2->setText(ui->coldHotTransactionTableWidget->item(i,6)->text());
    //            toolButton->setBackgroundColor(itemColor);
            ui->coldHotTransactionTableWidget->setCellWidget(i,6,toolButton2);
            connect(toolButton2,&ToolButtonWidget::clicked,std::bind(&ColdHotTransferPage::on_coldHotTransactionTableWidget_cellClicked,this,i,6));

            for (int j : {0,1,2,3,4})
            {
                if(i%2)
                {
                    ui->coldHotTransactionTableWidget->item(i,j)->setTextAlignment(Qt::AlignCenter);
                    ui->coldHotTransactionTableWidget->item(i,j)->setBackgroundColor(QColor(252,253,255));
                }
                else
                {
                    ui->coldHotTransactionTableWidget->item(i,j)->setTextAlignment(Qt::AlignCenter);
                    ui->coldHotTransactionTableWidget->item(i,j)->setBackgroundColor(QColor("white"));
                }
            }
        }
    }
    else if(currentType == 0)
    {
        QStringList keys = pendingColdHotTransactionMap.keys();
        int size = keys.size();
        ui->coldHotTransactionTableWidget->setRowCount(0);
        ui->coldHotTransactionTableWidget->setRowCount(size);

        for(int i = 0; i < size; i++)
        {
            ui->coldHotTransactionTableWidget->setRowHeight(i,40);

            ColdHotTransaction cht = pendingColdHotTransactionMap.value(keys.at(i));

            QDateTime time = QDateTime::fromString(cht.expirationTime, "yyyy-MM-ddThh:mm:ss");
    //                time = time.addSecs(-600);       // 时间减10分钟
            QString currentDateTime = time.toString("yyyy-MM-dd\nhh:mm:ss");
            ui->coldHotTransactionTableWidget->setItem(i, 0, new QTableWidgetItem(currentDateTime));
            ui->coldHotTransactionTableWidget->item(i,0)->setData(Qt::UserRole, cht.trxId);

            ui->coldHotTransactionTableWidget->setItem(i, 1, new QTableWidgetItem(cht.amount + " " + cht.assetSymbol));

            ui->coldHotTransactionTableWidget->setItem(i, 2, new QTableWidgetItem(cht.withdrawAddress));

            ui->coldHotTransactionTableWidget->setItem(i, 3, new QTableWidgetItem(cht.depositAddress));

            ui->coldHotTransactionTableWidget->setItem(i, 4, new QTableWidgetItem(tr("waiting")));

            ui->coldHotTransactionTableWidget->setItem(i, 5, new QTableWidgetItem(tr("check")));
            ToolButtonWidget *toolButton = new ToolButtonWidget();
            toolButton->setText(ui->coldHotTransactionTableWidget->item(i,5)->text());
    //            toolButton->setBackgroundColor(itemColor);
            ui->coldHotTransactionTableWidget->setCellWidget(i,5,toolButton);
            connect(toolButton,&ToolButtonWidget::clicked,std::bind(&ColdHotTransferPage::on_coldHotTransactionTableWidget_cellClicked,this,i,5));


            for (int j : {0,1,2,3,4})
            {
                if(i%2)
                {
                    ui->coldHotTransactionTableWidget->item(i,j)->setTextAlignment(Qt::AlignCenter);
                    ui->coldHotTransactionTableWidget->item(i,j)->setBackgroundColor(QColor(252,253,255));
                }
                else
                {
                    ui->coldHotTransactionTableWidget->item(i,j)->setTextAlignment(Qt::AlignCenter);
                    ui->coldHotTransactionTableWidget->item(i,j)->setBackgroundColor(QColor("white"));
                }
            }
        }
    }

}

void ColdHotTransferPage::refreshColdHotTtransactionsState()
{
    if(currentType != 1)    return;

    int rowCount = ui->coldHotTransactionTableWidget->rowCount();
    for(int i = 0; i < rowCount; i++)
    {
        QString trxId = ui->coldHotTransactionTableWidget->item(i,0)->data(Qt::UserRole).toString();
        QString crosschainTrxId = lookupCrosschainTrxByColdHotTrxId(trxId);
        QStringList guardAddresses = lookupSignedGuardByCrosschainTrx(crosschainTrxId);

        QString currentAddress = UBChain::getInstance()->formalGuardMap.value(ui->accountComboBox->currentText()).address;
        if(guardAddresses.contains(currentAddress))
        {
            ui->coldHotTransactionTableWidget->item(i,4)->setText(tr("signed"));
        }
        else
        {
            ui->coldHotTransactionTableWidget->item(i,4)->setText(tr("unsigned"));
        }
    }
}

QString ColdHotTransferPage::lookupCrosschainTrxByColdHotTrxId(QString coldHotTrxId)
{
    QString result;
    QStringList keys = coldHotCrosschainTransactionMap.keys();
    foreach (QString key, keys)
    {
        if(coldHotCrosschainTransactionMap.value(key).coldHotTrxId == coldHotTrxId)
        {
            result += key;
            break;
        }
    }

    return result;
}

QStringList ColdHotTransferPage::lookupSignedGuardByCrosschainTrx(QString crosschainTrxId)
{
    QStringList result;
    QStringList keys = coldHotSignTransactionMap.keys();

    foreach (QString key, keys)
    {
        if(coldHotSignTransactionMap.value(key).coldHotCrosschainTrxId == crosschainTrxId)
        {
            result += coldHotSignTransactionMap.value(key).guardAddress;
        }
    }

    return result;
}

void ColdHotTransferPage::on_coldHotTransactionTableWidget_cellClicked(int row, int column)
{
    if(column == 5)
    {
        if(ui->coldHotTransactionTableWidget->item(row,0))
        {
            QString trxId = ui->coldHotTransactionTableWidget->item(row,0)->data(Qt::UserRole).toString();

            qDebug() << trxId;

            ColdHotInfoDialog coldHotInfoDialog(this);
            coldHotInfoDialog.setTrxId(trxId);
            coldHotInfoDialog.pop();
        }

        return;
    }

    if(column == 6)
    {
        if(currentType != 1)    return;

        if(ui->coldHotTransactionTableWidget->item(row,4))
        {
            if(ui->coldHotTransactionTableWidget->item(row,4)->text() == tr("signed"))
            {
                CommonDialog commonDialog(CommonDialog::OkOnly);
                commonDialog.setText(tr("%1 has already signed this cold-hot transaction!").arg(ui->accountComboBox->currentText()));
                commonDialog.pop();
                return;
            }
            else if(ui->coldHotTransactionTableWidget->item(row,4)->text() == tr("checking"))
            {
                CommonDialog commonDialog(CommonDialog::OkOnly);
                commonDialog.setText(tr("Wait for checking the signature of %1!").arg(ui->accountComboBox->currentText()));
                commonDialog.pop();
                return;
            }

            CheckPwdDialog checkPwdDialog;
            if(!checkPwdDialog.pop())   return;

            if(ui->coldHotTransactionTableWidget->item(row,0))
            {
                QString trxId = ui->coldHotTransactionTableWidget->item(row,0)->data(Qt::UserRole).toString();

                QString chcTrxId = lookupCrosschainTrxByColdHotTrxId(trxId);

                if(!chcTrxId.isEmpty() && !ui->accountComboBox->currentText().isEmpty())
                {
                    UBChain::getInstance()->postRPC( "id-guard_sign_coldhot_transaction", toJsonFormat( "guard_sign_coldhot_transaction",
                                                                                                        QJsonArray() << chcTrxId << ui->accountComboBox->currentText()));

                }
            }
        }

        return;
    }
}

void ColdHotTransferPage::on_accountComboBox_currentIndexChanged(const QString &arg1)
{
    refresh();
}

void ColdHotTransferPage::on_coldHotTransactionTableWidget_cellPressed(int row, int column)
{
    if( column == 2 || column == 3)
    {
        ShowContentDialog showContentDialog( ui->coldHotTransactionTableWidget->item(row, column)->text(),this);

        int x = ui->coldHotTransactionTableWidget->columnViewportPosition(column) + ui->coldHotTransactionTableWidget->columnWidth(column) / 2
                - showContentDialog.width() / 2;
        int y = ui->coldHotTransactionTableWidget->rowViewportPosition(row) - 10 + ui->coldHotTransactionTableWidget->horizontalHeader()->height();

        showContentDialog.move( ui->coldHotTransactionTableWidget->mapToGlobal( QPoint(x, y)));
        showContentDialog.exec();

        return;
    }
}

void ColdHotTransferPage::on_typeCurrentBtn_clicked()
{
    currentType = 1;
    ui->typeCurrentBtn->setChecked(true);
    ui->typeWaitingBtn->setChecked(false);
    showColdHotTransactions();
}

void ColdHotTransferPage::on_typeWaitingBtn_clicked()
{
    currentType = 0;
    ui->typeCurrentBtn->setChecked(false);
    ui->typeWaitingBtn->setChecked(true);
    showColdHotTransactions();
}
