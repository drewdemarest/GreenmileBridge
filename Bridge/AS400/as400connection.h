#ifndef AS400_H
#define AS400_H

#include "JsonSettings/jsonsettings.h"
#include <QObject>
#include <QtSql>

enum AS400QueryType {GreenmileRouteInfo, Invoice, CustomerChain, OpenOrderHeader, OpenOrderDetail};

class AS400 : public QObject
{
    Q_OBJECT
public:
    explicit AS400(QObject *parent = nullptr);

    explicit AS400(const QString &systemIP,
                   const QString &username,
                   const QString &password,
                   QObject *parent = nullptr);

    void init();

    bool getCustomerChains(const int chunkSize);

    bool getInvoiceData(const QDate &minDate,
                        const QDate &maxDate,
                        const int chunkSize);

    bool getOpenOrderHeaders(const int chunkSize);

    bool getOpenOrderDetails(const int chunkSize);

    bool getCustomerData();

    bool getRouteDataForGreenmile(const QDate &date, const int chunkSize);


signals:
    void invoiceDataResults(QMap<QString,QVariantList> sqlResults);
    void customerChainResults(QMap<QString,QVariantList> sqlResults);
    void openOrderHeaderResults(bool needToTruncate, QMap<QString,QVariantList> sqlResults);
    void openOrderDetailResults(bool needToTruncate, QMap<QString,QVariantList> sqlResults);
    void customerDataResults(QMap<QString,QVariantList> sqlResults);
    void routeAssignmentResults(QMap<QString,QVariantList> sqlResults);
    void greenmileRouteInfoResults(QMap<QString,QVariantList> sqlResults);
    void debugMessage(QString dbg);

private:
    bool queryAS400(const AS400QueryType queryType, const QString &queryString, const int chunkSize);
    void dispatchSqlResults(const bool isFirstRun,
                            const AS400QueryType queryType,
                            const QMap<QString, QVariantList> &sqlResults);
    void processQuery(const AS400QueryType queryType, QSqlQuery &query, const int chunkSize);
    void inputAS400Settings();

    QString dbPath_ = qApp->applicationDirPath() + "/as400settings.db";
    JsonSettings settings_;
    QJsonObject jsonSettings_   =   {{"username",       QJsonValue("username")},
                                     {"password",       QJsonValue("password")},
                                     {"system",         QJsonValue("0.0.0.0")},
                                     {"driver",         QJsonValue("iSeries Access ODBC Driver")}};
    //Data Formats
};

#endif // AS400_H
