#include "PoundageWidget.h"
#include "ui_PoundageWidget.h"

#include <mutex>
#include <thread>
#include <QDebug>
#include "PoundageDataUtil.h"
#include "PublishPoundageWidget.h"
#include "PoundageShowWidget.h"

#include "wallet.h"
#include "commondialog.h"
#include "depositpage/FeeChargeWidget.h"
#include "dialog/ErrorResultDialog.h"
#include "dialog/TransactionResultDialog.h"

class PoundageWidget::PoundageWidgetPrivate
{
public:
    PoundageWidgetPrivate()
        :allPoundageWidget(new PoundageShowWidget())
        ,myPoundageWidget(new PoundageShowWidget())
        ,allPoundageSheet(std::make_shared<PoundageSheet>())
        ,myPoundageSheet(std::make_shared<PoundageSheet>())
    {
        myPoundageWidget->EnableDeleteAction(true);
        myPoundageWidget->EnalbeDefaultAction(false);
        allPoundageWidget->EnableDeleteAction(false);
        allPoundageWidget->EnalbeDefaultAction(false);
    }
public:
    PoundageShowWidget *allPoundageWidget;
    PoundageShowWidget *myPoundageWidget;
    std::shared_ptr<PoundageSheet> allPoundageSheet;
    std::shared_ptr<PoundageSheet> myPoundageSheet;
    std::mutex allMutex;
    std::mutex refreshMutex;
};

PoundageWidget::PoundageWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PoundageWidget),
    _p(new PoundageWidgetPrivate())
{
    ui->setupUi(this);
   // _p->allPoundageSheet = std::make_shared<PoundageSheet>();
   // _p->myPoundageSheet = std::make_shared<PoundageSheet>();
    InitWidget();
}

PoundageWidget::~PoundageWidget()
{
    delete _p;
    delete ui;
}

void PoundageWidget::autoRefreshSlots()
{
    std::lock_guard<std::mutex> lock(_p->refreshMutex);
    if(ui->toolButton_allPoundage->isChecked())
    {
        _p->allPoundageSheet->clear();
        //刷新目前对应的承税单
        if(ui->comboBox_coinType->currentData().toString() == "0")
        {
            //查询所有承兑单
            foreach(AssetInfo asset,UBChain::getInstance()->assetInfoMap){
                if(asset.id == "1.3.0") continue;
                UBChain::getInstance()->postRPC("poundage_list_guarantee_order_"+asset.symbol,
                                                toJsonFormat("list_guarantee_order",
                                                             QJsonArray()<<asset.symbol<<false
                                                             )
                                                );

                qDebug()<<toJsonFormat("list_guarantee_order",
                                       QJsonArray()<<asset.symbol<<false
                                       );
            }

        }
        else
        {
            UBChain::getInstance()->postRPC("poundage_list_guarantee_order_",
                                            toJsonFormat("list_guarantee_order",
                                                         QJsonArray()<<ui->comboBox_coinType->currentText()<<false
                                                         )
                                            );
        }
        UBChain::getInstance()->postRPC("poundage_finish_all_list",toJsonFormat("finish_all_list",QJsonArray()));

    }
    else if(ui->toolButton_myPoundage->isChecked())
    {
        _p->myPoundageSheet->clear();

        //获取自己账户的所有信息
        foreach(QString key,UBChain::getInstance()->accountInfoMap.keys())
        {
            UBChain::getInstance()->postRPC("poundage_get_my_guarantee_order_"+key,
                                            toJsonFormat("get_my_guarantee_order",
                                                         QJsonArray()<<key<<false
                                                         )
                                            );
        }
        UBChain::getInstance()->postRPC("poundage_finish_my_list",toJsonFormat("finish_all_list",QJsonArray()));

    }

}

void PoundageWidget::PublishPoundageSlots()
{
    if(UBChain::getInstance()->accountInfoMap.empty())
    {
        CommonDialog dia(CommonDialog::OkOnly);
        dia.setText(tr("Please Import Or Create Account First!"));
        dia.pop();
        UBChain::getInstance()->mainFrame->ShowMainPageSlot();
        return;
    }
    PublishPoundageWidget *publishWidget = new PublishPoundageWidget(this);
    connect(publishWidget,&PublishPoundageWidget::backBtnVisible,this,&PoundageWidget::backBtnVisible);
    publishWidget->setWindowFlags(Qt::WindowStaysOnTopHint /*| Qt::Dialog*/ | Qt::FramelessWindowHint);
    publishWidget->setWindowModality(Qt::WindowModal);
    publishWidget->setAttribute(Qt::WA_DeleteOnClose);
    publishWidget->move(QPoint(0,0));
    publishWidget->show();
    emit backBtnVisible(true);
}

void PoundageWidget::ShowAllPoundageSlots()
{
    ui->toolButton_allPoundage->setChecked(true);
    ui->toolButton_myPoundage->setChecked(false);

    autoRefreshSlots();
    ui->stackedWidget->setCurrentWidget(_p->allPoundageWidget);
    updateButtonStyle();
}

