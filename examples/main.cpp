#include <QCoreApplication>
#include <QWebSocketServer>
#include "WSListener.h"
//#include "SocketIOServer.h"
//#include "ExHandler.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    //SocketIOServer server("SocketIO");
    QWebSocketServer server("SocketIO", QWebSocketServer::NonSecureMode, 0);
    WSListener listen(&server);
    server.listen(QHostAddress::Any, 3000);
    //ExHandler handler;
    //server.registerMessage(&handler);

    return a.exec();
}


