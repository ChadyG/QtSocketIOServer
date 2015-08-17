#ifndef QIOSERVER_H
#define QIOSERVER_H

#include <QSslSocket>
#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>
#include <QTimer>
#include <QNetworkProxy>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QQueue>

#ifndef QT_NO_OPENSSL
#include "qsslserver_p.h"
#endif
#include "QWsSocket.h"

enum EIOsocketMessages
{
    IO_MUnknown = -1,
    IO_MDisconnect = 0,
    IO_MConnect = 1,
    IO_MHeartbeat = 2,
    IO_MMessage = 3,
    IO_MJSON = 4,
    IO_MEvent = 5,
    IO_MACK = 6,
    IO_MError = 7,
    IO_MNoop = 8
};

struct ServerPri;
class QIOServer : public QObject
{
    Q_OBJECT

public:
    QIOServer(QObject * parent = 0);
    virtual ~QIOServer();

    QStringList getHeaders(QString socketUuid);

    //QTcpServer functions
    QString errorString();
    bool isListening();
    int maxPendingConnections();
    QNetworkProxy proxy();
    QHostAddress serverAddress();
    QAbstractSocket::SocketError serverError();
    quint16 serverPort();
    void setMaxPendingConnections( int numConnections );
    void setProxy( const QNetworkProxy & networkProxy );
    bool setSocketDescriptor( int socketDescriptor );
    int socketDescriptor();
    bool waitForNewConnection( int msec = 0, bool * timedOut = 0 );


signals:
    void newConnection(QString socketUuid);
    void socketDisconnected(QString socketUuid);
    void newMessage(QString socketUuid, QString message);

protected:
    void processSocketHandshake(QTcpSocket *tcpSocket, QString request);

public slots:
    void start();
    void close();
    void sendMessage(const QString &socketUuid, const QString &message);
    void sendMessage(const QString &message);
    void listen(quint16 port = 0, const QHostAddress & address = QHostAddress::Any, bool ssl = false);


private slots:
    // incoming connections
    void newTcpConnection();
    void closeTcpConnection();
    void dataReceived();

#ifndef QT_NO_OPENSSL
    void peerVerifyError(const QSslError& error);
    void sslErrors(const QList<QSslError>& errors);
#endif

    // connected sockets
    void error(QAbstractSocket::SocketError socketError);
    void frameReceivedHandler( QString message );
    void socketDisconnectedHandler();
    void sendHeartbeats();

private:
    QTcpServer * _tcpServer;
    QQueue<QWsSocket*> pendingConnections;
    QMap<const QTcpSocket*, QStringList> headerBuffer;
    QMap<QString, QWsSocket*> _clients;
    QMap<QWsSocket*, QString> _clientUuids;
    bool _connected;
    QTimer* _timer;

public:
    // public static functions
    static QByteArray serializeInt( quint32 number, quint8 nbBytes = 4 );
    static QString computeAcceptV0( QString key1, QString key2, QString thirdPart );
    static QString computeAcceptV4( QString key );
    static QString computeSocketIOSession( QString key );
    static QString generateNonce();
    static QString composeOpeningHandshakeResponseV0( QString accept, QString origin, QString hostAddress, QString hostPort, QString resourceName, QString protocol = "" );
    static QString composeOpeningHandshakeResponseV4( QString accept, QString nonce, QString protocol = "", QString extensions = "" );
    static QString composeOpeningHandshakeResponseV6( QString accept, QString protocol = "", QString extensions = "" );
    static QString composeSocketIOHandshakeResponse( QString accept, QString origin, QString hostAddress, QString hostPort, QString resourceName, QString protocol = "" );
    static QString composeBadRequestResponse( QList<EWebsocketVersion> versions = QList<EWebsocketVersion>() );

    // public static vars
    static const QString regExpResourceNameStr;
    static const QString regExpHostStr;
    static const QString regExpOriginStr;


    static const QString regExpKeyStr;
    static const QString regExpKey1Str;
    static const QString regExpKey2Str;
    static const QString regExpKey3Str;
    static const QString regExpVersionStr;
    static const QString regExpOrigin2Str;
    static const QString regExpProtocolStr;
    static const QString regExpExtensionsStr;
};

#endif // QIOSERVER_H
