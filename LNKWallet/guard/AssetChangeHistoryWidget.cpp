#include "AssetChangeHistoryWidget.h"
#include "ui_AssetChangeHistoryWidget.h"

#include "wallet.h"
#include "ToolButtonWidget.h"
#include "control/AssetIconItem.h"
#include "showcontentdialog.h"
#include "commondialog.h"

AssetChangeHistoryWidget::AssetChangeHistoryWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AssetChangeHistoryWidget)
{
    ui->setupUi(this);

    connect( UBChain::getInstance(), SIGNAL(jsonDataUpdated(QString)), this, SLOT(jsonDataUpdated(QString)));

    ui->changeHistoryTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->changeHistoryTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->changeHistoryTableWidget->setFocusPolicy(Qt::NoFocus);
    ui->changeHistoryTableWidget->setMouseTracking(true);
    ui->changeHistoryTableWidget->setShowGrid(false);//隐藏表格线
    ui->changeHistoryTableWidget->horizontalHeader()->setSectionsClickable(true);
    ui->changeHistoryTableWidget->horizontalHeader()->setVisible(true);
    ui->changeHistoryTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->changeHistoryTableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->changeHistoryTableWidget->verticalHeader()->setVisible(false);
    ui->changeHistoryTableWidget->setColumnWidth(0,120);
    ui->changeHistoryTableWidget->setColumnWidth(1,140);
    ui->changeHistoryTableWidget->setColumnWidth(2,140);
    ui->changeHistoryTableWidget->setColumnWidth(3,100);
    ui->changeHistoryTableWidget->setColumnWidth(4,80);
    ui->changeHistoryTableWidget->setColumnWidth(5,80);
    ui->changeHistoryTableWidget->setStyleSheet(TABLEWIDGET_STYLE_1);

    init();
}

AssetChangeHistoryWidget::~AssetChangeHistoryWidget()
{
    delete ui;
}

void AssetChangeHistoryWidget::init()
{
    inited = false;

    ui->accountComboBox->clear();
    QStringList accounts = UBChain::getInstance()->getMyFormalGuards();
    ui->accountComboBox->addItems(accounts);

    if(accounts.contains(UBChain::getInstance()->currentAccount))
    {
        ui->accountComboBox->setCurrentText(UBChain::getInstance()->currentAccount);
    }

    QStringList assetIds = UBChain::getInstance()->assetInfoMap.keys();
    foreach (QString assetId, assetIds)
    {
        if(assetId == "1.3.0")  continue;
        ui->assetComboBox->addItem(UBChain::getInstance()->assetInfoMap.value(assetId).symbol, assetId);
    }

    UBChain::getInstance()->mainFrame->installBlurEffect(ui->changeHistoryTableWidget);

    inited = true;
}

void AssetChangeHistoryWidget::setAsset(QString _assetSymbol)
{
    ui->assetComboBox->setCurrentText(_assetSymbol);
}

