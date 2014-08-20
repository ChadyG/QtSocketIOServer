#include "QIOServer.h"

#include <QRegExp>
#include <QStringList>
#include <QByteArray>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>

const QString QIOServer::regExpResourceNameStr( QLatin1String("^GET\\s(.*)\\sHTTP/1.1\r\n") );
const QString QIOServer::regExpHostStr( QLatin1String("\r\nHost:\\s(.+(:\\d+)?)\r\n") );

const QString QIOServer::regExpCmdParamStr( QLatin1String("EIO=(\\d+)&?") );
const QString QIOServer::regExpTransportParamStr( QLatin1String("transport=(\\w+)&?") );
const QString QIOServer::regExpSIDParamStr( QLatin1String("sid=(\\w+)&?") );
const QString QIOServer::regExpB64ParamStr( QLatin1String("b64=(\\d+)&?") );

const QString QIOServer::regExpKeyStr( QLatin1String("\r\nSec-WebSocket-Key:\\s(.{24})\r\n") );
const QString QIOServer::regExpKey1Str( QLatin1String("\r\nSec-WebSocket-Key1:\\s(.+)\r\n") );
const QString QIOServer::regExpKey2Str( QLatin1String("\r\nSec-WebSocket-Key2:\\s(.+)\r\n") );
const QString QIOServer::regExpKey3Str( QLatin1String("\r\n(.{8})$") );
const QString QIOServer::regExpVersionStr( QLatin1String("\r\nSec-WebSocket-Version:\\s(\\d+)\r\n") );
const QString QIOServer::regExpOriginStr( QLatin1String("\r\nSec-WebSocket-Origin:\\s(.+)\r\n") );
const QString QIOServer::regExpOrigin2Str( QLatin1String("\r\nOrigin:\\s(.+)\r\n") );
const QString QIOServer::regExpProtocolStr( QLatin1String("\r\nSec-WebSocket-Protocol:\\s(.+)\r\n") );
const QString QIOServer::regExpExtensionsStr( QLatin1String("\r\nSec-WebSocket-Extensions:\\s(.+)\r\n") );


QIOServer::QIOServer(QObject * parent)
	: QObject(parent)
    , _tcpServer(0)
    , _timer(0)
    , _connected(false)
{
}

QIOServer::~QIOServer()
{
    _tcpServer->deleteLater();
}

void QIOServer::listen(quint16 port, const QHostAddress & address)
{
    if (_tcpServer == 0)
    {
        _tcpServer = new QTcpServer(this);
        connect( _tcpServer, SIGNAL(newConnection()), this, SLOT(newTcpConnection()) );
        qsrand( QDateTime::currentMSecsSinceEpoch() );
    }
    _tcpServer->listen(address, port);
}

void QIOServer::start()
{
    if (_timer == 0)
    {
        _timer = new QTimer(this);
        connect(_timer, SIGNAL(timeout()), this, SLOT(sendHeartbeats()));
        _timer->start(15000);
    }
    //this->moveToThread(&_backgroundThread);

    //_backgroundThread.start();
}

void QIOServer::close()
{
    _tcpServer->close();
   // _backgroundThread.quit();
   // _backgroundThread.deleteLater();
   // _backgroundThread.wait();
}

void QIOServer::sendMessage(const QString &message)
{
    QWsSocket * client;
    foreach ( client, _clients )
    {
        //QMetaObject::invokeMethod(client, "write", Q_ARG(const QString &, message));
        client->write( message );
    }
}

void QIOServer::newTcpConnection()
{
    QTcpSocket * tcpSocket = _tcpServer->nextPendingConnection();
	connect( tcpSocket, SIGNAL(readyRead()), this, SLOT(dataReceived()) );
	headerBuffer.insert( tcpSocket, QStringList() );
}

void QIOServer::closeTcpConnection()
{
	QTcpSocket * tcpSocket = qobject_cast<QTcpSocket*>( sender() );
	if (tcpSocket == 0)
		return;

	tcpSocket->close();
}

