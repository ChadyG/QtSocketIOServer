#include "SocketMessageHelper.h"

using namespace MarkAndy::IntelligentPlatform::Web;

QString SocketMessageHelper::Event(QString eventName)
{
    return QString("5:::{\"name\":\"" + eventName + "\"}");
}

QString SocketMessageHelper::Event(QString eventName, QString message)
{
    return QString("5:::{\"name\":\"" + eventName + "\", \"args\":\"" + EscapeValue(message) + "\"}");
}

QString SocketMessageHelper::ValueUpdate(QString key, QString value)
{
    return QString("5:::{\"name\":\"" + key + "\",\"args\":{\"value\":\"" + EscapeValue(value) + "\"}}");
}

QString SocketMessageHelper::ValueUpdate(QString key, QVariant value)
{
    QString sval = "";
    if (value.canConvert<QString>())
        sval = EscapeValue(value.toString());
    if (value.type() == QMetaType::Double)
        sval = QString("%1").arg(value.toDouble());
    if (value.type() == QMetaType::Int)
        sval = QString::number(value.toInt());
    if (value.type() == QMetaType::Bool)
        sval = value.toBool() ? "true" : "false";
    if (value.type() == QMetaType::QVariantMap || value.type() == QMetaType::QVariantList)
        sval = QString(QJsonDocument::fromVariant(value).toJson()); //TODO: add print options when available
    return QString("5:::{\"name\":\"" + key + "\",\"args\":{\"value\":" + sval + "}}");
}

QString SocketMessageHelper::LampConfig(quint16 id, QString name, QString type, quint16 instance, QString ip, QString partNum)
{
    return QString("5:::{\"name\":\"LampConfig\",\"args\":{\"index\":%1, \"name\":\"%2\", \"type\":\"%3\", \"ip\":\"%4\", \"instance\":%5, \"partnum\":\"%6\" }}")
            .arg(QString::number(id))
            .arg(name)
            .arg(type)
            .arg(ip)
            .arg(QString::number(instance))
            .arg(partNum);
}

QString SocketMessageHelper::EscapeValue(QString value)
{
    return value.replace("\"", "\\\"");
}
