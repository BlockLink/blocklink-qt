﻿#include <QDebug>
#include <QPainter>
#include <QTimer>
#include <QMouseEvent>
#include <QScrollBar>
#include <QMovie>

#include "mainpage.h"
#include "ui_mainpage.h"
#include "wallet.h"
#include "debug_log.h"
#include "namedialog.h"
#include "deleteaccountdialog.h"
#include "exportdialog.h"
#include "importdialog.h"
#include "commondialog.h"
#include "showcontentdialog.h"
#include "control/rightclickmenudialog.h"
#include "control/chooseaddaccountdialog.h"
#include "dialog/renamedialog.h"
#include "dialog/backupwalletdialog.h"

MainPage::MainPage(QWidget *parent) :
    QWidget(parent),
    hasDelegateOrNot(false),
    refreshOrNot(false),
    currentAccountIndex(-1),
    ui(new Ui::MainPage)
{
	

    ui->setupUi(this);

    connect( UBChain::getInstance(), SIGNAL(jsonDataUpdated(QString)), this, SLOT(jsonDataUpdated(QString)));


    ui->accountTableWidget->installEventFilter(this);
    ui->accountTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->accountTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->accountTableWidget->setFocusPolicy(Qt::NoFocus);
//    ui->accountTableWidget->setFrameShape(QFrame::NoFrame);
    ui->accountTableWidget->setMouseTracking(true);
    ui->accountTableWidget->setShowGrid(false);//隐藏表格线

    previousColorRow = 0;
    ui->accountTableWidget->horizontalHeader()->setSectionsClickable(true);
    ui->accountTableWidget->horizontalHeader()->setFixedHeight(30);
    ui->accountTableWidget->horizontalHeader()->setVisible(true);
    ui->accountTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    ui->accountTableWidget->setColumnWidth(0,160);
    ui->accountTableWidget->setColumnWidth(1,320);
    ui->accountTableWidget->setColumnWidth(2,160);

    QString language = UBChain::getInstance()->language;
    if( language.isEmpty())
    {
        retranslator("English");
    }
    else
    {
        retranslator(language);
    }


    // 由于首页是第一个页面，第一次打开先等待x秒钟 再 updateAccountList
    QTimer::singleShot(500, this, SLOT(init()));


}

MainPage::~MainPage()
{
	

    delete ui;

	
}

QString toThousandFigure( int);

void MainPage::importAccount()
{
    

    ImportDialog importDialog;
    connect(&importDialog,SIGNAL(accountImported()),this,SLOT(refresh()));
    importDialog.pop();

    emit refreshAccountInfo();

    
}

void MainPage::addAccount()
{
    NameDialog nameDialog;
    QString name = nameDialog.pop();

    if( !name.isEmpty())
    {
        UBChain::getInstance()->postRPC( "id-wallet_create_account-" + name, toJsonFormat( "wallet_create_account", QStringList() << name ));

    }


}


void MainPage::updateAccountList()
{
    AccountInfo info = UBChain::getInstance()->accountInfoMap.value(ui->accountComboBox->currentText());
    ui->addressLabel->setText(info.address);

    AssetAmountMap map = info.assetAmountMap;
    QStringList keys = map.keys();
    qDebug() << "kkkkkkkkkkk  " << keys;
    qDebug() << "aaaaaaaaaaaa  " << UBChain::getInstance()->assetInfoMap.keys();

    int size = keys.size();
    ui->accountTableWidget->setRowCount(size);

    for(int i = 0; i < size; i++)
    {
        ui->accountTableWidget->setRowHeight(i,37);

        QString assetId = keys.at(i);
        AssetInfo assetInfo = UBChain::getInstance()->assetInfoMap.value(assetId);

        //资产名
        QString symbol = assetInfo.symbol;
        ui->accountTableWidget->setItem(i,0,new QTableWidgetItem(symbol));

        //数量
        ui->accountTableWidget->setItem(i,1,new QTableWidgetItem(getBigNumberString(map.value(assetId).amount, assetInfo.precision)));


    }

}

void MainPage::on_addAccountBtn_clicked()
{
    addAccount();

//    ChooseAddAccountDialog* chooseAddAccountDialog = new ChooseAddAccountDialog(this);
//    chooseAddAccountDialog->move( ui->addAccountBtn->mapToGlobal( QPoint(10,-79) ) );
//    connect( chooseAddAccountDialog, SIGNAL(newAccount()), this, SLOT( addAccount()));
//    connect( chooseAddAccountDialog, SIGNAL(importAccount()), this, SLOT( importAccount()));
//    chooseAddAccountDialog->exec();
}

void MainPage::on_importAccountBtn_clicked()
{
    importAccount();
}