void QIOServer::dataReceived()
{
    //qDebug() << "QIOServer:Data Received";
	QTcpSocket * tcpSocket = qobject_cast<QTcpSocket*>( sender() );
	if (tcpSocket == 0)
		return;

	bool allHeadersFetched = false;

	const QLatin1String emptyLine("\r\n");

	while ( tcpSocket->canReadLine() )
	{
		QString line = tcpSocket->readLine();
        //qDebug() << "QIOServer:Header line: " << line;

		if (line == emptyLine)
		{
			allHeadersFetched = true;
			break;
		}

		headerBuffer[ tcpSocket ].append(line);
	}

	if (!allHeadersFetched)
    {
		return;
    }

	QString request( headerBuffer[ tcpSocket ].join("") );

	QRegExp regExp;
	regExp.setMinimal( true );

    // Resource name
    regExp.setPattern( QIOServer::regExpResourceNameStr );
    regExp.indexIn(request);
    QString resourceName = regExp.cap(1);

	// Extract mandatory datas
    // Version
    QStringList socketTemp = resourceName.split('/');
    QString pathSegment;
    QString commandStr;
    QString paramStr;
    QString sessionID;
    pathSegment = socketTemp[1];
    if (socketTemp.size() == 3)
    {
        processEngineIO(tcpSocket, request, resourceName);
        return;
    }
    if (socketTemp.size() > 3)
    {
        commandStr = socketTemp[2];
        paramStr = socketTemp[3];
    }
    if (socketTemp.size() >= 5)
    {
        sessionID = socketTemp[4];
    }
    EIOsocketMessages mtype;
    if ( ! commandStr.isEmpty() )
	{
        mtype = (EIOsocketMessages)commandStr.toInt();
    }
	else
	{
        mtype = IO_MUnknown;
    }

    //qDebug() << "QIOServer: " << resourceName;
    //TODO: we can tell if this is a handshake request or websockets request
    // fork logic here to continue with handshake or do websocket connection
    if (mtype == IO_MConnect && !sessionID.isEmpty())
    {
        processSocketHandshake(tcpSocket, request);
        return;
    }

	// Host (address & port)
	regExp.setPattern( QIOServer::regExpHostStr );
	regExp.indexIn(request);
	QString host = regExp.cap(1);
	QStringList hostTmp = host.split(':');
	QString hostAddress = hostTmp[0];
	QString hostPort;
	if ( hostTmp.size() > 1 )
		hostPort = hostTmp.last(); // fix for IPv6

	
	////////////////////////////////////////////////////////////////////

    // If the mandatory fields are not specified, we abort the connection to the Websocket server
    if ( mtype == IO_MUnknown || resourceName.isEmpty() || hostAddress.isEmpty() )
	{
		// Send bad request response
		QString response = QIOServer::composeBadRequestResponse( QList<EWebsocketVersion>() << WS_V6 << WS_V7 << WS_V8 << WS_V13 );
		tcpSocket->write( response.toUtf8() );
		tcpSocket->flush();
		return;
	}
	
	////////////////////////////////////////////////////////////////////
	
	// Extract optional datas

	// Origin
    regExp.setPattern( QIOServer::regExpOrigin2Str );
    regExp.indexIn(request);
    QString origin = regExp.cap(1);
	
	////////////////////////////////////////////////////////////////////
	
	// Compose opening handshake response
	QString response;
    QString accept;

    switch (mtype)
    {
    case IO_MDisconnect:
        break;
    case IO_MConnect:
        accept = computeSocketIOSession( origin );
        response = QIOServer::composeSocketIOHandshakeResponse( accept, origin, hostAddress, hostPort, resourceName, "" );
        break;
    case IO_MHeartbeat:
        break;
    case IO_MMessage:
        break;
    case IO_MJSON:
        break;
    case IO_MEvent:
        break;
    case IO_MACK:
        break;
    case IO_MError:
        break;
    case IO_MNoop:
        break;
    }

	
	// Handshake OK, disconnect readyRead
    //disconnect( tcpSocket, SIGNAL(readyRead()), this, SLOT(dataReceived()) );

	// Send opening handshake response
    //if ( version == WS_V0 )
        tcpSocket->write( response.toLatin1() );
    //else
    //	tcpSocket->write( response.toUtf8() );
    tcpSocket->flush();
    tcpSocket->close();
    //qDebug() << "QIOServer: handshake sent";

    //TODO: Once handshake is complete, connect websocket server
    //add pass through for websocket newConnection
	
	// ORIGINAL CODE
	//int socketDescriptor = tcpSocket->socketDescriptor();
	//incomingConnection( socketDescriptor );
	
	// CHANGED CODE FOR LINUX COMPATIBILITY
    //addPendingConnection( ioSocket );
    //emit newConnection();
}

