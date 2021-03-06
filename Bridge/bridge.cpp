#include "bridge.h"

Bridge::Bridge(QObject *parent) : QObject(parent)
{
    qDebug() << "bridge ctor";
    connect(queueTimer, &QTimer::timeout, this, &Bridge::processQueue);
    connect(bridgeTimer, &QTimer::timeout, this, &Bridge::startOnTimer);
    connect(bridgeMalfunctionTimer, &QTimer::timeout, this, &Bridge::abort);
    connect(componentDeletionTimer, &QTimer::timeout, this, &Bridge::componentsDeleted);
    init();
}

Bridge::~Bridge()
{

}

void Bridge::init()
{
    connect(dataCollector, &BridgeDataCollector::finished,      this,   &Bridge::finishedDataCollection);
    connect(dataCollector, &BridgeDataCollector::failed,        this,   &Bridge::handleComponentFailure);

    connect(accountType_,           &AccountType::finished,     this,   &Bridge::finishedAccountTypes);
    connect(accountType_,           &AccountType::failed,       this,   &Bridge::handleComponentFailure);

    connect(serviceTimeType_,       &ServiceTimeType::finished, this,   &Bridge::finishedServiceTimeTypes);
    connect(serviceTimeType_,       &ServiceTimeType::failed,   this,   &Bridge::handleComponentFailure);

    //connect(locationType_,       &LocationType::finished,       this,   &Bridge::finishedLocationTypes);
    //connect(locationType_,       &LocationType::failed,         this,   &Bridge::handleComponentFailure);

    connect(locationUpdateGeocode_, &LocationGeocode::finished, this,   &Bridge::finishedLocationUpdateGeocode);
    connect(locationUpdateGeocode_, &LocationGeocode::failed,   this,   &Bridge::handleComponentFailure);
    connect(locationUpdate_,        &LocationUpload::finished,  this,   &Bridge::finishedLocationUpdate);
    connect(locationUpdate_,        &LocationUpload::failed,    this,   &Bridge::handleComponentFailure);

    connect(locationUploadGeocode_, &LocationGeocode::finished, this,   &Bridge::finishedLocationUploadGeocode);
    connect(locationUploadGeocode_, &LocationGeocode::failed,   this,   &Bridge::handleComponentFailure);
    connect(locationUpload_,        &LocationUpload::finished,  this,   &Bridge::finishedLocationUpload);
    connect(locationUpload_,        &LocationUpload::failed,    this,   &Bridge::handleComponentFailure);

    connect(lotw_,                  &LocationOverrideTimeWindow::finished, this,    &Bridge::finishedLocationOverrideTimeWindows);
    connect(lotw_,                  &LocationOverrideTimeWindow::failed, this,      &Bridge::handleComponentFailure);

    connect(routeCheck_,                &RouteCheck::finished,                  this, &Bridge::finishedRouteCheck);
    connect(routeUpload_,               &RouteUpload::finished,                 this, &Bridge::finishedRouteUpload);
    connect(routeAssignmentCorrection_, &RouteAssignmentCorrection::finished,   this, &Bridge::finishedRouteAssignmentCorrections);

    connect(dataCollector, &BridgeDataCollector::progress, this, &Bridge::currentJobProgress);
    connect(dataCollector, &BridgeDataCollector::statusMessage, this, &Bridge::statusMessage);
    connect(dataCollector, &BridgeDataCollector::debugMessage, this, &Bridge::debugMessage);
    connect(dataCollector, &BridgeDataCollector::errorMessage, this, &Bridge::errorMessage);

    connect(locationUploadGeocode_, &LocationGeocode::statusMessage,    this, &Bridge::statusMessage);
    connect(locationUploadGeocode_, &LocationGeocode::debugMessage,     this, &Bridge::debugMessage);
    connect(locationUploadGeocode_, &LocationGeocode::errorMessage,     this, &Bridge::errorMessage);

    connect(locationUpload_, &LocationUpload::statusMessage,    this, &Bridge::statusMessage);
    connect(locationUpload_, &LocationUpload::debugMessage,     this, &Bridge::debugMessage);
    connect(locationUpload_, &LocationUpload::errorMessage,     this, &Bridge::errorMessage);

    connect(locationUpdateGeocode_, &LocationGeocode::statusMessage,    this, &Bridge::statusMessage);
    connect(locationUpdateGeocode_, &LocationGeocode::debugMessage,     this, &Bridge::debugMessage);
    connect(locationUpdateGeocode_, &LocationGeocode::errorMessage,     this, &Bridge::errorMessage);

    connect(locationUpdate_, &LocationUpload::statusMessage,    this, &Bridge::statusMessage);
    connect(locationUpdate_, &LocationUpload::debugMessage,     this, &Bridge::debugMessage);
    connect(locationUpdate_, &LocationUpload::errorMessage,     this, &Bridge::errorMessage);

    connect(lotw_, &LocationOverrideTimeWindow::statusMessage,    this, &Bridge::statusMessage);
    connect(lotw_, &LocationOverrideTimeWindow::debugMessage,     this, &Bridge::debugMessage);
    connect(lotw_, &LocationOverrideTimeWindow::errorMessage,     this, &Bridge::errorMessage);

    connect(accountType_, &AccountType::statusMessage,    this, &Bridge::statusMessage);
    connect(accountType_, &AccountType::debugMessage,     this, &Bridge::debugMessage);
    connect(accountType_, &AccountType::errorMessage,     this, &Bridge::errorMessage);

    connect(routeCheck_, &RouteCheck::statusMessage, this, &Bridge::statusMessage);
    connect(routeCheck_, &RouteCheck::debugMessage, this, &Bridge::debugMessage);
    connect(routeCheck_, &RouteCheck::errorMessage, this, &Bridge::errorMessage);

    connect(routeUpload_, &RouteUpload::statusMessage, this, &Bridge::statusMessage);
    connect(routeUpload_, &RouteUpload::debugMessage, this, &Bridge::debugMessage);
    connect(routeUpload_, &RouteUpload::errorMessage, this, &Bridge::errorMessage);

    connect(routeAssignmentCorrection_, &RouteAssignmentCorrection::statusMessage, this, &Bridge::statusMessage);
    connect(routeAssignmentCorrection_, &RouteAssignmentCorrection::errorMessage, this, &Bridge::errorMessage);

    connect(this, &Bridge::statusMessage, logger_, &LogWriter::writeLogEntry);
    connect(this, &Bridge::debugMessage, logger_, &LogWriter::writeLogEntry);
    connect(this, &Bridge::errorMessage, logger_, &LogWriter::writeLogEntry);

    loadSettings();
    int intervalTimer = settings_["bridgeIntervalSec"].toInt() * 1000;
    qDebug() << intervalTimer << "interval timer!";
    bridgeTimer->start(intervalTimer);
    queueTimer->start(1000);
}

