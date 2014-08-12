#ifndef WSLISTENER_H
#define WSLISTENER_H
#include <QWebSocketServer>
#include <QWebSocket>
#include <QList>
#include <QDebug>

class WSListener : QObject
{
    Q_OBJECT
public:
    WSListener(QWebSocketServer * server);

    virtual ~WSListener();
public slots:
    void newConnectionHandler();
    void closedHandler();

    void bytesWrittenHandler(qint64 bytes);
    void connectedHandler();
    void disconnectedHandler();
    void errorHandler(QAbstractSocket::SocketError error);
    void pongHandler(quint64 elapsedTime, const QByteArray& payload);
    void textFrameRecievedHandler(const QString& frame, bool isLastFrame);
    void textMessageRecievedHandler(const QString& message);

private:
    QWebSocketServer* _server;
    QList<QWebSocket*> _sockets;
};


#endif // WSLISTENER_H