void QIOServer::processEngineIO(QTcpSocket *tcpSocket, QString request, QString resourceName)
{
    QRegExp regExp;
    QStringList socketTemp = resourceName.split('/');
    QString pathSegment;
    QString commandStr;
    QString paramStr;
    QString sessionID;
    pathSegment = socketTemp[1];
    paramStr = socketTemp[2];
    regExp.setPattern(QIOServer::regExpCmdParamStr);
    regExp.indexIn(paramStr);
    commandStr = regExp.cap(1);
    // v1.0 spec, check parameters
    // if no sid, this is a new connection
    // b64=1 means encode with base64
    // transport: polling, flashsocket, websocket
    // respond similar to
    // 97:0{"sid":"mYI0xtIJnXu-D2bWADhY","upgrades":["websocket"],"pingInterval":25000,"pingTimeout":60000}
    regExp.setPattern(regExpSIDParamStr);
    if (regExp.indexIn(paramStr) > -1)
    {
        sessionID = regExp.cap(1);
    }
    else
    {
        newEngineIOConnection(tcpSocket, request, paramStr);
    }

    // Origin
    regExp.setMinimal( true );
    regExp.setPattern( QIOServer::regExpOriginStr );
    if ( regExp.indexIn(request) == -1 )
    {
        regExp.setPattern( QIOServer::regExpOrigin2Str );
        regExp.indexIn(request);
    }
    QString origin = regExp.cap(1);
    // find existing connection, respond with 2:40 (message connect) or pong
    for (int i=0; i < _connections.count(); i++)
    {
        if (_connections[i].sID == sessionID)
        {
            if (_connections[i].connected == false)
            {

                tcpSocket->write(composeEngineIOConect(
                                     _connections[i].sID,
                                     origin));
                tcpSocket->close();
                _connections[i].connected = true;
            }
            else
            {
                qDebug() << "QIOServer: pending pong/message";
                // this is a long poll request, we should return with any pending messages or a pong
                //tcpSocket->write(composeEngineIOPong(
                //                     _connections[i].sID,
                //                     origin));
                //tcpSocket->close();
            }
            break;
        }
    }
}

void QIOServer::newEngineIOConnection(QTcpSocket *tcpSocket, QString request, QString paramStr)
{
    QRegExp regExp;
    IOConnection conn;
    conn.socket = tcpSocket;
    conn.connected = false;
    // Generate unique sid
    conn.sID = QCryptographicHash::hash(QDateTime::currentDateTime().toString().toLocal8Bit() + paramStr.toLocal8Bit(), QCryptographicHash::Md5).toHex().left(20).constData();

    regExp.setPattern(regExpTransportParamStr);
    regExp.indexIn(paramStr);
    QString transport = regExp.cap(1);
    if (transport == "polling")
    {
        conn.transport = IO_TPolling;
    }
    else if (transport == "flashsocket")
    {
        conn.transport = IO_TFlashSocket;
    }
    else if (transport == "websocket")
    {
        conn.transport = IO_TWebSocket;
    }
    else
    {
        conn.transport = IO_TUnknown;
    }
    regExp.setPattern(regExpB64ParamStr);
    if (regExp.indexIn(paramStr) > -1)
    {
        conn.b64 = regExp.cap(1) == "1" ? true : false;
    }
    else
    {
        conn.b64 = false;
    }
    _connections.append(conn);

    // Origin
    regExp.setMinimal( true );
    regExp.setPattern( QIOServer::regExpOriginStr );
    if ( regExp.indexIn(request) == -1 )
    {
        regExp.setPattern( QIOServer::regExpOrigin2Str );
        regExp.indexIn(request);
    }
    QString origin = regExp.cap(1);

    // respond with accept string, then return
    tcpSocket->write(composeEngineIOAccept(
                         conn.sID,
                         origin));
    tcpSocket->close();
}