void MainPage::on_accountTableWidget_cellClicked(int row, int column)
{
    

//    AccountCellWidget* cellWidget = static_cast<AccountCellWidget*>( ui->accountTableWidget->cellWidget(row,0) );

//    emit openAccountPage( cellWidget->accountName);

    emit openAccountPage( ui->accountTableWidget->item(row,0)->text());

    //    showDetailWidget( cellWidget->accountName );

//        showDetailWidget( ui->accountTableWidget->item(row,0)->text() );

//    if( currentAccountIndex >= 0 && currentAccountIndex != row)
//    {
//        AccountCellWidget* cellWidget2 = static_cast<AccountCellWidget*>( ui->accountTableWidget->cellWidget(currentAccountIndex,0) );
//        cellWidget2->setBackgroundColor( 255,255,255);
//        for( int i = 1; i < 3; i++)
//        {
//            ui->accountTableWidget->item(currentAccountIndex,i)->setBackgroundColor(QColor(255,255,255));
//        }
//    }

//    cellWidget->setBackgroundColor( 245,248,248,150);
//    for( int i = 1; i < 3; i++)
//    {
//        ui->accountTableWidget->item(row,i)->setBackgroundColor(QColor(245,248,248,150));
//    }

//    currentAccountIndex = row;

	
}


void MainPage::on_accountTableWidget_cellEntered(int row, int column)
{

//    if( ui->accountTableWidget->rowCount() > 0)
//        {
//            for( int i = 0; i < 3; i++)
//            {
//                if( ui->accountTableWidget->item(previousColorRow,i) != NULL)
//                {
//                    ui->accountTableWidget->item(previousColorRow,i)->setBackgroundColor(QColor(255,255,255));
//                }
//            }
//        }

//        for( int i = 0; i < 3; i++)
//        {
//            if( ui->accountTableWidget->item(row,i) != NULL)
//            {
//                ui->accountTableWidget->item(row,i)->setBackgroundColor(QColor(245,248,248));
//            }
//        }

//    previousColorRow = row;

}

int tableWidgetPosToRow(QPoint pos, QTableWidget* table);

void MainPage::refresh()
{
//    qDebug() << "mainpage refresh"   << refreshOrNot;
//    if( !refreshOrNot) return;

    updateAccountList();

}

void MainPage::init()
{
//    refreshOrNot = false;

    updateAccountList();
//    updateTotalBalance();

//    refreshOrNot = true;

     QStringList accounts = UBChain::getInstance()->accountInfoMap.keys();
     ui->accountComboBox->addItems(accounts);


}


void MainPage::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(QPen(QColor(40,46,66),Qt::SolidLine));
    painter.setBrush(QBrush(QColor(40,46,66),Qt::SolidPattern));
    painter.drawRect(0,0,680,482);

}

void MainPage::retranslator(QString language)
{
    ui->retranslateUi(this);

    if( language == "Simplified Chinese")
    {

    }
    else if( language == "English")
    {

    }
}

void MainPage::jsonDataUpdated(QString id)
{
    if( id.startsWith("id-wallet_create_account-") )
    {
        QString result = UBChain::getInstance()->jsonDataValue(id);

        if(result.startsWith(QString("\"result\":\"%1").arg(ACCOUNT_ADDRESS_PREFIX)))
        {
            CommonDialog commonDialog(CommonDialog::OkOnly);
            commonDialog.setText( tr("Please backup up the wallet!!!") );
            commonDialog.pop();

            BackupWalletDialog backupWalletDialog;
            backupWalletDialog.pop();

        }
        else if(result.startsWith("\"error\":"))
        {
            int pos = result.indexOf("\"message\":\"") + 11;
            QString errorMessage = result.mid(pos, result.indexOf("\"", pos) - pos);

            CommonDialog commonDialog(CommonDialog::OkOnly);
            commonDialog.setText( "Create account failed: " + errorMessage );
            commonDialog.pop();
        }
        else
        {
            CommonDialog commonDialog(CommonDialog::OkOnly);
            commonDialog.setText( tr("Failed"));
            commonDialog.pop();
        }

        return;
    }

}


void MainPage::updatePending()
{
    
    mutexForPending.lock();

    if( !UBChain::getInstance()->pendingFile->open(QIODevice::ReadWrite))
    {
        qDebug() << "pending.dat not exist";
        return;
    }

    QByteArray ba = QByteArray::fromBase64( UBChain::getInstance()->pendingFile->readAll());
    QString str(ba);
    QStringList strList = str.split(";");
    strList.removeLast();

    mutexForAddressMap.lock();
    QStringList keys = UBChain::getInstance()->addressMap.keys();
    mutexForAddressMap.unlock();

    mutexForConfigFile.lock();
    UBChain::getInstance()->configFile->beginGroup("recordId");
    QStringList recordKeys = UBChain::getInstance()->configFile->childKeys();
    UBChain::getInstance()->configFile->endGroup();


    foreach (QString ss, strList)
    {
        QStringList sList = ss.split(",");

        if( !keys.contains( sList.at(1)) && keys.size() > 0)   // 如果账号被删除了， 删掉pending 中的记录   keys.size() 防止刚启动 addressmap为空
        {
            strList.removeAll( ss);
            continue;
        }
        // 如果config中recordId会被置为1， 则删除该记录
        if( UBChain::getInstance()->configFile->value("recordId/" + sList.at(0)).toInt() != 0 )
        {
            strList.removeAll( ss);
            continue;
        }

        // 如果config中recordId被删除了， 则删除该记录
        if( !UBChain::getInstance()->configFile->contains("recordId/" + sList.at(0)))
        {
            strList.removeAll( ss);
            continue;
        }

    }
    mutexForConfigFile.unlock();


    ba.clear();
    foreach (QString ss, strList)
    {
        ba += QString( ss + ";").toUtf8();
    }
    ba = ba.toBase64();
    UBChain::getInstance()->pendingFile->resize(0);
    QTextStream ts(UBChain::getInstance()->pendingFile);
    ts << ba;
    UBChain::getInstance()->pendingFile->close();

    mutexForPending.unlock();
    
}

