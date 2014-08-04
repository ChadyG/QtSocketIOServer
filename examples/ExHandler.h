#ifndef HANDLERTEST_H
#define HANDLERTEST_H

#include "SocketHandler.h"

class ExHandler : public SocketHandler
{
    Q_OBJECT

public:
    ExHandler();

    virtual void messageReceived(QString message);
    virtual void messageReceived(QJsonValue  message);
    virtual void eventReceived(QString event);
    virtual void eventReceived(QString event, QString message);
    virtual void eventReceived(QString event, QJsonValue  message);

public slots:
    void ClientConnectedHandler();

private:
    void sendDBValue(QString nameSpace, QString doc, QString key, QVariant val);
    void sendValue(QString service, QString key, QVariant val);

    QMutex _MutexDataUpdate;
    QMap<QString, QVariant> _dataUpdates;
    QString _tcpGetKey;
    QTimer *_timer;
};

#endif // HANDLERTEST_H