void QIOServer::processSocketHandshake(QTcpSocket *tcpSocket, QString request)
{
    QRegExp regExp;
    regExp.setMinimal( true );

    // Resource name
    regExp.setPattern( QIOServer::regExpResourceNameStr );
    regExp.indexIn(request);
    QString resourceName = regExp.cap(1);

    // Host (address & port)
    regExp.setPattern( QIOServer::regExpHostStr );
    regExp.indexIn(request);
    QString host = regExp.cap(1);
    QStringList hostTmp = host.split(':');
    QString hostAddress = hostTmp[0];
    QString hostPort;
    if ( hostTmp.size() > 1 )
        hostPort = hostTmp.last(); // fix for IPv6

    // Extract mandatory datas
    // Version
    regExp.setPattern( QIOServer::regExpVersionStr );
    regExp.indexIn(request);
    QString versionStr = regExp.cap(1);
    EWebsocketVersion version;
    if ( ! versionStr.isEmpty() )
    {
        version = (EWebsocketVersion)versionStr.toInt();
    }
    else if ( tcpSocket->bytesAvailable() >= 8 )
    {
        version = WS_V0;
        request.append( tcpSocket->read(8) );
    }
    else
    {
        version = WS_VUnknow;
    }


    // Key
    QString key, key1, key2, key3;
    if ( version >= WS_V4 )
    {
        regExp.setPattern( QIOServer::regExpKeyStr );
        regExp.indexIn(request);
        key = regExp.cap(1);
    }
    else
    {
        regExp.setPattern( QIOServer::regExpKey1Str );
        regExp.indexIn(request);
        key1 = regExp.cap(1);
        regExp.setPattern( QIOServer::regExpKey2Str );
        regExp.indexIn(request);
        key2 = regExp.cap(1);
        regExp.setPattern( QIOServer::regExpKey3Str );
        regExp.indexIn(request);
        key3 = regExp.cap(1);
    }

    ////////////////////////////////////////////////////////////////////

    // If the mandatory fields are not specified, we abord the connection to the Websocket server
    if ( version == WS_VUnknow || resourceName.isEmpty() || hostAddress.isEmpty() || ( key.isEmpty() && ( key1.isEmpty() || key2.isEmpty() || key3.isEmpty() ) ) )
    {
        // Send bad request response
        QString response = QIOServer::composeBadRequestResponse( QList<EWebsocketVersion>() << WS_V6 << WS_V7 << WS_V8 << WS_V13 );
        tcpSocket->write( response.toUtf8() );
        tcpSocket->flush();
        return;
    }

    ////////////////////////////////////////////////////////////////////

    // Extract optional datas

    // Origin
    regExp.setPattern( QIOServer::regExpOriginStr );
    if ( regExp.indexIn(request) == -1 )
    {
        regExp.setPattern( QIOServer::regExpOrigin2Str );
        regExp.indexIn(request);
    }
    QString origin = regExp.cap(1);

    // Protocol
    regExp.setPattern( QIOServer::regExpProtocolStr );
    regExp.indexIn(request);
    QString protocol = regExp.cap(1);

    // Extensions
    regExp.setPattern( QIOServer::regExpExtensionsStr );
    regExp.indexIn(request);
    QString extensions = regExp.cap(1);

    ////////////////////////////////////////////////////////////////////

    // Compose opening handshake response
    QString response;

    if ( version >= WS_V6 )
    {
        QString accept = computeAcceptV4( key );
        response = QIOServer::composeOpeningHandshakeResponseV6( accept, protocol );
    }
    else if ( version >= WS_V4 )
    {
        QString accept = computeAcceptV4( key );
        QString nonce = generateNonce();
        response = QIOServer::composeOpeningHandshakeResponseV4( accept, nonce, protocol );
    }
    else
    {
        QString accept = computeAcceptV0( key1, key2, key3 );
        response = QIOServer::composeOpeningHandshakeResponseV0( accept, origin, hostAddress, hostPort, resourceName , protocol );
    }

    // Handshake OK, disconnect readyRead
    disconnect( tcpSocket, SIGNAL(readyRead()), this, SLOT(dataReceived()) );
    _connected = true;

    // Send opening handshake response
    if ( version == WS_V0 )
        tcpSocket->write( response.toLatin1() );
    else
        tcpSocket->write( response.toUtf8() );
    tcpSocket->flush();

    QWsSocket * wsSocket = new QWsSocket( this, tcpSocket, version );
    wsSocket->setResourceName( resourceName );
    wsSocket->setHost( host );
    wsSocket->setHostAddress( hostAddress );
    wsSocket->setHostPort( hostPort.toInt() );
    wsSocket->setOrigin( origin );
    wsSocket->setProtocol( protocol );
    wsSocket->setExtensions( extensions );
    wsSocket->serverSideSocket = true;

    wsSocket->write(QString("1::"));

    // ORIGINAL CODE
    //int socketDescriptor = tcpSocket->socketDescriptor();
    //incomingConnection( socketDescriptor );

    // CHANGED CODE FOR LINUX COMPATIBILITY

    connect( wsSocket, SIGNAL(frameReceived(QString)), this, SLOT(frameReceivedHandler(QString)) );
    connect( wsSocket, SIGNAL(disconnected()), this, SLOT(socketDisconnectedHandler()) );

    _clients << wsSocket;
    //addPendingConnection( wsSocket );
    emit newConnection();
}