void Bridge::loadSettings()
{
    qDebug() << "starting settings load";

    bridgeSettings_     = jsonSettings_->loadSettings(QFile(bridgeSettingsDbPath_),bridgeSettings_);
    qDebug() << bridgeSettings_ << bridgeSettingsDbPath_;
    scheduleSettings_   = jsonSettings_->loadSettings(QFile(scheduleSettingsDbPath_),scheduleSettings_);
    qDebug() << scheduleSettings_ << scheduleSettingsDbPath_;

    settings_["daysToUpload"]               = QJsonValue(generateUploadDays(bridgeSettings_["daysToUploadInt"].toInt()));
    settings_["scheduleTables"]             = QJsonValue(scheduleSettings_["scheduleList"].toArray());
    settings_["organization:key"]           = QJsonValue(bridgeSettings_["organization:key"].toString());
    settings_["monthsUntilCustDisabled"]    = QJsonValue(bridgeSettings_["monthsUntilCustDisabled"].toInt());
    settings_["schedulePrimaryKeys"]        = QJsonValue(QJsonArray{"route:key", "route:date", "organization:key"});
    settings_["bridgeIntervalSec"]          = QJsonValue(bridgeSettings_["bridgeIntervalSec"].toInt());

    qDebug() << "settings loaded" <<  settings_;
}

QJsonArray Bridge::generateUploadDays(int daysToUpload)
{
    QJsonArray dateArr;
    dateArr.append(QJsonValue(QDate::currentDate().toString(Qt::ISODate)));

    for(int i = 0; i < daysToUpload; i++){
        QDate today = QDate::currentDate();
        dateArr.append(QJsonValue(today.addDays(i+1).toString(Qt::ISODate)));
    }
    return dateArr;
}

void Bridge::startOnTimer()
{
    addRequest("AUTO_START_TIMER");
}

