#ifndef HANDLERTEST_H
#define HANDLERTEST_H

#include "SocketHandler.h"

class ExHandler : public SocketHandler
{
    Q_OBJECT

public:
    ExHandler();

    virtual void messageReceived(QString socketUuid, QString message);
    virtual void messageReceived(QString socketUuid, QVariant  message);
    virtual void eventReceived(QString socketUuid, QString event);
    virtual void eventReceived(QString socketUuid, QString event, QString message);
    virtual void eventReceived(QString socketUuid, QString event, QVariant message);

public slots:
    virtual void ClientConnectedHandler(QString socketUuid);

private:
    QString sendMessageEvent(QString name, QString message);

    QMutex _MutexDataUpdate;
    QMap<QString, QVariant> _dataUpdates;
    QString _tcpGetKey;
    QTimer *_timer;

    QList<QVariant> _chatLog;
};

#endif // HANDLERTEST_H
