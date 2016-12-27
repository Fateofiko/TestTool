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

    QString hostIp;
    QString hostPort;
    int hostId;

    void sendDataToHost( const QString &messageForHost );
    void sendPackageToHost(const QByteArray &packageForHost);
    void restartTheSocket();
    void abortSocketConnection();
    void sendHeartBeatPackage();
    bool establishConnection();
    void setHostIp(const QString &ip);
    void sendQueryConfiguration();
    void sendSetConfiguration();
    void sendQueryClientInfo( const QString &clientAddr );
    void sendDisplayToClient(const QString &clientAddr , const QString &boxCount);
    void sendDisplayConfiguration(const QString &clientAddr);
    void sendSpecialDisplay(const QString &clientAddr, const QString &screenMessage, int screenType);
    void sendScan(const QString &clientAddr, int scanType, int scanTime);
    void sendDisableDisplay(const QString &clientAddr, int typeD );
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
    void handleScannedMessage( ScanType type, BarcodeState state, const QString &scanedData );
private:

    QByteArray halfPackage;             ///<
    void createSocket();
    void manageBufferedData( QByteArray &buffer );

protected:
    void timerEvent(QTimerEvent *event);
};

#endif // CLIENTSOCKET_H