void QIOServer::frameReceivedHandler( QString frame )
{
    QWsSocket * socket = qobject_cast<QWsSocket*>( sender() );
    if (socket == 0)
        return;

    emit newMessage(frame);
}

void QIOServer::socketDisconnectedHandler()
{
    QWsSocket * socket = qobject_cast<QWsSocket*>( sender() );
    if (socket == 0)
        return;

    _clients.removeOne(socket);

    socket->deleteLater();

    emit socketDisconnected();
}

void QIOServer::sendHeartbeats()
{
    //qDebug() << "IOServer:sendHeartbeats ";
    QWsSocket * client;
    foreach ( client, _clients )
    {
        //QMetaObject::invokeMethod(client, "write", Q_ARG(const QString &, QString("2::")));
        client->write( QString("2::") );
    }
}


//void QIOServer::addPendingConnection( QWsSocket * socket )
//{
//	if ( pendingConnections.size() < maxPendingConnections() )
//		pendingConnections.enqueue( socket );
//}

//QWsSocket * QIOServer::nextPendingConnection()
//{
//	return pendingConnections.dequeue();
//}

//bool QIOServer::hasPendingConnections()
//{
//	if ( pendingConnections.size() > 0 )
//		return true;
//	return false;
//}



/*  QTcpServer Functions  */

QAbstractSocket::SocketError QIOServer::serverError()
{
    return _tcpServer->serverError();
}

QString QIOServer::errorString()
{
    return _tcpServer->errorString();
}

bool QIOServer::isListening()
{
    return _tcpServer->isListening();
}

QNetworkProxy QIOServer::proxy()
{
    return _tcpServer->proxy();
}

QHostAddress QIOServer::serverAddress()
{
    return _tcpServer->serverAddress();
}

quint16 QIOServer::serverPort()
{
    return _tcpServer->serverPort();
}

