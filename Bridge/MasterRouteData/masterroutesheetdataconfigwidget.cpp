#include "masterroutesheetdataconfigwidget.h"
#include "ui_masterroutesheetdataconfigwidget.h"

MasterRouteSheetDataConfigWidget::MasterRouteSheetDataConfigWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MasterRouteSheetDataConfigWidget)
{
    ui->setupUi(this);

    connect(ui->saveSettingsPushButton, &QPushButton::pressed,
            this, &MasterRouteSheetDataConfigWidget::saveUItoSettings);

    connect(ui->loadFromFilePushButton,
            &QPushButton::pressed,
            this,
            &MasterRouteSheetDataConfigWidget::loadSettingsFromFile);

    connect(ui->addRedirectURIPushButton,
            &QPushButton::pressed,
            this,
            &MasterRouteSheetDataConfigWidget::addRedirectURI);

    connect(ui->removeSelectedRedirectURIPushButton,
            &QPushButton::pressed,
            this,
            &MasterRouteSheetDataConfigWidget::removeRedirectURI);

    applySettingsToUI();
}

MasterRouteSheetDataConfigWidget::~MasterRouteSheetDataConfigWidget()
{
    delete ui;
}

void MasterRouteSheetDataConfigWidget::saveUItoSettings()
{
    QJsonArray redirectURIs;
    jsonSettings_["auth_uri"] = QJsonValue(ui->authURILineEdit->text());
    jsonSettings_["client_id"] = QJsonValue(ui->clientIDLineEdit->text());
    jsonSettings_["client_secret"] = QJsonValue(ui->clientSecretLineEdit->text());
    jsonSettings_["base_url"] = QJsonValue(ui->googleSheetsBaseAddrLineEdit->text());
    jsonSettings_["api_scope"] = QJsonValue(ui->googleSheetsScopeLineEdit->text());
    jsonSettings_["project_id"] = QJsonValue(ui->projectIDLineEdit->text());
    jsonSettings_["auth_provider_x509_cert_url"] = QJsonValue(ui->x509LineEdit->text());
    jsonSettings_["request_timeout"] = QJsonValue(ui->requestTimeoutSpinBox->value());
    jsonSettings_["oauth2_user_timeout"] = QJsonValue(ui->oauth2UserTimeoutSpinBox->value());

    for(int i = 0; i < ui->redirectURIListWidget->count(); ++i)
        redirectURIs.append(ui->redirectURIListWidget->item(i)->text());

    jsonSettings_["redirect_uris"] = redirectURIs;

    settings_->saveSettings(QFile(dbPath_), jsonSettings_);
}

void MasterRouteSheetDataConfigWidget::loadSettingsFromFile()
{
    QString jsonFilePath = QFileDialog::getOpenFileName
                            (this,
                            tr("Select Google Json File"),
                            qApp->applicationDirPath(),
                            "JSON file (*.json)");

    QJsonObject jsonCredentials = makeJsonFromFile(jsonFilePath);
    jsonCredentials = jsonCredentials["web"].toObject();

    if(noSettingsNullOrUndefined(jsonCredentials))
    {
        qDebug() << "saving settings" << jsonCredentials;
        settings_->saveSettings(QFile(dbPath_), jsonCredentials);
    }

    applySettingsToUI();
}

void MasterRouteSheetDataConfigWidget::addRedirectURI()
{
    bool ok;
    QString redirectURI = QInputDialog::getText(this,
                                                 tr("Insert Redirect URI"),
                                                 tr("Redirect URI:"), QLineEdit::Normal,
                                                 "URI:Port", &ok);
    if(ok)
        ui->redirectURIListWidget->addItem(redirectURI);
}

void MasterRouteSheetDataConfigWidget::removeRedirectURI()
{
    ui->redirectURIListWidget->takeItem(ui->redirectURIListWidget->currentRow());
}

bool MasterRouteSheetDataConfigWidget::noSettingsNullOrUndefined(const QJsonObject &settings)
{
    QSet<QString> validKeys;
    for(auto key: settings.keys())
        if(!settings[key].isNull() && !settings[key].isUndefined())
            validKeys.insert(key);

    if(validKeys.size() == settings.keys().size())
        return true;

    else
        return false;
}

void MasterRouteSheetDataConfigWidget::applySettingsToUI()
{
    jsonSettings_ = settings_->loadSettings(QFile(dbPath_), jsonSettings_);
    ui->clientIDLineEdit->setText(jsonSettings_["client_id"].toString());
    ui->clientSecretLineEdit->setText(jsonSettings_["client_secret"].toString());
    ui->googleSheetsBaseAddrLineEdit->setText(jsonSettings_["base_url"].toString());
    ui->googleSheetsScopeLineEdit->setText(jsonSettings_["api_scope"].toString());
    ui->projectIDLineEdit->setText(jsonSettings_["project_id"].toString());
    ui->authURILineEdit->setText(jsonSettings_["auth_uri"].toString());
    ui->x509LineEdit->setText(jsonSettings_["auth_provider_x509_cert_url"].toString());
    ui->requestTimeoutSpinBox->setValue(jsonSettings_["request_timeout"].toInt());
    ui->oauth2UserTimeoutSpinBox->setValue(jsonSettings_["oauth2_user_timeout"].toInt());

    ui->redirectURIListWidget->clear();
    for(auto json:jsonSettings_["redirect_uris"].toArray())
        ui->redirectURIListWidget->addItem(json.toString());
}

QJsonObject MasterRouteSheetDataConfigWidget::makeJsonFromFile(const QString &jsonCredentialPath)
{
    QByteArray credentials;
    QFile jsonCredentialFile(jsonCredentialPath);

    if(!jsonCredentialFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Failed to open json credential file in\
                    MasterRouteSheetDataConfigWidget::setOAuth2CredWithJson";
                    return QJsonObject();
    }
    while(!jsonCredentialFile.atEnd())
    {
        credentials.append(jsonCredentialFile.readLine());
    }

    jsonCredentialFile.close();
    return QJsonObject(QJsonDocument::fromJson(credentials).object());
}

void MasterRouteSheetDataConfigWidget::loadSettings()
{
    jsonSettings_ = settings_->loadSettings(QFile(dbPath_), jsonSettings_);
}

void MasterRouteSheetDataConfigWidget::saveSettings()
{
    settings_->saveSettings(QFile(dbPath_), jsonSettings_);
}

