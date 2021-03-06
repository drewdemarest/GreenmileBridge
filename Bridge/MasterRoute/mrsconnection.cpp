#include "mrsconnection.h"

MRSConnection::MRSConnection(const QString &databaseName, QObject *parent) : QObject(parent)
{
    dbPath_ = qApp->applicationDirPath() + "/" + databaseName;
    googleSheets_ = new GoogleSheetsConnection(databaseName, this);
    jsonSettings_ = settings_->loadSettings(QFile(dbPath_), jsonSettings_);
    connect(googleSheets_, &GoogleSheetsConnection::data, this, &MRSConnection::handleNetworkReply);
    connect(googleSheets_, &GoogleSheetsConnection::failed, this, &MRSConnection::failed);
    connect(googleSheets_, &GoogleSheetsConnection::statusMessage, this, &MRSConnection::statusMessage);
    connect(googleSheets_, &GoogleSheetsConnection::debugMessage, this, &MRSConnection::debugMessage);
    connect(googleSheets_, &GoogleSheetsConnection::errorMessage, this, &MRSConnection::errorMessage);
}

MRSConnection::~MRSConnection()
{

}

void MRSConnection::requestAssignments(const QString &key, const QDate &date)
{
    googleSheets_->requestValuesFromAGoogleSheet(key, date.toString("dddd"));
}

void MRSConnection::requestAssignments(const QString &key, const QString &sheetName)
{
    googleSheets_->requestValuesFromAGoogleSheet(key, sheetName);
}

void MRSConnection::handleNetworkReply(const QString &key, const QJsonObject &data)
{
    if(data.isEmpty())
    {
        emit errorMessage("Empty result set for " + key + ". Check network connections.");
        emit failed(key, "Empty result set for " + key + ". Check network connections.");
    }
    else
    {
        emit statusMessage("Google sheets retrieved for " + key + ".");
        data["organization:key"] = QJsonValue(key.split(":").first());
        emit mrsDailyScheduleSQL(key, mrsDailyScheduleJsonToSQL(data));
    }
}

QMap<QString, QVariantList> MRSConnection::mrsDailyScheduleJsonToSQL(const QJsonObject &data)
{
    jsonSettings_ = settings_->loadSettings(QFile(dbPath_), jsonSettings_);
    QMap<QString, QVariantList> sqlData;
    bool foundDate = false;
    QDate date;
    QString routeKey;
    QString dateFormat  = jsonSettings_["date_format"].toString(); //"d-MMM-yyyy" by default;
    QString orgKey      = jsonSettings_["organization_key"].toString();
    int driverOffset    = jsonSettings_["driver_offset"].toInt();
    int truckOffset     = jsonSettings_["truck_offset"].toInt();
    int trailerOffset   = jsonSettings_["trailer_offset"].toInt();
    //int routeKeyLength  = 5;
    QString routeRegex  = jsonSettings_["route_regex"].toString();

    QJsonArray rows = data["values"].toArray();

    for(auto rowArr:rows)
    {
        if(foundDate)
            break;

        QJsonArray row = rowArr.toArray();
        for(int i = 0; i<row.size(); ++i)
        {
            QString value = row[i].toString();
            QDate dateTest = QDate::fromString(value, dateFormat);
            if(dateTest.isValid() && !dateTest.isNull())
            {
                foundDate = true;
                date = dateTest;
                break;
            }
        }
    }

    for(auto rowArr:rows)
    {
        QJsonArray row = rowArr.toArray();
        for(int i = 0; i<row.size(); ++i)
        {
            QString value = row[i].toString();
            QRegularExpression routeRegExp(routeRegex);
            qDebug() << routeRegex;
            QRegularExpressionMatch match = routeRegExp.match(value);
            if(match.hasMatch()/* && (value.size() == routeKeyLength)*/)
            {
                qDebug() << "Schedule route key captured" << routeKey << match << match.capturedTexts();
                routeKey = match.captured(0);
                sqlData["route:key"].append(QVariant(routeKey));
                sqlData["route:date"].append(QVariant(date));
                sqlData["organization:key"].append(QVariant(orgKey));

                if(i+driverOffset < row.size()
                        && !row[i+driverOffset].toString().simplified().isEmpty()
                        && driverOffset != 0)
                    sqlData["driver:name"].append(QVariant(row[i+driverOffset].toString().simplified()));
                else
                    sqlData["driver:name"].append(QVariant());

                if(i+truckOffset < row.size()
                        && !row[i+truckOffset].toString().simplified().isEmpty()
                        && truckOffset != 0)
                    sqlData["truck:key"].append(QVariant(row[i+truckOffset].toString().simplified()));
                else
                    sqlData["truck:key"].append(QVariant());

                if(i+trailerOffset < row.size()
                        && !row[i+trailerOffset].toString().simplified().isEmpty()
                        && trailerOffset != 0)

                    sqlData["trailer:key"].append(QVariant(row[i+trailerOffset].toString().simplified()));
                else
                    sqlData["trailer:key"].append(QVariant());
            }
        }
    }
    return sqlData;
}