//  tablewidget 从 pos 获取 item（每行第0个）
int tableWidgetPosToRow(QPoint pos, QTableWidget* table)
{
    int headerHeight = 30;
    int rowHeight = 37;

    // 获得当前滚动条的位置
    int scrollBarValue = table->verticalScrollBar()->sliderPosition();

    if( pos.y() < headerHeight || pos.y() > table->height())
    {
        return -1;
    }
    else
    {
        int rowCount = floor( (pos.y() - headerHeight) / rowHeight) + scrollBarValue;
        return rowCount;
    }

}

bool MainPage::eventFilter(QObject *watched, QEvent *e)
{
//    if( watched == ui->accountTableWidget)
//    {
//        if( e->type() == QEvent::ContextMenu)
//        {
//            QContextMenuEvent* contextMenuEvent = static_cast<QContextMenuEvent*>(e);
//            QPoint pos = contextMenuEvent->pos();
//            pos -= QPoint(0,47);
//            int row = ui->accountTableWidget->row(ui->accountTableWidget->itemAt(pos));
//            if( row == -1)    return false;

//            if( ui->accountTableWidget->item(row,0) == NULL)   return false;
//            QString name = ui->accountTableWidget->item(row,0)->text();
//            RightClickMenuDialog* rightClickMenuDialog = new RightClickMenuDialog( name, this);
//            rightClickMenuDialog->move( ui->accountTableWidget->mapToGlobal(contextMenuEvent->pos()) );
//            connect( rightClickMenuDialog, SIGNAL(transferFromAccount(QString)), this, SIGNAL(showTransferPage(QString)));
//            connect( rightClickMenuDialog, SIGNAL(renameAccount(QString)), this, SLOT(renameAccount(QString)));
//            connect( rightClickMenuDialog, SIGNAL(exportAccount(QString)), this, SLOT(showExportDialog(QString)));
//            connect( rightClickMenuDialog, SIGNAL(deleteAccount(QString)), this, SLOT(deleteAccount(QString)));
//            rightClickMenuDialog->exec();

//            return true;


//        }
//    }

    return QWidget::eventFilter(watched,e);
}


void MainPage::showExportDialog(QString name)
{

    ExportDialog exportDialog(name);
    exportDialog.pop();
}



void MainPage::renameAccount(QString name)
{
    RenameDialog renameDialog;
    QString newName = renameDialog.pop();

    if( !newName.isEmpty() && newName != name)
    {
        UBChain::getInstance()->write("wallet_account_rename " + name + " " + newName + '\n');
        QString result = UBChain::getInstance()->read();
        qDebug() << result;
        if( result.mid(0,2) == "OK")
        {
            mutexForConfigFile.lock();
            UBChain::getInstance()->configFile->beginGroup("/accountInfo");
            QStringList keys = UBChain::getInstance()->configFile->childKeys();
            foreach (QString key, keys)
            {
                if( UBChain::getInstance()->configFile->value(key) == name)
                {
                    UBChain::getInstance()->configFile->setValue(key, newName);
                    break;
                }
            }
            UBChain::getInstance()->configFile->endGroup();
            mutexForConfigFile.unlock();
            UBChain::getInstance()->balanceMapInsert( newName, UBChain::getInstance()->balanceMapValue(name));
            UBChain::getInstance()->balanceMapRemove(name);
            UBChain::getInstance()->registerMapInsert( newName, UBChain::getInstance()->registerMapValue(name));
            UBChain::getInstance()->registerMapRemove(name);
            UBChain::getInstance()->addressMapInsert( newName, UBChain::getInstance()->addressMapValue(name));
            UBChain::getInstance()->addressMapRemove(name);

//            detailWidget->accountName = newName;
            emit newAccount(newName);

        }
        else
        {
            return;
        }


    }
}

void MainPage::deleteAccount(QString name)
{
    DeleteAccountDialog deleteACcountDialog( name);
    deleteACcountDialog.pop();
}


void MainPage::on_accountComboBox_currentIndexChanged(const QString &arg1)
{


}
