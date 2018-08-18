#ifndef ROUTECHECK_H
#define ROUTECHECK_H

#include "Bridge/Greenmile/gmconnection.h"
#include "Bridge/bridgedatabase.h"
#include <QObject>

class RouteCheck : public QObject
{
    Q_OBJECT
public:
    explicit RouteCheck(QObject *parent = nullptr);

signals:
    void debugMessage(const QString &debug);
    void errorMessage(const QString &error);
    void statusMessage(const QString &status);
    void finished(const QString &key, const QJsonObject &result);

public slots:
    void UploadRoutes(const QString &key, const QList<QVariantMap> &argList);

private slots:
    void handleGMResponse(const QString &key, const QJsonValue &response);

private:
    GMConnection *gmConn_ = new GMConnection(this);
    BridgeDatabase *bridgeDB_ = new BridgeDatabase(this);

    QString currentKey_;
    QVariantMap currentRequest_;
    QSet<QString> activeJobs_;

    QJsonObject routesToUpload_;
    QJsonObject uploadedRoutes_;
    void mergeRoutesToUpload(const QJsonObject &locations);
    void reset();
};
};

#endif // ROUTECHECK_H