void PoundageWidget::ShowMyPoundageSlots()
{
    ui->toolButton_allPoundage->setChecked(false);
    ui->toolButton_myPoundage->setChecked(true);

    autoRefreshSlots();
    ui->stackedWidget->setCurrentWidget(_p->myPoundageWidget);
    updateButtonStyle();
}

void PoundageWidget::SortByStuffSlots()
{
    std::shared_ptr<PoundageSheet> data = ui->toolButton_allPoundage->isChecked()?
                                          _p->allPoundageSheet : _p->myPoundageSheet;

    switch (ui->comboBox_sortType->currentData(Qt::UserRole).value<int>()) {
    case 0:
        data->sortByTime(true);
        break;
    case 1:
        data->sortByTime(false);
        break;
    case 2:
        data->sortByRate(true);
        break;
    case 3:
        data->sortByRate(false);
        break;
    default:
        break;
    }
    if(ui->toolButton_allPoundage->isChecked())
    {
        _p->allPoundageWidget->InitData(_p->allPoundageSheet);
    }
    else
    {
        _p->myPoundageWidget->InitData(_p->myPoundageSheet);
    }

}

void PoundageWidget::jsonDataUpdated(QString id)
{
    if("publish_create_guarantee_order" == id)
    {
        QString result = UBChain::getInstance()->jsonDataValue( id);
        qDebug() << id << result;

        if( result.startsWith("\"result\":{"))             // 成功
        {
            TransactionResultDialog transactionResultDialog;
            transactionResultDialog.setInfoText(tr("Transaction of create-guarantee-order has been sent!"));
            transactionResultDialog.setDetailText(result);
            transactionResultDialog.pop();
        }
        else
        {
            ErrorResultDialog errorResultDialog;
            errorResultDialog.setInfoText(tr("Fail to create guarantee order!"));
            errorResultDialog.setDetailText(result);
            errorResultDialog.pop();
        }

        //刷新承税单
        autoRefreshSlots();
    }
    else if(id.startsWith("poundage_list_guarantee_order_"))
    {
        std::lock_guard<std::mutex> guar(_p->allMutex);
        //转化为结构体
        QString result = UBChain::getInstance()->jsonDataValue( id);
        if( result.isEmpty() )  return;
        result.prepend("{");
        result.append("}");
        qDebug()<<result;
        PoundageDataUtil::convertJsonToPoundage(result,_p->allPoundageSheet);
    }
    else if("poundage_finish_all_list" == id)
    {
        std::lock_guard<std::mutex> guar(_p->allMutex);
        SortByStuffSlots();
        //qDebug()<<_p->allPoundageSheet->size();
        //_p->allPoundageWidget->InitData(_p->allPoundageSheet);

    }
    else if(id.startsWith("poundage_get_my_guarantee_order_"))
    {
        //转化为结构体
        QString result = UBChain::getInstance()->jsonDataValue( id);
        if( result.isEmpty() )  return;
        result.prepend("{");
        result.append("}");

        qDebug()<<result;
        PoundageDataUtil::convertJsonToPoundage(result,_p->myPoundageSheet);

    }
    else if("poundage_finish_my_list" == id)
    {
        _p->myPoundageSheet->filterByChainType(ui->comboBox_coinType->currentText());
        SortByStuffSlots();
        //_p->myPoundageWidget->InitData(_p->myPoundageSheet);

    }
    else if("poundage_cancel_guarantee_order" == id)
    {
        //转化为结构体
        QString result = UBChain::getInstance()->jsonDataValue( id);
        qDebug() << id << result;

        if( result.startsWith("\"result\":{"))             // 成功
        {
            TransactionResultDialog transactionResultDialog;
            transactionResultDialog.setInfoText(tr("Transaction of cancel-guarantee-order has been sent!"));
            transactionResultDialog.setDetailText(result);
            transactionResultDialog.pop();
        }
        else
        {
            ErrorResultDialog errorResultDialog;
            errorResultDialog.setInfoText(tr("Fail to cancel guarantee order!"));
            errorResultDialog.setDetailText(result);
            errorResultDialog.pop();
        }

        autoRefreshSlots();
    }
}

void PoundageWidget::DeletePoundageSlots(const QString &orderID,const QString &accountName)
{
    FeeChargeWidget *feeCharge = new FeeChargeWidget(UBChain::getInstance()->feeChargeInfo.poundageCancelFee.toDouble(),"LNK",
                                                     accountName,UBChain::getInstance()->mainFrame);
    feeCharge->setAttribute(Qt::WA_DeleteOnClose);
    feeCharge->show();
    connect(feeCharge,&FeeChargeWidget::confirmSignal,[orderID](){
        UBChain::getInstance()->postRPC("poundage_cancel_guarantee_order",
                                        toJsonFormat("cancel_guarantee_order",
                                                     QJsonArray()<<orderID<<true)
                                        );
    });

}

