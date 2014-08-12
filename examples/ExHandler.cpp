#include "ExHandler.h"

#include "SocketMessageHelper.h"

ExHandler::ExHandler()
{
//    _timer = new QTimer(this);
//    connect(_timer, SIGNAL(timeout()), this, SLOT(ProcessDataChanges()));
//    _timer->start(500);// This update period seems to work the best across desktop/iPad/nexus 10  in chrome+FF+safari

}

void ExHandler::messageReceived(QString message)
{
    qDebug() << "eventReceived1: " << message;

}

void ExHandler::messageReceived(QJsonValue  message)
{
    qDebug() << "eventReceived2: " << message;

}

void ExHandler::eventReceived(QString event)
{
    qDebug() << "eventReceived3: " << event;

}

void ExHandler::eventReceived(QString event, QString message)
{
    qDebug() << "eventReceived4: " << event << " : " << message;
    //TODO
    //GetOPCServerdata
    //GetInterLockdata
    //GetPressConfigdata
    //SaveProLEDConfig
    //"SaveProLEDConfig"  :  QJsonValue(object, QJsonObject({"data": {"configuration": {"commMethod": "serial","pressCommMethod": "usb"},"lampPosition": [{"position": "lower"},{"position": "lower"},{"position": "lower"},{"position": "lower"},{"position": "lower"},{"position": "lower"},{"position": "lower"}],"lampSerialNumbers": [{"serialNumber": "1231"},{"serialNumber": "1232"},{"serialNumber": "1233"},{"serialNumber": "1234"},{"serialNumber": "1235"},{"serialNumber": "1236"},{"serialNumber": "1237"}],"stationNumbers": [{"stationNumber": "1"},{"stationNumber": "2"},{"stationNumber": "3"},{"stationNumber": "4"},{"stationNumber": "5"},{"stationNumber": "6"},{"stationNumber": "7"}]},"id": "proLEDConfig"}) )

    if (event == "testState")
    {
        qDebug() << "testState" << message;

//        if (message == "Initialize")
//            _state = Enumerations::Initialize;
//        if (message == "Update")
//            _state = Enumerations::ApplyUpdates;
//        if (message == "Shutdown")
//            _state = Enumerations::Shutdown;
//        if (message == "Run")
//            _state = Enumerations::Run;

//        _cacheClient->SetValue(DataTags::ProLEDMaster::State, (int)_state, "");
//        sendMessage(SocketMessageHelper::ValueUpdate("proLEDMaster:state", _state));
    }
}

void ExHandler::eventReceived(QString event, QJsonValue message)
{
    if (event != "isServerConnected")
        qDebug() << "SocketIO::eventReceived: " << event << " : " << message;
    QStringList parts;
    QString doc;
    QString field;
    QString workspace;

    QVariant value;
    QVariant vreturn;

    QJsonValue jvalue;

    if (event == "getValue")
    {
        qDebug() << "getValue" << message;
    }

    if (event == "setValue")
    {
    }

    if (event == "isServerConnected")
    {
        value = message.toObject()["value"].toString();
        //sendMessage(SocketMessageHelper::ValueUpdate("isServerConnected", true));
    }
}


// SocketIO Client
void ExHandler::ClientConnectedHandler()
{
    // alarm ping
    //sendMessage(SocketMessageHelper::ValueUpdate("isServerConnected", true));
}

void ExHandler::sendDBValue(QString nameSpace, QString doc, QString key, QVariant val)
{
    QString path = doc;
    if (key != "")
        path = doc + "." + key;

    if (val.type() == QMetaType::Bool)
    {
        //sendMessage(SocketMessageHelper::ValueUpdate(QString("%1:%2").arg(nameSpace).arg(path), val));
        return;
    }
    if (val.type() == QMetaType::QString)
    {
        //sendMessage(SocketMessageHelper::ValueUpdate(nameSpace + ":" + path, val.toString()));
        return;
    }
    if (val.type() == QMetaType::Int)
    {
        //sendMessage(SocketMessageHelper::ValueUpdate(nameSpace + ":" + path, QString::number(val.toInt())));
        return;
    }
    if (val.type() == QMetaType::Double)
    {
        //sendMessage(SocketMessageHelper::ValueUpdate(nameSpace + ":" + path, QString::number((int)val.toDouble())));
        return;
    }
    if (val.type() == QMetaType::QVariantMap)
    {
        //sendMessage(SocketMessageHelper::ValueUpdate(nameSpace + ":" + path, val));
        return;
    }
}

void ExHandler::sendValue(QString service, QString key, QVariant val)
{
    // localservice is master for this file
    if (val.type() == QMetaType::Bool)
    {
        //sendMessage(SocketMessageHelper::ValueUpdate(QString("%1:%2").arg(service).arg(key), val));
        return;
    }
    if (val.type() == QMetaType::QString)
    {
        //sendMessage(SocketMessageHelper::ValueUpdate(QString("%1:%2").arg(service).arg(key), val.toString()));
        return;
    }
    if (val.type() == QMetaType::Int)
    {
        //sendMessage(SocketMessageHelper::ValueUpdate(QString("%1:%2").arg(service).arg(key), QString::number(val.toInt())));
        return;
    }
    if (val.type() == QMetaType::Double)
    {
        //sendMessage(SocketMessageHelper::ValueUpdate(QString("%1:%2").arg(service).arg(key), QString::number((int)val.toDouble())));
        return;
    }
}
