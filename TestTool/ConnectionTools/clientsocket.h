#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QDebug>
#include <QMovie>
#include <QMessageBox>
#include <Protocol/protocolmanager.h>
#include <QHostAddress>

#define HOST_ID 101
#define HOST_PORT 36447
#define HOST_IP "192.168.0.101"
#define MAX_HEARTBEAT_DROPS 2

class ClientSocket : public QObject
{
    Q_OBJECT
public:
    explicit ClientSocket(QObject *parent = 0);
    ~ ClientSocket();

    QTcpSocket *tcpSocket;              ///<
    ProtocolManager protocolManager;    ///<

    int counterHeartbeatDrops;
    bool autoRestart;

    void sendDataToHost( const QString &messageForHost );
    void sendPackageToHost(const QByteArray &packageForHost);
    void restartTheSocket();
    void abortSocketConnection();
    void sendHeartBeatPackage();
    bool establishConnection();
    void sendQueryConfiguration();
    void sendSetConfiguration();
    void sendQueryClientInfo( const QString &clientAddr );
signals:

public slots:
    void socketConnected();
    void socketDisconnected();
    void readResposeData();
    void displayError( QAbstractSocket::SocketError socketError);
    void dataToClient( const QString &addr, const QString &data );

    void handleCurrentConfiguration(int reactionTimeOut, int charTimeOut, int pauseTime, int scanRate, int checkPollRate, int connectionRepeats);
    void handleDeliverClientInfo(const QString &clientAddr, ClientStates state );
    void handleStatusOk();
private:

    QByteArray halfPackage;             ///<
    void createSocket();
    void manageBufferedData( QByteArray &buffer );

protected:
    void timerEvent(QTimerEvent *event);
};

#endif // CLIENTSOCKET_H
