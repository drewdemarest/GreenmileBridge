#include "locationupload.h"

LocationUpload::LocationUpload(QObject *parent) : QObject(parent)
{
    connect(gmConn_, &GMConnection::networkResponse, this, &LocationUpload::handleGMResponse);

    connect(gmConn_,    &GMConnection::statusMessage,   this, &LocationUpload::statusMessage);
    connect(gmConn_,    &GMConnection::errorMessage,    this, &LocationUpload::errorMessage);
    connect(gmConn_,    &GMConnection::debugMessage,    this, &LocationUpload::debugMessage);
    connect(gmConn_,    &GMConnection::failed,          this, &LocationUpload::handleFailure);

    //connect(bridgeDB_, &BridgeDatabase::statusMessage, this, &LocationUpload::statusMessage);
    connect(bridgeDB_, &BridgeDatabase::errorMessage, this, &LocationUpload::errorMessage);
    connect(bridgeDB_, &BridgeDatabase::statusMessage, this, &LocationUpload::statusMessage);
    connect(bridgeDB_, &BridgeDatabase::debugMessage, this, &LocationUpload::debugMessage);
    connect(bridgeDB_, &BridgeDatabase::failed, this, &LocationUpload::handleFailure);

}

LocationUpload::~LocationUpload()
{

}

//QJsonObject LocationUpload::getResults()
//{
//    return uploadedLocations_;
//}

void LocationUpload::reset()
{
    if(!activeJobs_.isEmpty())
    {
        errorMessage("Location upload in progress. Try again once current request is finished.");
        qDebug() << "Location upload in progress. Try again once current request is finished.";
        return;
    }

    currentKey_.clear();
    activeJobs_.clear();
    currentRequest_.clear();
    locationsToUpload_ = QJsonObject();
    uploadedLocations_ = QJsonObject();
}

void LocationUpload::UploadLocations(const QString &key, const QList<QVariantMap> &argList, const QJsonObject &geocodes)
{
    if(!activeJobs_.isEmpty())
    {
        errorMessage("Location upload in progress. Try again once current request is finished.");
        qDebug() << "Location upload in progress. Try again once current request is finished.";
        return;
    }

    currentKey_ = key;
    uploadedLocations_.empty();
    locationsToUpload_.empty();

    for(auto vMap:argList)
    {
        QString tableName       = vMap["tableName"].toString();
        QString organizationKey = vMap["organization:key"].toString();
        QDate date              = vMap["date"].toDate();
        QString minRouteKey     = vMap["minRouteKey"].toString();
        QString maxRouteKey     = vMap["maxRouteKey"].toString();
        QJsonObject locations   = bridgeDB_->getLocationsToUpload(tableName, organizationKey, date, minRouteKey, maxRouteKey);
        mergeLocationsToUpload(locations);
    }

    applyGeocodesToLocations(geocodes);

    for(auto key:locationsToUpload_.keys())
    {
        activeJobs_.insert(key);
        gmConn_->uploadALocation(key, locationsToUpload_[key].toObject());
    }

    if(activeJobs_.empty())
    {
        emit finished(currentKey_, QJsonObject());
        reset();
    }
}

void LocationUpload::UpdateLocations(const QString &key, const QList<QVariantMap> &argList, const QJsonObject &geocodes)
{
    if(!activeJobs_.isEmpty())
    {
        errorMessage("Location update in progress. Try again once current request is finished.");
        return;
    }

    currentKey_ = key;
    uploadedLocations_.empty();
    locationsToUpload_.empty();

    for(auto vMap:argList)
    {
        QString organizationKey = vMap["organization:key"].toString();

        mergeLocationsToUpload(bridgeDB_->getLocationsToUpdate(organizationKey));
        //mergeLocationsToUpload(bridgeDB_->getGMLocationsWithBadGeocode(organizationKey));
    }

    if(failState_)
        emit failed("Location database error", "Failed in initial LocationUpload::UpdateLocations location update.");

    applyGeocodesToLocations(geocodes);

    for(auto key:locationsToUpload_.keys())
    {
        activeJobs_.insert(key);
        qDebug() << QString::number(locationsToUpload_[key].toObject()["id"].toInt()) << locationsToUpload_[key].toObject();
        gmConn_->patchLocation(key, locationsToUpload_[key].toObject());
    }

    if(activeJobs_.empty())
    {
        emit finished(currentKey_, QJsonObject());
        reset();
    }
}

void LocationUpload::handleGMResponse(const QString &key, const QJsonValue &response)
{
    activeJobs_.remove(key);
    uploadedLocations_[key] = response;

    if(activeJobs_.empty())
    {
        emit finished(currentKey_, uploadedLocations_);
        reset();
    }
}

void LocationUpload::handleFailure(const QString &key, const QString &reason)
{
    qDebug() << "LocationUpload::handleFailure Fail Key " << key;
    qDebug() << "LocationUpload::handleFailure Fail Reason " << reason;
    failState_ = true;
}

void LocationUpload::mergeLocationsToUpload(const QJsonObject &locations)
{
    for(auto key:locations.keys())
    {
        locationsToUpload_[key] = locations[key];
    }
}

void LocationUpload::applyGeocodesToLocations(const QJsonObject &geocodes)
{
    for(auto key:geocodes.keys())
    {
        QJsonObject jObj = locationsToUpload_[key].toObject();

        jObj["geocodingQuality"]    = geocodes[key]["geocodingQuality"];
        jObj["latitude"]            = geocodes[key]["latitude"];
        jObj["longitude"]           = geocodes[key]["longitude"];
        jObj["geocodingDate"]       = geocodes[key]["geocodingDate"];
        locationsToUpload_[key] = QJsonValue(jObj);
    }
}