bool Bridge::hasActiveJobs()
{
    if(activeJobs_.isEmpty())
        return false;
    else
        return true;
}

void Bridge::addRequest(const QString &key)
{
    qDebug() << "request addedddd!";
    QVariantMap request;

    for(auto jVal : settings_["daysToUpload"].toArray())
    {
        request["key"] = key;
        request["organization:key"] = settings_["organization:key"].toString();
        request["date"] = QDate::fromString(jVal.toString(), Qt::ISODate);
        request["scheduleTables"] = settings_["scheduleTables"].toArray();
        request["schedulePrimaryKeys"] = settings_["schedulePrimaryKeys"].toArray();
        request["monthsUntilCustDisabled"] = settings_["monthsUntilCustDisabled"].toInt();
        requestQueue_.enqueue(request);
    }
}

void Bridge::removeRequest(const QString &key)
{
    for(int i = 0; i < requestQueue_.size(); i++)
        if(requestQueue_[i]["key"] == key)
            requestQueue_.removeAt(i);
}

void Bridge::handleComponentFailure(const QString &key, const QString &reason)
{
    emit errorMessage(key + ". Aborting bridge as soon as possible. " + reason);
    failState_ = true;
}

void Bridge::processQueue()
{
    if(hasActiveJobs() || requestQueue_.isEmpty())
        return;
    else
    {
        loadSettings();
        currentRequest_ = requestQueue_.dequeue();

        emit started(currentRequest_["key"].toString());
        QString jobKey = "initialCollection:" + currentRequest_["key"].toString();

        emit statusMessage("-------------------------------------------------");
        emit statusMessage("Bridge started for: " + currentRequest_["key"].toString() + "_" + currentRequest_["date"].toDate().toString(Qt::ISODate) + ".");
        emit statusMessage("-------------------------------------------------");

        emit bridgeKeyChanged(currentRequest_["key"].toString() + "_" + currentRequest_["date"].toDate().toString(Qt::ISODate));
        addActiveJob(jobKey);
        startDataCollection(jobKey, currentRequest_["date"].toDate(), currentRequest_["monthsUntilCustDisabled"].toInt());

        bridgeMalfunctionTimer->start(1800000);
    }
}

void Bridge::addActiveJob(const QString &key)
{
    //addActiveJob(jobKey);
    emit currentJobChanged(key);
    activeJobs_.insert(key);

    ++totalJobCount_;
    activeJobCount_ = activeJobs_.size();

    emit bridgeProgress(activeJobCount_, totalJobCount_);
}

void Bridge::removeActiveJob(const QString &key)
{
    qDebug() << "Bridge::removeActiveJob " << key;
    activeJobs_.remove(key);
    qDebug() << "Bridge::removeActiveJob activeJobs_.remove(key) " << activeJobs_;
    activeJobCount_ = activeJobs_.size();
    qDebug() << "Bridge::removeActiveJob activeJobs_.size() " << activeJobs_.size();

    emit bridgeProgress(activeJobCount_, totalJobCount_);
}

void Bridge::startDataCollection(const QString &key, const QDate &date, const int monthsUntilCustDisabled)
{
    qDebug() << "Bridge::startDataCollection";
    emit currentJobChanged(key);
    dataCollector->addRequest(key, date, monthsUntilCustDisabled);
}

void Bridge::finishedDataCollection(const QString &key)
{
    qDebug() << "Bridge::finishedDataCollection";
    emit statusMessage(key + " has been completed.");
    bridgeDB_->reprocessAS400LocationTimeWindows();
    applyScheduleHierarchy();
    generateArgs();

    if(key.split(":").first() == "initialCollection")
    {
        emit statusMessage("Processing account types.");
        QString jobKey = "accountTypes:" + currentRequest_["key"].toString();
        qDebug() << "Do I go here 0?";
        addActiveJob(jobKey);
        accountType_->processAccountTypes(jobKey, argList_);
    }

    if(key.split(":").first() == "refreshDataForRouteAssignmentCorrections")
    {
        qDebug() << "correcting route assignments";
        QString jobKey = "routeAssignmentCorrections:" + currentRequest_["key"].toString();
        addActiveJob(jobKey);
        routeAssignmentCorrection_->CorrectRouteAssignments(jobKey, argList_);
    }

    handleJobCompletion(key);
}