void AssetChangeHistoryWidget::jsonDataUpdated(QString id)
{
    if( id == "id-get_multisig_account_pair-" + ui->assetComboBox->currentText())
    {
        QString result = UBChain::getInstance()->jsonDataValue(id);
//        qDebug() << id << result;

        if(result.startsWith("\"result\":"))
        {
            result.prepend("{");
            result.append("}");
            QJsonDocument parse_doucment = QJsonDocument::fromJson(result.toLatin1());
            QJsonObject jsonObject = parse_doucment.object();
            QJsonArray array = jsonObject.take("result").toArray();

            int size = array.size();
            ui->changeHistoryTableWidget->setRowCount(0);
            ui->changeHistoryTableWidget->setRowCount(size);

            for(int i = 0; i < size; i++)
            {
                int row = size - i - 1;
                ui->changeHistoryTableWidget->setRowHeight(row,40);

                QJsonObject object = array.at(i).toObject();
                QString pairId = object.take("id").toString();
                QString hotAddress = object.take("bind_account_hot").toString();
                QString coldAddress = object.take("bind_account_cold").toString();
                int effectiveBlock = object.take("effective_block_num").toInt();

                ui->changeHistoryTableWidget->setItem(row,0,new QTableWidgetItem(ui->assetComboBox->currentText()));
                ui->changeHistoryTableWidget->setItem(row,1,new QTableWidgetItem(hotAddress));
                ui->changeHistoryTableWidget->setItem(row,2,new QTableWidgetItem(coldAddress));
                
                if(effectiveBlock == 0)
                {
                    ui->changeHistoryTableWidget->setItem(row,3,new QTableWidgetItem(tr("updating")));
                }
                else
                {
                    ui->changeHistoryTableWidget->setItem(row,3,new QTableWidgetItem(QString::number(effectiveBlock)));
                }

                ui->changeHistoryTableWidget->setItem(row,4,new QTableWidgetItem(tr("checking")));
                ui->changeHistoryTableWidget->setItem(row,5,new QTableWidgetItem(tr("export")));

                GuardMultisigAddress gma = UBChain::getInstance()->getGuardMultisigByPairId(ui->assetComboBox->currentText(),
                                                                   ui->accountComboBox->currentText(), pairId);
                ui->changeHistoryTableWidget->item(row,1)->setData(Qt::UserRole, gma.hotAddress);
                ui->changeHistoryTableWidget->item(row,2)->setData(Qt::UserRole, gma.coldAddress);
                fetchCrosschainPrivateKey(gma.hotAddress);
                fetchCrosschainPrivateKey(gma.coldAddress);
                if(i == size - 1)
                {
                    UBChain::getInstance()->postRPC( "finish-dump_crosschain_private_key", toJsonFormat( "finish-dump_crosschain_private_key",
                                                                                                     QJsonArray() ));
                }

                QString itemColor = (row % 2) ?"rgb(252,253,255)":"white";

                AssetIconItem* assetIconItem = new AssetIconItem();
                assetIconItem->setAsset(ui->changeHistoryTableWidget->item(row,0)->text());
                assetIconItem->setBackgroundColor(itemColor);
                ui->changeHistoryTableWidget->setCellWidget(row, 0, assetIconItem);

                ToolButtonWidget *toolButton = new ToolButtonWidget();
                toolButton->setText(ui->changeHistoryTableWidget->item(row,5)->text());
                toolButton->setBackgroundColor(itemColor);
                ui->changeHistoryTableWidget->setCellWidget(row,5,toolButton);
                connect(toolButton,&ToolButtonWidget::clicked,std::bind(&AssetChangeHistoryWidget::on_changeHistoryTableWidget_cellClicked,this,row,5));

                for(int j = 1; j < 5; j++)
                {
                    if(row%2)
                    {
                        ui->changeHistoryTableWidget->item(row,j)->setTextAlignment(Qt::AlignCenter);
                        ui->changeHistoryTableWidget->item(row,j)->setBackgroundColor(QColor(252,253,255));
                    }
                    else
                    {
                        ui->changeHistoryTableWidget->item(row,j)->setTextAlignment(Qt::AlignCenter);
                        ui->changeHistoryTableWidget->item(row,j)->setBackgroundColor(QColor("white"));
                    }
                }
            }
        }

        return;
    }

    if( id == "id-dump_crosschain_private_key")
    {
        QString result = UBChain::getInstance()->jsonDataValue(id);
        qDebug() << id << result;

        if(result.startsWith("\"result\":"))
        {
            result.prepend("{");
            result.append("}");
            QJsonDocument parse_doucment = QJsonDocument::fromJson(result.toLatin1());
            QJsonObject jsonObject = parse_doucment.object();
            QJsonArray array = jsonObject.take("result").toArray();
            if(array.size() == 1)
            {
                QJsonArray array2 = array.at(0).toArray();
                QJsonObject object = array2.at(1).toObject();

                QString address = array2.at(0).toString();
                crosschainKeyObjectMap.insert(address, object);
            }
        }

        return;
    }

    if( id == "finish-dump_crosschain_private_key")
    {
        refreshKeyState();
        return;
    }
}

