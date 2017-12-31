#if defined(HAVE_CONFIG_H)
#include "config/bitcoin-config.h"
#endif
 
#include "smartrewardslist.h"
#include "ui_smartrewardslist.h"
 
#include "addresstablemodel.h"
#include "bitcoinunits.h"
#include "guiutil.h"
#include "optionsmodel.h"
#include "platformstyle.h"
#include "txmempool.h"
#include "walletmodel.h"
#include "coincontrol.h"
#include "init.h"
#include "main.h" // For minRelayTxFee
#include "wallet/wallet.h"
 
#include <boost/assign/list_of.hpp> // for 'map_list_of()'
 
#include <QIcon>
#include <QMenu>
#include <QMessageBox>
#include <QSortFilterProxyModel>
#include <QTableWidget>
#include <QTime>

int GetTimeOffsetFromUtc()
{
#if QT_VERSION < 0x050200
    const QDateTime dateTime1 = QDateTime::currentDateTime();
    const QDateTime dateTime2 = QDateTime(dateTime1.date(), dateTime1.time(), Qt::UTC);
    return dateTime1.secsTo(dateTime2);
#else
    return QDateTime::currentDateTime().offsetFromUtc();
#endif
}

SmartrewardsList::SmartrewardsList(const PlatformStyle *platformStyle, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SmartrewardsList),
    model(0)
{
    ui->setupUi(this);
 
    int columnAliasWidth = 200;
    int columnAddressWidth = 250;
    int columnAmountWidth = 160;
    int columnSmartAmountWidth = 200;
 
    ui->tableWidget->setColumnWidth(0, columnAliasWidth);
    ui->tableWidget->setColumnWidth(1, columnAddressWidth);
    ui->tableWidget->setColumnWidth(2, columnAmountWidth);
    ui->tableWidget->setColumnWidth(3, columnSmartAmountWidth);
 
}
 
SmartrewardsList::~SmartrewardsList()
{
    delete ui;
}
 
 void SmartrewardsList::setModel(WalletModel *model)
{
    this->model = model;
    if(!model) {
       return;
    }

    ui->tableWidget->setAlternatingRowColors(true);
 
    int nDisplayUnit = model->getOptionsModel()->getDisplayUnit();
 
    std::map<QString, std::vector<COutput> > mapCoins;
    model->listCoins(mapCoins);
 
    ui->tableWidget->setColumnCount(4);
    ui->tableWidget->setShowGrid(false);
 
    int nNewRow = 0;

    //Smartrewards snapshot
    QDateTime lastSmartrewardsSnapshotDateTimeUtc = QDateTime::currentDateTime();
    int currentDay = QDateTime::currentDateTimeUtc().toString("dd").toInt();

    //Set snapshot time
    QString snapshotTime = QString("%1:00:00").arg(SMARTREWARDS_UTC_HOUR);
    lastSmartrewardsSnapshotDateTimeUtc.setTime(QTime::fromString(snapshotTime, "hh:mm:ss"));

    //Set snapshot date
    if(currentDay < SMARTREWARDS_DAY){
       lastSmartrewardsSnapshotDateTimeUtc = lastSmartrewardsSnapshotDateTimeUtc.addMonths(-1);
       QString snapshotDate = QString("%1").arg(SMARTREWARDS_DAY);
       lastSmartrewardsSnapshotDateTimeUtc.setDate(QDate::fromString(snapshotDate, "dd"));
    }else{
        QString snapshotDate = QString("%1").arg(SMARTREWARDS_DAY);
        lastSmartrewardsSnapshotDateTimeUtc.setDate(QDate::fromString(snapshotDate, "dd"));
    }
 
    BOOST_FOREACH(const PAIRTYPE(QString, std::vector<COutput>)& coins, mapCoins) {
        QString sWalletAddress = coins.first;
        QString sWalletLabel = model->getAddressTableModel()->labelForAddress(sWalletAddress);
        if (sWalletLabel.isEmpty())
            sWalletLabel = tr("(no label)");
 
        ui->tableWidget->insertRow(nNewRow);
 
        CAmount nSum = 0;
        int nChildren = 0;
        BOOST_FOREACH(const COutput& out, coins.second) {

            nSum += out.tx->vout[out.i].nValue;
            nChildren++;
 
            //address
            CTxDestination outputAddress;
            QString sAddress = "";
            if(ExtractDestination(out.tx->vout[out.i].scriptPubKey, outputAddress))
            {
                sAddress = QString::fromStdString(CBitcoinAddress(outputAddress).ToString());
            }

            //tx date
            int64_t nTimeTx = out.tx->GetTxTime();
            QDateTime txDateTime = QDateTime::fromTime_t((qint32)nTimeTx);
            QDateTime txDateTimeUtc = txDateTime.toUTC();

            if(txDateTimeUtc > lastSmartrewardsSnapshotDateTimeUtc){
                int a = 1;
            }else{
                int a = 2;
            }

            ui->tableWidget->setItem(nNewRow, 0, new QTableWidgetItem(sWalletLabel));
            ui->tableWidget->setItem(nNewRow, 1, new QTableWidgetItem(sWalletAddress));
            ui->tableWidget->setItem(nNewRow, 2, new QTableWidgetItem(BitcoinUnits::format(nDisplayUnit, nSum)));

            //eligible rewards
            if(nSum >= SMARTREWARDS_MINIMUM_AMOUNT){
                 ui->tableWidget->setItem(nNewRow, 3, new QTableWidgetItem(BitcoinUnits::format(nDisplayUnit, nSum)));
            }else{
                 ui->tableWidget->setItem(nNewRow, 3, new QTableWidgetItem(BitcoinUnits::format(nDisplayUnit, 0)));
            }

        }
        nNewRow++;
 
    }

}
 