void Bridge::finishedAccountTypes(const QString &key, const QMap<QString,QJsonObject> &result)
{
    emit statusMessage(key + " has been completed.");
    qDebug() << result;
    QString jobKey = "serviceTimeTypes:" + currentRequest_["key"].toString();
    qDebug() << "Do I go here 0.1?";
    addActiveJob(jobKey);
    serviceTimeType_->processServiceTimeTypes(jobKey, argList_);
    handleJobCompletion(key);
}

void Bridge::finishedServiceTimeTypes(const QString &key, const QMap<QString, QJsonObject> &result)
{
    emit statusMessage(key + " has been completed.");
    qDebug() << result;
    QString jobKey = "geocodeUpdatedLocations:" + currentRequest_["key"].toString();
    qDebug() << "Do I go here 0.3?";
    addActiveJob(jobKey);
    locationUpdateGeocode_->GeocodeLocations(jobKey, argList_, true, false);
    handleJobCompletion(key);
}

void Bridge::finishedLocationTypes(const QString &key, const QMap<QString, QJsonObject> &result)
{
    emit statusMessage(key + " has been completed.");
    qDebug() << result;
    QString jobKey = "geocodeUpdatedLocations:" + currentRequest_["key"].toString();
    qDebug() << "Do I go here 0.3?";
    addActiveJob(jobKey);
    locationUpdateGeocode_->GeocodeLocations(jobKey, argList_, true, false);
    handleJobCompletion(key);
}

void Bridge::finishedLocationUpdateGeocode(const QString &key, const QJsonObject &result)
{
    qDebug() << "Bridge::finishedLocationUpdateGeocode";
    emit statusMessage(key + " has been completed.");

    QString jobKey = "updateLocations:" + currentRequest_["key"].toString();
    addActiveJob(jobKey);
    qDebug() << "Do I go here 1.1?";

    locationUpdate_->UpdateLocations(jobKey, argList_, result);
    handleJobCompletion(key);
}

void Bridge::finishedLocationUpdate(const QString &key, const QJsonObject &result)
{
    qDebug() << "Bridge::finishedLocationUpdate";
    emit statusMessage(key + " has been completed.");
    qDebug() << result;
    qDebug() << "Do I go here 2?";

    QString jobKey = "geocodeUploadLocations:" + currentRequest_["key"].toString();
    addActiveJob(jobKey);
    locationUploadGeocode_->GeocodeLocations(jobKey, argList_, false, false);
    handleJobCompletion(key);
}

void Bridge::finishedLocationUploadGeocode(const QString &key, const QJsonObject &result)
{
    qDebug() << "Bridge::finishedLocationUploadGeocode";
    emit statusMessage(key + " has been completed.");
    qDebug() << "Do I go here 3?";
    QString jobKey = "uploadLocations:" + currentRequest_["key"].toString();
    addActiveJob(jobKey);
    locationUpload_->UploadLocations(jobKey, argList_, result);
    handleJobCompletion(key);
}

void Bridge::finishedLocationUpload(const QString &key, const QJsonObject &result)
{
    qDebug() << "Bridge::finishedLocationUpload";
    emit statusMessage(key + " has been completed.");
    qDebug() << result;
    qDebug() << "Do I go here 4?";
    QString jobKey = "locationOverrideTimeWindows:" + currentRequest_["key"].toString();
    addActiveJob(jobKey);
    lotw_->processLocationOverrideTimeWindows(jobKey, argList_);
    //routeCheck_->deleteIncorrectRoutes(jobKey, argList_);
    handleJobCompletion(key);
}

void Bridge::finishedLocationOverrideTimeWindows(const QString &key, const QMap<QString,QJsonObject> &result)
{
    qDebug() << "Bridge::finishedLocationOverrideTimeWindows";
    emit statusMessage(key + " has been completed. ");

    for(auto resultKey: result.keys())
    {
        QString resultVerb = resultKey.split(":").first();

        if(resultVerb.back() == 'e')
        {
            resultVerb = resultVerb + "d";
        }
        else
        {
            resultVerb = resultVerb+"ed";
        }

        emit statusMessage(key + " " +  QString::number(result[resultKey].size()) + " have been "+resultVerb+".");
    }

    QString jobKey = "routeCheck:" + currentRequest_["key"].toString();
    addActiveJob(jobKey);
    routeCheck_->deleteIncorrectRoutes(jobKey, argList_);
    handleJobCompletion(key);
}