void AssetChangeHistoryWidget::on_changeHistoryTableWidget_cellPressed(int row, int column)
{
    if( column == 1 || column == 2 )
    {
        ShowContentDialog showContentDialog( ui->changeHistoryTableWidget->item(row, column)->text(),this);

        int x = ui->changeHistoryTableWidget->columnViewportPosition(column) + ui->changeHistoryTableWidget->columnWidth(column) / 2
                - showContentDialog.width() / 2;
        int y = ui->changeHistoryTableWidget->rowViewportPosition(row) - 10 + ui->changeHistoryTableWidget->horizontalHeader()->height();

        showContentDialog.move( ui->changeHistoryTableWidget->mapToGlobal( QPoint(x, y)));
        showContentDialog.exec();

        return;
    }
}

void AssetChangeHistoryWidget::on_changeHistoryTableWidget_cellClicked(int row, int column)
{
    if(column == 5)
    {
        QString assetSymbol = ui->changeHistoryTableWidget->item(row,0)->text();
        QString hotAddress = ui->changeHistoryTableWidget->item(row,1)->data(Qt::UserRole).toString();
        QString coldAddress = ui->changeHistoryTableWidget->item(row,2)->data(Qt::UserRole).toString();

        if(hotAddress.isEmpty() || coldAddress.isEmpty() || !crosschainKeyObjectMap.contains(hotAddress)
                || !crosschainKeyObjectMap.contains(coldAddress))
        {
            CommonDialog commonDialog(CommonDialog::OkOnly);
            commonDialog.setText(tr("There is no corresponding private key in the wallet"));
            commonDialog.pop();
        }
        else
        {
            QJsonArray array;
            array.append(crosschainKeyObjectMap.value(hotAddress));
            array.append(crosschainKeyObjectMap.value(coldAddress));
            QJsonObject object;
            object.insert(assetSymbol, array);
            QString str = QJsonDocument(object).toJson();

            QString path = QFileDialog::getSaveFileName(this, tr("Select the export path"), "", "(*.gcck)");
            if(!path.isEmpty())
            {
                QFile file(path);
                if(!file.open(QIODevice::WriteOnly)) return;

                QTextStream ts(&file);
                ts << str.toLatin1();
                file.close();
            }
        }

        return;
    }
}

void AssetChangeHistoryWidget::on_assetComboBox_currentIndexChanged(const QString &arg1)
{
    fetchMultisigAccountPair();
}

void AssetChangeHistoryWidget::fetchMultisigAccountPair()
{
    UBChain::getInstance()->postRPC( "id-get_multisig_account_pair-" + ui->assetComboBox->currentText(), toJsonFormat( "get_multisig_account_pair",
                                                                           QJsonArray() << ui->assetComboBox->currentText()));
}

void AssetChangeHistoryWidget::fetchCrosschainPrivateKey(QString address)
{
    UBChain::getInstance()->postRPC( "id-dump_crosschain_private_key", toJsonFormat( "dump_crosschain_private_key",
                                                                                     QJsonArray() << address));
}

void AssetChangeHistoryWidget::refreshKeyState()
{
    int rowCount = ui->changeHistoryTableWidget->rowCount();
    for(int i = 0; i < rowCount; i++)
    {
        QString hotAddress = ui->changeHistoryTableWidget->item(i,1)->data(Qt::UserRole).toString();
        QString coldAddress = ui->changeHistoryTableWidget->item(i,2)->data(Qt::UserRole).toString();

        if(crosschainKeyObjectMap.contains(hotAddress) && crosschainKeyObjectMap.contains(coldAddress))
        {
            ui->changeHistoryTableWidget->item(i,4)->setText(tr("available"));
        }
        else
        {
            ui->changeHistoryTableWidget->item(i,4)->setText(tr("unavailable"));
        }
    }
}

void AssetChangeHistoryWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(QPen(QColor(248,249,253),Qt::SolidLine));
    painter.setBrush(QBrush(QColor(248,249,253),Qt::SolidPattern));

    painter.drawRect(rect());
}

void AssetChangeHistoryWidget::on_accountComboBox_currentIndexChanged(const QString &arg1)
{
    if(!inited)     return;

    UBChain::getInstance()->currentAccount = ui->accountComboBox->currentText();
    fetchMultisigAccountPair();
}
