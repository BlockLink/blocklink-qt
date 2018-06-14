#ifndef FRAME_H
#define FRAME_H
#include <QFrame>
#include <QWidget>
#include <QMap>
#include <QTranslator>
#include <QSystemTrayIcon>
#ifdef WIN32
#include "windows.h"
#endif

//namespace Ui {
//   class Frame;
//}

class FirstLogin;
class NormalLogin;
class MainPage;
class TransferPage;
class BottomBar;
class LockPage;
class TitleBar;
class QMenu;
class WaitingForSync;
class UpgradePage;
class ApplyDelegatePage;
class SelectWalletPathWidget;
class ShadowWidget;
class SmartContractPage;
class MultiSigPage;
class MultiSigTransactionPage;
class NewOrImportWalletWidget;

class DepositAutomatic;
class ContactWidget;
class FunctionWidget;
class MinerPage;
class OnchainOrderPage;
class MyExchangeContractPage;
class PoundageWidget;
class AssetPage;

class CrossCapitalMark;
class CustomShadowEffect;
class Frame:public QFrame
{
    Q_OBJECT
public:
    Frame();
    ~Frame();

protected:
    void mousePressEvent(QMouseEvent*event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *);

public slots:
    void refreshAccountInfo();
    void autoRefresh();
    void shadowWidgetShow();
    void shadowWidgetHide();
    void setLanguage(QString);
    void syncFinished();

    void setCurrentAccount(QString accountName);

    void ShowMainPageSlot();

    void onCloseWallet();
signals:
    void delegateListUpdated();
    void titleBackVisible(bool);
private slots:
    void alreadyLogin();

    void showMinerPage();

    void showLockPage();
    void autoLock();
    void unlock();
    void updateTimer();
    void settingSaved();
    void privateKeyImported();

    void jsonDataUpdated(QString id);

    void onBack();
    void showMainPage();

    void showTransferPage();
    void showTransferPage(QString,QString);
    void showTransferPageWithAddress(QString, QString remark = "");//对方地址、名称

    void showContactPage();
    void showMultiSigPage();
    void showPoundagePage();//显示手续费承税单--朱正天
    void showOnchainOrderPage();
    void showMyExchangeContractPage();
    void showAssetPage();
    void showMultiSigTransactionPage(QString _multiSigAddress);
    void showWaittingForSyncWidget();
    void showNewOrImportWalletWidget();


    void iconIsActived(QSystemTrayIcon::ActivationReason reason);
    void showNormalAndActive();

    void newAccount(QString name);


private slots:
    //显示通知气泡
    void ShowBubbleMessage(const QString &title,const QString &context,QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information, int msecs = 10000);
private:
    bool mouse_press;
    QPoint move_point;
    SelectWalletPathWidget*   selectWalletPathWidget;
    FirstLogin* firstLogin;
    NormalLogin* normalLogin;

    MainPage*   mainPage;
    TransferPage* transferPage;
    MultiSigPage* multiSigPage;
    MultiSigTransactionPage* multiSigTransactionPage;
    MinerPage* minerPage;
    OnchainOrderPage* onchainOrderPage;
    MyExchangeContractPage* myExchangeContractPage;
    PoundageWidget *poundage;
    AssetPage*  assetPage;

    BottomBar* bottomBar;
    QWidget* centralWidget;
    LockPage*  lockPage;
    QTimer* timer;
    TitleBar* titleBar;
    QString currentAccount;
    WaitingForSync* waitingForSync;
    NewOrImportWalletWidget* newOrImportWalletWidget;

    int currentPageNum;  //  0:mainPage   1:  2:delegatePgae  3:transferPage    4:contactPage    5:myExchangeContractPage
                         //   6: onchainOrderPage   7:minerPage   8: assetPage     9: multiSigPage   10: multiSigTransactionPage
    ShadowWidget* shadowWidget;
    QSystemTrayIcon* trayIcon;
    void createTrayIconActions();
    void createTrayIcon();
    QAction *minimizeAction;
    QAction *restoreAction;
    QAction *quitAction;
    QMenu *trayIconMenu;
//    ShowBottomBarWidget* showBottomBarWidget;

#ifdef WIN32
    RECT rtConfined;   // 由于定义了 framelesswindowhint 为了不让鼠标拖动时能移到任务栏下
    RECT rtDefault;
#endif

    void  getAccountInfo();
    void startTimerForAutoRefresh();      // 自动刷新
    QTimer* timerForAutoRefresh;
    FunctionWidget *functionBar;
    void closeCurrentPage();
    bool eventFilter(QObject *watched, QEvent *e);
    void closeEvent(QCloseEvent* e);
    void init();

    ContactWidget *contactPage;

    QTranslator translator;         //  选择语言
    QTranslator menuTranslator;     //  右键菜单语言
    QTranslator translatorForTextBrowser;   // QTextBrowser的右键菜单翻译

    bool needToRefresh;

private:
    void paintEvent(QPaintEvent* e);
private slots:
    void EnlargeRightPart();
    void RestoreRightPart();

private:
    DepositAutomatic *autoDeposit;
public:
    CrossCapitalMark *crossMark;
public:
    void installBlurEffect(QWidget *widget);
};


#endif // FRAME_H
