#ifndef SERVER_H
#define SERVER_H

#include <QtCore>
#include <QtNetwork>
#include <QTimer>
#include <QThread>

#include "QIOServer.h"

#include "SocketHandler.h"

/**
 *  Basic Socket.IO Server
 *
 *  Handles incoming connections and routes incoming messages to SocketHandlers.
 *  Parses socket.io messages to convert to internal format
 *
 */

class SocketIOServer : public QObject
{
	Q_OBJECT

public:
    SocketIOServer(QString name = "SocketIO", int port = 3000);
    ~SocketIOServer();

    void registerMessage( SocketHandler* handler);
    void Start();

public slots:
    void processNewConnection();
    void processMessage( QString message );
    void socketDisconnected();
    void sendMessage( QString message );
    //void sendHeartbeats();

signals:
    void clientConnectedEvent();

private:
    QTimer* _timer;
    QIOServer* _server;
    QList<QWsSocket*> _clients;
    QList<SocketHandler*> _handlers;
    QThread _backgroundThread;
};


#endif // SERVER_H
