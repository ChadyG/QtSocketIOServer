#ifndef SOCKETMESSAGEHELPER_H
#define SOCKETMESSAGEHELPER_H

#include <QtCore>
#include <QString>

class SocketMessageHelper
{
public:
    static QString Event(QString eventName);
    static QString Event(QString eventName, QString message);
    static QString ValueUpdate(QString key, QString value);
    static QString ValueUpdate(QString key, QVariant value);
    static QString LampConfig(quint16 id, QString name, QString type, quint16 instance, QString ip, QString partNum);

    static QString EscapeValue(QString value);
};

#endif // SOCKETMESSAGEHELPER_H