void QIOServer::setMaxPendingConnections( int numConnections )
{
    _tcpServer->setMaxPendingConnections( numConnections );
}

void QIOServer::setProxy( const QNetworkProxy & networkProxy )
{
    _tcpServer->setProxy( networkProxy );
}

bool QIOServer::setSocketDescriptor( int socketDescriptor )
{
    return _tcpServer->setSocketDescriptor( socketDescriptor );
}

int QIOServer::socketDescriptor()
{
    return _tcpServer->socketDescriptor();
}

bool QIOServer::waitForNewConnection( int msec, bool * timedOut )
{
    return _tcpServer->waitForNewConnection( msec, timedOut );
}

int QIOServer::maxPendingConnections()
{
    return _tcpServer->maxPendingConnections();
}


/*   Static Functions   */

QString QIOServer::computeSocketIOSession(QString key)
{
    //TODO: generate key differently?
	key += QLatin1String("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
	QByteArray hash = QCryptographicHash::hash ( key.toUtf8(), QCryptographicHash::Sha1 );
    return hash.toBase64()+ ":60:30:websocket";
}

QString QIOServer::computeAcceptV0( QString key1, QString key2, QString key3 )
{
    QString numStr1;
    QString numStr2;

    QChar carac;
    for ( int i=0 ; i<key1.size() ; i++ )
    {
        carac = key1[ i ];
        if ( carac.isDigit() )
            numStr1.append( carac );
    }
    for ( int i=0 ; i<key2.size() ; i++ )
    {
        carac = key2[ i ];
        if ( carac.isDigit() )
            numStr2.append( carac );
    }

    quint32 num1 = numStr1.toUInt();
    quint32 num2 = numStr2.toUInt();

    int numSpaces1 = key1.count( ' ' );
    int numSpaces2 = key2.count( ' ' );

    num1 /= numSpaces1;
    num2 /= numSpaces2;

    QString concat = serializeInt( num1 ) + serializeInt( num2 ) + key3;

    QByteArray md5 = QCryptographicHash::hash( concat.toLatin1(), QCryptographicHash::Md5 );

    return QString( md5 );
}

QString QIOServer::computeAcceptV4(QString key)
{
    key += QLatin1String("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
    QByteArray hash = QCryptographicHash::hash ( key.toUtf8(), QCryptographicHash::Sha1 );
    return hash.toBase64();
}

QString QIOServer::generateNonce()
{
	qsrand( QDateTime::currentDateTime().toTime_t() );

	QByteArray nonce;
	int i = 16;

	while( i-- )
	{
		nonce.append( qrand() % 0x100 );
	}

	return QString( nonce.toBase64() );
}

QByteArray QIOServer::serializeInt( quint32 number, quint8 nbBytes )
{
	QByteArray ba;
	quint8 currentNbBytes = 0;
	while (number > 0 && currentNbBytes < nbBytes)
	{
		char car = static_cast<char>(number & 0xFF);
		ba.prepend( car );
		number = number >> 8;
		currentNbBytes++;
	}
	char car = 0x00;
	while (currentNbBytes < nbBytes)
	{
		ba.prepend( car );
		currentNbBytes++;
	}
	return ba;
}

QByteArray QIOServer::composeEngineIOAccept( QString sessionID, QString origin )
{
    QByteArray response;
    static const char EngineIOConnectPrefix[] = {0,9,9,"\xff"};
    QByteArray message = QByteArray::fromRawData(EngineIOConnectPrefix, sizeof(EngineIOConnectPrefix)-1)
            + QString("0{\"sid\":\"%1\",\"upgrades\":[\"websocket\"],\"pingInterval\":25000,\"pingTimeout\":60000}")
            .arg(sessionID).toLocal8Bit();

    response.append( QLatin1String("HTTP/1.1 200 OK\r\n") );
    response.append( QLatin1String("Server: QtSocketIOServer/1.0\r\n") );
    response.append( QLatin1String("Access-Control-Allow-Origin: ") + origin + QLatin1String("\r\n") );
    response.append( QLatin1String("Access-Control-Allow-Credentials: true\r\n") );
    response.append( QLatin1String("Content-Type: application/octet-stream\r\n") );
    response.append( QLatin1String("Content-Length: ")  + QString::number(message.size()) + QLatin1String("\r\n") );
    response.append( QLatin1String("Connection: keep-alive\r\n") );
    response.append( QLatin1String("Set-Cookie: io=")  + sessionID + QLatin1String("\r\n") );
    response.append( QLatin1String("\r\n") );
    response.append( message );

    return response;
}

QByteArray QIOServer::composeEngineIOConect( QString sessionID, QString origin )
{
    QByteArray response;
    static const char EngineIOConnectPrefix[] = {0,2,"\xff"};
    QByteArray message = QByteArray::fromRawData(EngineIOConnectPrefix, sizeof(EngineIOConnectPrefix)-1)
            + QString("40").toLocal8Bit();

    response.append( QLatin1String("HTTP/1.1 200 OK\r\n") );
    response.append( QLatin1String("Server: QtSocketIOServer/1.0\r\n") );
    response.append( QLatin1String("Access-Control-Allow-Origin: ") + origin + QLatin1String("\r\n") );
    response.append( QLatin1String("Access-Control-Allow-Credentials: true\r\n") );
    response.append( QLatin1String("Content-Type: application/octet-stream\r\n") );
    response.append( QLatin1String("Content-Length: ")  + QString::number(message.size()) + QLatin1String("\r\n") );
    response.append( QLatin1String("Connection: keep-alive\r\n") );
    response.append( QLatin1String("Set-Cookie: io=")  + sessionID + QLatin1String("\r\n") );
    response.append( QLatin1String("\r\n") );
    response.append( message );

    return response;
}

QByteArray QIOServer::composeEngineIOPong( QString sessionID, QString origin )
{
    QByteArray response;
    static const char EngineIOConnectPrefix[] = {0,1,"\xff"};
    QByteArray message = QByteArray::fromRawData(EngineIOConnectPrefix, sizeof(EngineIOConnectPrefix)-1)
            + QString("3").toLocal8Bit();

    response.append( QLatin1String("HTTP/1.1 200 OK\r\n") );
    response.append( QLatin1String("Server: QtSocketIOServer/1.0\r\n") );
    response.append( QLatin1String("Access-Control-Allow-Origin: ") + origin + QLatin1String("\r\n") );
    response.append( QLatin1String("Access-Control-Allow-Credentials: true\r\n") );
    response.append( QLatin1String("Content-Type: application/octet-stream\r\n") );
    response.append( QLatin1String("Content-Length: ")  + QString::number(message.size()) + QLatin1String("\r\n") );
    response.append( QLatin1String("Connection: keep-alive\r\n") );
    response.append( QLatin1String("Set-Cookie: io=")  + sessionID + QLatin1String("\r\n") );
    response.append( QLatin1String("\r\n") );
    response.append( message );

    return response;
}

QByteArray QIOServer::composeEngineIOUpgrade( QString protocol )
{
    QByteArray response;

    response.append( QLatin1String("HTTP/1.1 101 Switching Protocols\r\n") );
    response.append( QLatin1String("Access-Control-Allow-Origin: *\r\n") );
    response.append( QLatin1String("Content-Type: application/octet-stream\r\n") );
    response.append( QLatin1String("Connection: upgrade\r\n") );
    response.append( QLatin1String("Upgrade: ") + protocol + QLatin1String("\r\n") );
    response.append( QLatin1String("\r\n") );

    return response;
}

QString QIOServer::composeSocketIOHandshakeResponse( QString accept, QString origin, QString hostAddress, QString hostPort, QString resourceName, QString protocol )
{
	QString response;
	
    response.append( QLatin1String("HTTP/1.1 200 OK\r\n") );
    response.append( QLatin1String("Access-Control-Allow-Origin: ") + origin + QLatin1String("\r\n") );
    response.append( QLatin1String("Access-Control-Allow-Credentials: true\r\n") );
    response.append( QLatin1String("Content-Type: text/plain\r\n") );
    response.append( QLatin1String("Content-Length: ")  + QString::number(accept.size()) + QLatin1String("\r\n") );
    response.append( QLatin1String("Connection: keep-alive\r\n") );
	response.append( QLatin1String("\r\n") );
    response.append( accept );

	return response;
}

QString QIOServer::composeOpeningHandshakeResponseV0( QString accept, QString origin, QString hostAddress, QString hostPort, QString resourceName, QString protocol )
{
    QString response;

    response.append( QLatin1String("HTTP/1.1 101 WebSocket Protocol Handshake\r\n") );
    response.append( QLatin1String("Upgrade: Websocket\r\n") );
    response.append( QLatin1String("Connection: Upgrade\r\n") );
    response.append( QLatin1String("Sec-WebSocket-Origin: ") + origin + QLatin1String("\r\n") );
    response.append( QLatin1String("Sec-WebSocket-Location: ws://") + hostAddress);
    if (!hostPort.isEmpty())
        response.append(QLatin1String(":") + hostPort);
    response.append(resourceName + QLatin1String("\r\n"));
    if ( ! protocol.isEmpty() )
        response.append( QLatin1String("Sec-WebSocket-Protocol: ") + protocol + QLatin1String("\r\n") );
    response.append( QLatin1String("\r\n") );
    response.append( accept );

    return response;
}

QString QIOServer::composeOpeningHandshakeResponseV4( QString accept, QString nonce, QString protocol, QString extensions )
{
    QString response;

    response.append( QLatin1String("HTTP/1.1 101 Switching Protocols\r\n") );
    response.append( QLatin1String("Upgrade: websocket\r\n") );
    response.append( QLatin1String("Connection: Upgrade\r\n") );
    response.append( QLatin1String("Sec-WebSocket-Accept: ") + accept + QLatin1String("\r\n") );
    response.append( QLatin1String("Sec-WebSocket-Nonce: ") + nonce + QLatin1String("\r\n") );
    if ( ! protocol.isEmpty() )
        response.append( QLatin1String("Sec-WebSocket-Protocol: ") + protocol + QLatin1String("\r\n") );
    if ( ! extensions.isEmpty() )
        response.append( QLatin1String("Sec-WebSocket-Extensions: ") + extensions + QLatin1String("\r\n") );
    response.append( QLatin1String("\r\n") );

    return response;
}

QString QIOServer::composeOpeningHandshakeResponseV6( QString accept, QString protocol, QString extensions )
{
    QString response;

    response.append( QLatin1String("HTTP/1.1 101 Switching Protocols\r\n") );
    response.append( QLatin1String("Upgrade: websocket\r\n") );
    response.append( QLatin1String("Connection: Upgrade\r\n") );
    response.append( QLatin1String("Sec-WebSocket-Accept: ") + accept + QLatin1String("\r\n") );
    if ( ! protocol.isEmpty() )
        response.append( QLatin1String("Sec-WebSocket-Protocol: ") + protocol + QLatin1String("\r\n") );
    if ( ! extensions.isEmpty() )
        response.append( QLatin1String("Sec-WebSocket-Extensions: ") + extensions + QLatin1String("\r\n") );
    response.append( QLatin1String("\r\n") );

    return response;
}

QString QIOServer::composeBadRequestResponse( QList<EWebsocketVersion> versions )
{
	QString response;
	
	response.append( QLatin1String("HTTP/1.1 400 Bad Request\r\n") );
	if ( ! versions.isEmpty() )
	{
		QString versionsStr = QString::number( (int)versions.takeLast() );
		int i = versions.size();
		while ( i-- )
		{
			versionsStr.append( QLatin1String(", ") + QString::number( (int)versions.takeLast() ) );
		}
		response.append( QLatin1String("Sec-WebSocket-Version: ") + versionsStr + QLatin1String("\r\n") );
	}

	return response;
}
