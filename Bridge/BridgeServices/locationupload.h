#ifndef LOCATIONUPLOAD_H
#define LOCATIONUPLOAD_H

#include "Bridge/Greenmile/gmconnection.h"
#include "Bridge/bridgedatabase.h"
#include <QObject>

class LocationUpload : public QObject
{
    Q_OBJECT
public:
    explicit LocationUpload(QObject *parent = nullptr);
    virtual ~LocationUpload();
    //QJsonObject getResults();

signals:
    void statusMessage(const QString &status);
    void debugMessage(const QString &debug);
    void errorMessage(const QString &error);
    void finished(const QString &key, const QJsonObject &result);
    void failed(const QString &key, const QString &reason);

public slots:
//    void UploadLocationTypes(const QString &key, const QList<QVariantMap> &argList);
//    void UploadAccountTypes(const QString &key, const QList<QVariantMap> &argList);
//    void UploadServiceTimeTypes(const QString &key, const QList<QVariantMap> &argList);
    void UploadLocations(const QString &key, const QList<QVariantMap> &argList, const QJsonObject &geocodes);
    void UpdateLocations(const QString &key, const QList<QVariantMap> &argList, const QJsonObject &geocodes);

private slots:
    void handleGMResponse(const QString &key, const QJsonValue &response);
    void handleFailure(const QString &key, const QString &reason);

private:
    GMConnection *gmConn_ = new GMConnection(this);
    BridgeDatabase *bridgeDB_ = new BridgeDatabase(this);

    QString currentKey_;
    QVariantMap currentRequest_;
    QSet<QString> activeJobs_;

    QJsonObject locationsToUpload_;
    QJsonObject uploadedLocations_;
    void mergeLocationsToUpload(const QJsonObject &locations);
    void applyGeocodesToLocations(const QJsonObject &geocodes);
    void reset();

    bool failState_ = false;

};

#endif // LOCATIONUPLOAD_H