void Bridge::finishedRouteCheck(const QString &key, const QJsonObject &result)
{
    qDebug() << "Bridge::finishedRouteCheck";
    emit statusMessage(key + " has been completed.");
    qDebug() << result;
    qDebug() << "Do I go here 5?";
    qDebug() << "UPLOAD THE ROUTES";
    QString jobKey = "uploadRoutes:" + currentRequest_["key"].toString();
    addActiveJob(jobKey);
    routeUpload_->UploadRoutes(jobKey, argList_);
    handleJobCompletion(key);
}

void Bridge::finishedRouteUpload(const QString &key, const QJsonObject &result)
{
    qDebug() << "Bridge::finishedRouteUpload";
    emit statusMessage(key + " has been completed.");
    qDebug() << result;

    QString jobKey = "refreshDataForRouteAssignmentCorrections:" + currentRequest_["key"].toString();
    addActiveJob(jobKey);
    dataCollector->addRequest(  jobKey,
                                currentRequest_["date"].toDate(),
                                currentRequest_["monthsUntilCustDisabled"].toInt(),
                                QStringList{"gmRoutes"});

    handleJobCompletion(key);
}

void Bridge::finishedRouteAssignmentCorrections(const QString &key, const QJsonObject &result)
{
    qDebug() << "Bridge::finishedRouteAssignmentCorrections";
    emit statusMessage(key + " has been completed.");
    qDebug() << result;
    handleJobCompletion(key);
}

void Bridge::handleJobCompletion(const QString &key)
{    
    if(failState_)
    {
        abort();
        return;
    }

    qDebug() << "IMPORTANT DEBUG" << activeJobs_.size() << " jobs remaining" << activeJobs_ << "job completed" << key;

    if(currentRequest_["key"].toString().isEmpty())
    {
        emit errorMessage("ERROR: Job key has no value. Something went wrong, aborting.");
        handleComponentFailure("Bridge job", "Empty job key error.");
    }

    emit statusMessage("The remaining bridge jobs are: " + activeJobs_.toList().join(",") + ".");
    removeActiveJob(key);
    if(!hasActiveJobs())
    {
        currentJobChanged(QString());
        activeJobCount_ = 0;
        totalJobCount_ = 0;

        bridgeProgress(activeJobCount_, totalJobCount_);

        bridgeMalfunctionTimer->stop();
        emit finished(currentRequest_["key"].toString());
        emit currentJobChanged("Done");

        qDebug() << "Finished job with key " << currentRequest_["key"].toString();
        emit statusMessage("-------------------------------------------------");
        emit statusMessage("Bridge finished for: " + currentRequest_["key"].toString() + "_" + currentRequest_["date"].toDate().toString(Qt::ISODate) + ".");
        emit statusMessage("-------------------------------------------------");

        currentRequest_.clear();
    }
}

void Bridge::abort()
{
    qDebug() << "abortInProcess_" << abortInProcess_;
    qDebug() << "failState_" << failState_;
    qDebug() << "abortInProcess_ && !failState_" << (abortInProcess_ && !failState_);
    if(abortInProcess_){
        qDebug() << "Abort signal recieved, but ignoring as abort is in progress";
        emit errorMessage("Abort signal recieved, but ignoring as abort is in progress");
        return;
    }

    abortInProcess_ = true;

    qDebug() << "2abortInProcess_" << abortInProcess_;
    qDebug() << "2failState_" << failState_;
    qDebug() << "2abortInProcess_ && !failState_" << (abortInProcess_ && !failState_);

    qDebug() << "Abort";
    QString key = currentRequest_["key"].toString();
    emit errorMessage("ERROR: " + key + " ABORTED.");

    queueTimer->stop();
    bridgeTimer->stop();
    bridgeMalfunctionTimer->stop();

    requestQueue_.clear();
    argList_.clear();
    currentRequest_.clear();
    activeJobs_.clear();

    dataCollector->deleteLater();
    bridgeDB_->deleteLater();
    locationUploadGeocode_->deleteLater();
    locationUpload_->deleteLater();
    locationUpdateGeocode_->deleteLater();
    locationUpdate_->deleteLater();
    routeCheck_->deleteLater();
    routeUpload_->deleteLater();
    routeAssignmentCorrection_->deleteLater();
    logger_->deleteLater();
    lotw_->deleteLater();
    accountType_->deleteLater();
    serviceTimeType_->deleteLater();
    locationType_->deleteLater();

    totalJobCount_ = 0;
    activeJobCount_ = 0;
    emit bridgeKeyChanged("ABORTED");
    emit currentJobChanged("ABORTED");
    emit bridgeProgress(0, 0);
    emit currentJobProgress(0, 0);
    emit aborted(key);

    componentDeletionTimer->start(1000);
}

