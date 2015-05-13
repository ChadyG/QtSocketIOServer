#include "WSListener.h"

WSListener::WSListener(QWebSocketServer * server)
{
    _server = server;
    connect(_server, SIGNAL(newConnection()), this, SLOT(newConnectionHandler()));
    connect(_server, SIGNAL(closed()), this, SLOT(closedHandler()));
}

WSListener::~WSListener()
{
}

void WSListener::newConnectionHandler()
{
    qDebug()<< "new Connection";
    QWebSocket* ws = _server->nextPendingConnection();
    connect(ws, SIGNAL(bytesWritten(qint64)), this, SLOT(bytesWrittenHandler(qint64)));
    connect(ws, SIGNAL(connected()), this, SLOT(connectedHandler()));
    connect(ws, SIGNAL(disconnected()), this, SLOT(disconnectedHandler()));
    connect(ws, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(errorHandler(QAbstractSocket::SocketError)));
    connect(ws, SIGNAL(pong(quint64,QByteArray)), this, SLOT(pongHandler(quint64,QByteArray)));
    connect(ws, SIGNAL(textFrameReceived(QString,bool)), this, SLOT(textFrameRecievedHandler(QString,bool)));
    connect(ws, SIGNAL(textMessageReceived(QString)), this, SLOT(textMessageRecievedHandler(QString)));
    _sockets.append(ws);
}

void WSListener::closedHandler()
{
    qDebug()<< "server closed";
}


void WSListener::bytesWrittenHandler(qint64 bytes)
{
    qDebug()<< "bytes written: " << bytes;
}

void WSListener::connectedHandler()
{
    qDebug()<< "ws connected";
}

void WSListener::disconnectedHandler()
{
    qDebug()<< "ws disconnected";
}

void WSListener::errorHandler(QAbstractSocket::SocketError error)
{
    qDebug()<< "ws error: " << error;
}

void WSListener::pongHandler(quint64 elapsedTime, const QByteArray& payload)
{
    qDebug()<< "ws pong: " << elapsedTime << " payload: " << payload;
}

void WSListener::textFrameRecievedHandler(const QString& frame, bool isLastFrame)
{
    qDebug()<< "ws recieved frame: " << frame;
}

void WSListener::textMessageRecievedHandler(const QString& message)
{
    qDebug()<< "ws recieved message: " << message;

    if (message == "test:event")
    {
        foreach (QWebSocket* ws, _sockets)
        {
            ws->sendTextMessage("test:response");
        }
    }
}
