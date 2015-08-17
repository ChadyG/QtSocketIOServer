#include "ExHandler.h"

ExHandler::ExHandler()
{
//    _timer = new QTimer(this);
//    connect(_timer, SIGNAL(timeout()), this, SLOT(ProcessDataChanges()));
//    _timer->start(500);// This update period seems to work the best across desktop/iPad/nexus 10  in chrome+FF+safari

}

void ExHandler::messageReceived(QString socketUuid, QString message)
{
    qDebug() << "eventReceived1: " << message;

}

void ExHandler::messageReceived(QString socketUuid, QVariant  message)
{
    qDebug() << "eventReceived2: " << message;

}

void ExHandler::eventReceived(QString socketUuid, QString event)
{
    qDebug() << "eventReceived3: " << event;

}

void ExHandler::eventReceived(QString socketUuid, QString event, QString message)
{
    qDebug() << "eventReceived4: " << event << " : " << message;
    if (event == "sendMessage")
    {
        qDebug() << "sendMessage" << message;

    }
}

void ExHandler::eventReceived(QString socketUuid, QString event, QVariant message)
{
    if (event != "isServerConnected")
        qDebug() << "SocketIO::eventReceived: " << event << " : " << message;

    if (event == "sendMessage")
    {
        _chatLog.append(message);
        emit sendMessage(sendMessageEvent(message.toMap()["name"].toString(), message.toMap()["message"].toString()));
    }
}


// SocketIO Client
void ExHandler::ClientConnectedHandler(QString socketUuid)
{
    // alarm ping
    //sendMessage(SocketMessageHelper::ValueUpdate("isServerConnected", true));
    foreach (QVariant chat, _chatLog)
    {
        emit sendMessage(socketUuid, sendMessageEvent(chat.toMap()["name"].toString(), chat.toMap()["message"].toString()));
    }
}


QString ExHandler::sendMessageEvent(QString name, QString message)
{
    return QString("5:::{\"name\":\"newMessage\",\"args\":{\"name\":\"%1\",\"message\":\"%2\"}}").arg(name).arg(message);
}