void Bridge::componentsDeleted()
{
    if(dataCollector                        == Q_NULLPTR
            && bridgeDB_                    == Q_NULLPTR
            && locationUploadGeocode_       == Q_NULLPTR
            && locationUpload_              == Q_NULLPTR
            && locationUpdateGeocode_       == Q_NULLPTR
            && locationUpdate_              == Q_NULLPTR
            && routeCheck_                  == Q_NULLPTR
            && routeUpload_                 == Q_NULLPTR
            && routeAssignmentCorrection_   == Q_NULLPTR
            && logger_                      == Q_NULLPTR
            && lotw_                        == Q_NULLPTR
            && accountType_                 == Q_NULLPTR
            && serviceTimeType_             == Q_NULLPTR
            && locationType_                == Q_NULLPTR)
    {
        componentDeletionTimer->stop();
        rebuild();
    }
}

void Bridge::rebuild()
{
    dataCollector               = new BridgeDataCollector(this);
    bridgeDB_                   = new BridgeDatabase(this);
    locationUploadGeocode_      = new LocationGeocode(this);
    locationUpload_             = new LocationUpload(this);
    locationUpdateGeocode_      = new LocationGeocode(this);
    locationUpdate_             = new LocationUpload(this);
    routeCheck_                 = new RouteCheck(this);
    routeUpload_                = new RouteUpload(this);
    routeAssignmentCorrection_  = new RouteAssignmentCorrection(this);
    logger_                     = new LogWriter(this);
    lotw_                       = new LocationOverrideTimeWindow(this);
    accountType_                = new AccountType(this);
    serviceTimeType_            = new ServiceTimeType(this);
    locationType_               = new LocationType(this);
    failState_ = false;
    abortInProcess_ = false;
    init();


    emit statusMessage("-------------------------------------------------");
    emit statusMessage("Bridge has been reset. Restarting queue.");
    emit statusMessage("-------------------------------------------------");

    qDebug() << 12;
    //emit rebuilt(key);
    qDebug() << 13;
    bridgeTimer->start(600000);
    qDebug() << 14;
    queueTimer->start(1000);
    qDebug() << 14.5;
}

void Bridge::applyScheduleHierarchy()
{
    QJsonArray scheduleTables = currentRequest_["scheduleTables"].toJsonArray();
    QJsonArray schedulePrimaryKeysJson = currentRequest_["schedulePrimaryKeys"].toJsonArray();
    QStringList schedulePrimaryKeys;
    for(auto jVal:schedulePrimaryKeysJson)
        schedulePrimaryKeys.append(jVal.toString());

    if(scheduleTables.size() > 1)
    {
        for(int i = 0; i < scheduleTables.size()-1; ++i)
        {
            qDebug() << "Enforcing table sanity.";
            qDebug() << "Primary table" << scheduleTables[i].toObject()["tableName"].toString();
            qDebug() << "Secondary table" << scheduleTables[i+1].toObject()["tableName"].toString();
            bridgeDB_->enforceTableSanity(schedulePrimaryKeys,
                                          scheduleTables[i].toObject()["tableName"].toString(),
                                          scheduleTables[i+1].toObject()["tableName"].toString());
        }
    }
}

void Bridge::generateArgs()
{
    argList_.clear();
    for(auto jVal:currentRequest_["scheduleTables"].toJsonArray())
    {
        QJsonObject tableObj = jVal.toObject();

        QVariantMap args;
        args["key"]         = currentRequest_["key"].toString();
        args["minRouteKey"] = tableObj["minRouteKey"].toString();
        args["maxRouteKey"] = tableObj["maxRouteKey"].toString();
        args["tableName"]   = tableObj["tableName"].toString();
        args["organization:key"] = currentRequest_["organization:key"].toString();
        args["date"] = currentRequest_["date"].toDate();
        argList_.append(args);
    }
}