void PoundageWidget::SetDefaultPoundageSlots(const QString &orderID)
{
    //往配置文件里写入
    UBChain::getInstance()->configFile->setValue("/settings/feeOrderID", orderID);
    UBChain::getInstance()->feeOrderID = orderID;
}

void PoundageWidget::InitWidget()
{
    InitStyle();

    //
    ui->toolButton_allPoundage->setCheckable(true);
    ui->toolButton_allPoundage->setChecked(true);
    ui->toolButton_myPoundage->setCheckable(true);
    ui->toolButton_myPoundage->setChecked(false);
    //初始化排序comboBox
    ui->comboBox_sortType->clear();
//    ui->comboBox_sortType->addItem(tr("时间由早到晚"),0);
//    ui->comboBox_sortType->addItem(tr("时间由晚到早"),1);
    ui->comboBox_sortType->addItem(tr("汇率由低到高"),2);
    ui->comboBox_sortType->addItem(tr("汇率由高到低"),3);
    ui->comboBox_sortType->setCurrentIndex(1);

    //初始化币种
    InitCoinType();

    ui->stackedWidget->addWidget(_p->allPoundageWidget);
    ui->stackedWidget->addWidget(_p->myPoundageWidget);
    ui->stackedWidget->setCurrentWidget(_p->allPoundageWidget);


    connect( UBChain::getInstance(), &UBChain::jsonDataUpdated, this, &PoundageWidget::jsonDataUpdated);

    connect(ui->toolButton_publishPoundage,&QToolButton::clicked,this,&PoundageWidget::PublishPoundageSlots);
    connect(ui->toolButton_allPoundage,&QToolButton::clicked,this,&PoundageWidget::ShowAllPoundageSlots);
    connect(ui->toolButton_myPoundage,&QToolButton::clicked,this,&PoundageWidget::ShowMyPoundageSlots);

    connect(ui->comboBox_sortType,static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,&PoundageWidget::SortByStuffSlots);
    connect(ui->comboBox_coinType,static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),this,&PoundageWidget::autoRefreshSlots);

    connect(_p->myPoundageWidget,&PoundageShowWidget::DeletePoundageSignal,this,&PoundageWidget::DeletePoundageSlots);
    connect(_p->allPoundageWidget,&PoundageShowWidget::SetDefaultPoundageSignal,this,&PoundageWidget::SetDefaultPoundageSlots);
    connect(_p->myPoundageWidget,&PoundageShowWidget::SetDefaultPoundageSignal,this,&PoundageWidget::SetDefaultPoundageSlots);
    autoRefreshSlots();
    updateButtonStyle();
}

void PoundageWidget::InitStyle()
{
    setAutoFillBackground(true);
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(248,249,253));
    setPalette(palette);

    QFont font("\"Microsoft YaHei UI Light\"",14,50);
    font.setPixelSize(14);
    ui->label->setFont(font);
    ui->label_2->setFont(font);

    ui->toolButton_publishPoundage->setStyleSheet(TOOLBUTTON_STYLE_1);

    setStyleSheet("QToolButton#toolButton_allPoundage{background:transparent;color:rgb(144,144,144);\
                                                      font-family: \"Microsoft YaHei UI Light\";font:14px;font-weight:600;text-align:left;}\
                   QToolButton#toolButton_allPoundage:checked{color:rgb(0,0,0);}\
                   QToolButton#toolButton_myPoundage{background:transparent;color:rgb(144,144,144);\
                                                     font-family: \"Microsoft YaHei UI Light\";font:14px;font-weight:600;text-align:left;}\
                   QToolButton#toolButton_myPoundage:checked{color:rgb(0,0,0);}"
                   TABLEWIDGET_STYLE_1);
}

void PoundageWidget::InitCoinType()
{
    //初始化币种(币symbol，币id)
    ui->comboBox_coinType->clear();
    foreach(AssetInfo asset,UBChain::getInstance()->assetInfoMap){
        if(asset.id == "1.3.0") continue;
        ui->comboBox_coinType->addItem(asset.symbol,asset.id);
    }
    ui->comboBox_coinType->insertItem(0,"All","0");
    if(ui->comboBox_coinType->count() > 0)
    {
        ui->comboBox_coinType->setCurrentIndex(0);
    }
}

void PoundageWidget::updateButtonStyle()
{
    QPalette pa;
    pa.setColor(QPalette::WindowText,Qt::black);

    QPalette pa1;
    pa1.setColor(QPalette::WindowText,QColor(192,202,212));

    ui->label->setPalette(ui->toolButton_allPoundage->isChecked()?pa:pa1);
    ui->frame->setVisible(ui->toolButton_allPoundage->isChecked());
    ui->label_2->setPalette(ui->toolButton_myPoundage->isChecked()?pa:pa1);
    ui->frame_2->setVisible(ui->toolButton_myPoundage->isChecked());
}
