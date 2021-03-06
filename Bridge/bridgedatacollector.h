#ifndef BRIDGEDATACOLLECTOR_H
#define BRIDGEDATACOLLECTOR_H

#include <QtCore>
#include <QObject>
#include "Bridge/bridgedatabase.h"
#include "MasterRoute/mrsconnection.h"
#include "GoogleSheets/googlesheetsconnection.h"
#include "Greenmile/gmconnection.h"
#include "AS400/as400connection.h"
#include "JsonSettings/jsonsettings.h"

class BridgeDataCollector : public QObject
{
    Q_OBJECT
public:
    explicit BridgeDataCollector(QObject *parent = Q_NULLPTR);
    virtual ~BridgeDataCollector();
    bool hasActiveJobs();
    void addRequest(const QString &key, const QDate &date, const int monthsUntilCustDisabled, const QStringList &sourceOverrides = QStringList());
    void removeRequest(const QString &key);
    void buildTables();

    void handleAS400RouteQuery(const QMap<QString, QVariantList> &sql);
    void handleAS400LocationQuery(const QMap<QString, QVariantList> &sql);

    void handleGMLocationInfo(const QJsonArray &array);
    void handleGMLocationOverrideTimeWindowInfo(const QJsonArray &array);
    void handleGMDriverInfo(const QJsonArray &array);
    void handleGMEquipmentInfo(const QJsonArray &array);
    void handleGMOrganizationInfo(const QJsonArray &array);
    void handleGMRouteInfo(const QJsonArray &array);
    void handleGMServiceTimeTypes(const QJsonArray &array);
    void handleGMAccountTypes(const QJsonArray &array);
    void handleGMLocationTypes(const QJsonArray &array);
    void handleGMStopTypes(const QJsonArray &array);

    void handleRSAssignments(const QString &tableName, const QMap<QString, QVariantList> &sql);
    void handleRSDrivers(const QJsonObject &data);
    void handleRSPowerUnits(const QJsonObject &data);
    void handleRSRouteStartTimes(const QJsonObject &data);
    void handleRSRouteOverrides(const QJsonObject &data);
    void handleJobCompletion(const QString &key);

signals:
    void statusMessage(const QString &status);
    void debugMessage(const QString &debug);
    void errorMessage(const QString &error);

    void failed(const QString &key, const QString &reason);
    void finished(const QString &key);
    void progress(const int remainingJobs, const int totalJobs);

private slots:
    void handleSQLResponse(const QString &key, const QMap<QString, QVariantList> &sql);
    void handleJsonResponse(const QString &key, const QJsonValue &jVal);
    void handleComponentFailure(const QString &key, const QString &reason);

private:
    JsonSettings *jsonSettings_         = new JsonSettings(this);
    QString scheduleSheetDbPath_       = qApp->applicationDirPath() + "/scheduleconfig.db";
    QJsonObject scheduleSheetSettings_  {{"scheduleList",QJsonArray()}};

    QQueue<QVariantMap> requestQueue_;
    QSet<QString> activeJobs_;

    void assembleKnownSources();
    void prepDatabases(const QStringList &sourceOverrides = QStringList());
    void beginGathering(const QVariantMap &request);

    BridgeDatabase *bridgeDB                = new BridgeDatabase(this);
    QMap<QString,MRSConnection*>            scheduleSheets;
    GoogleSheetsConnection *routeSheetData  = new GoogleSheetsConnection("mrsdataconnection.db", this);
    GMConnection *gmConn                    = new GMConnection(this);
    AS400 *as400                            = new AS400(this);
    QTimer *queueTimer                      = new QTimer(this);

    QJsonObject knownSourcesJson_;

    QStringList knownSources_ =
    {"routeStartTimes",
     "drivers",
     "powerUnits",
     "routeOverrides",
     "gmOrganizations",
     "gmRoutes",
     "gmDrivers",
     "gmEquipment",
     "gmLocations",
     "gmLocationOverrideTimeWindows",
     "gmServiceTimeTypes",
     "gmAccountTypes",
     "gmStopTypes",
     "gmLocationTypes",
     "as400RouteQuery",
     "as400LocationQuery"};

    int totalJobs_ = 0;
    QString currentKey_;

    void processQueue();

    void generateScheduleSheets();
    void deleteScheduleSheets();
};

#endif // BRIDGEDATACOLLECTOR_H
