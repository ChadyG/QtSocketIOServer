#include <QCoreApplication>
#include "SocketIOServer.h"
#include "ExHandler.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    SocketIOServer server("SocketIO");
    ExHandler handler;
    server.registerMessage(&handler);
    server.start();

    return a.exec();
}
