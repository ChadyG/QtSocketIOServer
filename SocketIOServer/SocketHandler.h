#ifndef SOCKETHANDLER_H
#define SOCKETHANDLER_H

#include <QtCore>

/**
 *  SocketHandler
 *
 *  Abstract interface for custom handling of websockets.
 *  Custom implementations are registered with the server alongside an event namespace.
 *  All events recieved with that namespace (X:) will be sent to the custom handler.
 *  The sendMessage slot allows simple message sending capability to communicate back.
 *
 */


class SocketHandler : public QObject
{
	Q_OBJECT

public:
    SocketHandler();
    ~SocketHandler();

    virtual void messageReceived(QString message) = 0;
    virtual void messageReceived(QJsonValue message) = 0;
    virtual void eventReceived(QString event) = 0;
    virtual void eventReceived(QString event, QString message) = 0;
    virtual void eventReceived(QString event, QJsonValue  message) = 0;
signals:
    void sendMessage(QString message);

public slots:
    virtual void ClientConnectedHandler() = 0;
};


#endif // SOCKETHANDLER_H
