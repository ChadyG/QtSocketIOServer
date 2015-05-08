#include <QDebug>
#include "QtJson/json.h"

#include "SocketIOServer.h"


SocketIOServer::SocketIOServer(QString name, int port)
{
    _server = new QIOServer(0);
    _server->moveToThread(&_backgroundThread);

    connect(_server, SIGNAL(newConnection()), this, SLOT(processNewConnection()));
    connect(_server, SIGNAL(socketDisconnected()), this, SLOT(socketDisconnected()));
    connect(_server, SIGNAL(newMessage(QString)), this, SLOT(processMessage(QString)));

    _backgroundThread.start();

    QMetaObject::invokeMethod(_server, "listen", Q_ARG(quint16, port));
    QMetaObject::invokeMethod(_server, "start");

}

SocketIOServer::~SocketIOServer()
{
    if (_server != 0)
    {
        QMetaObject::invokeMethod(_server, "stop");
        _server->deleteLater();
    }
    _server = 0;
}

void SocketIOServer::registerMessage( SocketHandler* handler)
{
    _handlers << handler;
    connect( handler, SIGNAL(sendMessage(QString)), this, SLOT(sendMessage(QString)));
    connect(this, SIGNAL(clientConnectedEvent()), handler, SLOT(ClientConnectedHandler()));
}

void SocketIOServer::processNewConnection()
{
    emit clientConnectedEvent();

    qDebug() << tr("Client connected");
}

void SocketIOServer::processMessage( QString frame )
{
    qDebug() << "SocketIOServer:processMessage: " <<  frame;

    //Do basic parsing of socket.io messages here
    QString eventName;
    QVariantMap json;
    QVariantList args;
    QStringList socketargs = frame.split(":");
    EIOsocketMessages mtype = (EIOsocketMessages)socketargs[0].toInt();
    QString messageID;
    QString messageEndPoint;
    QString messageData;
    if (socketargs.size() >= 2)
        messageID = socketargs[1];
    if (socketargs.size() >= 3)
        messageEndPoint = socketargs[2];
    if (socketargs.size() >= 4)
        messageData = socketargs[3];

    // rest of socketargs are all part of the message
    for (int i = 4; i < socketargs.size(); i++)
    {
        messageData += ":" + socketargs[i];
    }
    switch (mtype)
    {
    case IO_MDisconnect:
        break;
    case IO_MConnect:
        break;
    case IO_MHeartbeat:
        //qDebug() << "SocketIOServer:processMessage:Heartbeat " <<  frame;
        break;
    case IO_MMessage:
        //already have messageData
        foreach (SocketHandler* handler, _handlers)
        {
            handler->messageReceived(messageData);
        }
        break;
    case IO_MJSON:
        // JSON object {"name" : "EVENT", "args" : [OBJ]}
        json = QtJson::Json::parse(messageData.toUtf8()).toMap();
        eventName = json["name"].toString();
        //messageData from args

        foreach (SocketHandler* handler, _handlers)
        {
            handler->messageReceived(json["args"]);
        }
        break;
    case IO_MEvent:
        // JSON object {"name" : "EVENT", "args" : [STRING]}
        json = QtJson::Json::parse(messageData.toUtf8()).toMap();
        eventName = json["name"].toString();
        if (json.contains("args"))
        {
            args = json["args"].toList();
            for (int i = 0; i < args.size(); i++)
            {
                if (args[i].type() == QVariant::Map)
                {
                    //qDebug() << "message is JSON";
                    //messageData = QJsonDocument::fromVariant(args[0]).toJson();
                    foreach (SocketHandler* handler, _handlers)
                    {
                        handler->eventReceived(eventName, args[i]);
                    }
                }
                else if (args[i].type() == QVariant::String)
                {
                    //qDebug() << "message is string";
                    //messageData = args[0].toString();foreach (SocketHandler* handler, _handlers)
                    foreach (SocketHandler* handler, _handlers)
                    {
                        handler->eventReceived(eventName, args[i].toString());
                    }
                }
            }
        }
        break;
    case IO_MACK:
        break;
    case IO_MError:
        break;
    case IO_MNoop:
        break;
    }
}

void SocketIOServer::sendMessage( QString message )
{
    if (!message.contains("isServerConnected"))
        qDebug() <<  tr("sendMessage: %1").arg(message);

     QMetaObject::invokeMethod(_server, "sendMessage", Q_ARG(const QString &, message));
}

//void SocketIOServer::sendHeartbeats()
//{
//    qDebug() << "SocketIOServer:sendHeartbeats ";
//    QWsSocket * client;
//    foreach ( client, _clients )
//    {
//        QMetaObject::invokeMethod(client, "write", Q_ARG(const QString &, QString("2::")));
//        //client->write( QString("2::") );
//    }
//}

void SocketIOServer::socketDisconnected()
{
    qDebug() << "SocketIOServer::socketDisconnected()";

    qDebug() << tr("Client disconnected");
}
