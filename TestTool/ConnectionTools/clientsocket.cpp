#include "clientsocket.h"

ClientSocket::ClientSocket(QObject *parent) : QObject(parent)
{
    createSocket();
    startTimer( 50000 );
    counterHeartbeatDrops = 0;
    autoRestart = true;
    hostIp = "";
    hostPort = "";
    hostId = 0;
    testNoReceiver = 0;

    connect( &protocolManager, SIGNAL( executed_DTC(QString,QString)), this, SLOT( dataToClient(QString,QString) ) );
    connect( &protocolManager, SIGNAL( statusOk()), this, SLOT( handleStatusOk() ) );
    connect( &protocolManager, SIGNAL( executed_CurrentConfiguration(int,int,int,int,int,int)), this, SLOT( handleCurrentConfiguration(int,int,int,int,int,int)) );
    connect( &protocolManager, SIGNAL( executed_ClientInfo(QString,ClientStates)), this, SLOT( handleDeliverClientInfo(QString,ClientStates)) );
    connect( &protocolManager, SIGNAL( executed_ScannedMessage(ScanType,BarcodeState,QString)), this, SLOT( handleScannedMessage(ScanType,BarcodeState,QString) ) );
    connect( &protocolManager, SIGNAL( executed_ClientResetDone(QString,QString,QString)), this, SLOT( handleClientResetDone(QString,QString,QString) ) );
    connect( &protocolManager, SIGNAL( executed_KeyMessage(QList<KeyState>,int,int)), this, SLOT( handleKeyMessage(QList<KeyState>,int,int) ) );
}

ClientSocket::~ClientSocket()
{
//    if( tcpSocket != NULL){
//        tcpSocket->abort();
//        tcpSocket->waitForDisconnected();
//        delete tcpSocket;
//    }

}

void ClientSocket::timerEvent(QTimerEvent *event)
{
//    if( tcpSocket && tcpSocket->state() != 0 ){
//        if( counterHeartbeatDrops == MAX_HEARTBEAT_DROPS )
//            tcpSocket->abort();
//        else
//            counterHeartbeatDrops ++;
//    }
    QByteArray package;
    protocolManager.setProtocolAddress( QString::number( hostId!=0 ? hostId : HOST_ID ) );
    protocolManager.createEmptyPackage( package );
    tcpSocket->write(package);
}

void ClientSocket::sendHeartBeatPackage()
{
        counterHeartbeatDrops = 0;
        QByteArray package;
        protocolManager.setProtocolAddress( QString::number( hostId!=0 ? hostId : HOST_ID ) );
        protocolManager.createEmptyPackage( package );
        protocolManager.insertCommand_DFC( package, QString("%1:%2;%3:%4").arg(CMD).arg(DATA_CMD_HEARTBEAT)
                                                                          .arg(FIELD_STATUS).arg( DATA_STATUS_OK ), QString::number( hostId!=0 ? hostId : HOST_ID ), false);
        tcpSocket->write(package);
}

void ClientSocket::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        qDebug()<< "The host was not found. Please check the host name and port settings.";
        break;
    case QAbstractSocket::ConnectionRefusedError:
        qDebug()<< "The connection was refused by the peer. Make sure the fortune server is running, and check that the host name and port settings are correct.";
        break;
    default:
        qDebug()<< QString("The following error occurred: %1.").arg( tcpSocket->errorString() );
    }
}

void ClientSocket::createSocket()
{
    tcpSocket = new QTcpSocket(this);
    connect(tcpSocket, SIGNAL(connected()), this, SLOT(socketConnected()));
    connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readResposeData()));
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));
}

void ClientSocket::socketConnected()
{
    qDebug()<<"Socket is connected ...";
}

void ClientSocket::socketDisconnected()
{
    qDebug() << "Socket disconnected!";
    if( autoRestart )
        restartTheSocket();
    autoRestart = true;
}

void ClientSocket::restartTheSocket()
{
    qDebug() << "Restarting the socket ... ";
    if( !establishConnection() ){
        qDebug()<< "Connection is lost! Please check the server or set the server settings!" ;
        return;
    }
}

void ClientSocket::abortSocketConnection()
{
    autoRestart = false;
    tcpSocket->abort();
}

bool ClientSocket::establishConnection()
{
  counterHeartbeatDrops = 0;
  QHostAddress address( QString( !hostIp.isEmpty() ? hostIp : HOST_IP ) );
  quint16 port = !hostPort.isEmpty() ? hostPort.toInt() : HOST_PORT ;
  int tries = 0;
  do{
      tcpSocket->abort();
      tcpSocket->connectToHost( address, port );
      if( tcpSocket->waitForConnected( 1000 ) )
          return true;
      tries++;
      qDebug()<<"not connected";
  } while ( tries < 4 );
  return false;
//    connect( tcpSocket, SIGNAL(disconnected()), tcpSocket, SLOT(deleteLater()) );
}


void ClientSocket::sendDataToHost( const QString &messageForHost )
{
    QByteArray array;
    array.append( messageForHost );
    tcpSocket->write(array);
    tcpSocket->flush();
}

void ClientSocket::sendPackageToHost( const QByteArray &packageForHost )
{
    if( tcpSocket && tcpSocket->state() != 0){
        tcpSocket->write( packageForHost );
        tcpSocket->flush();
    }
}

void ClientSocket::readResposeData()
{
    qDebug() << "Client Data for read ...";
    quint64 maxLen = tcpSocket->bytesAvailable();
    QByteArray package;
    if( maxLen > 0 ){
        package = tcpSocket->read( maxLen );
        manageBufferedData( package );
    }
}

void ClientSocket::manageBufferedData(QByteArray &buffer)
{
    if( halfPackage.isEmpty() ){
        if( buffer.contains( STX ) ){
            int startStx = buffer.indexOf(STX);
            for( int i = startStx; i < buffer.size(); i++){
                if( buffer.at(i) != ETX ){
                    halfPackage.append(buffer.at(i));
                } else {
                    halfPackage.append(buffer.at(i));
                    if( halfPackage.contains(STX) && halfPackage.contains(ETX) ){
                        protocolManager.parsePackage(halfPackage);
//                        qDebug()<< QString(halfPackage);
                    }
                    halfPackage.clear();
                }
            }
        } else {
            qDebug() << QString("Trash: %1").arg( QString( buffer ) );
        }
    } else if( halfPackage.contains(STX) ) {
        for( int i = 0; i < buffer.size(); i++){
            if( buffer.at(i) != ETX ){
                halfPackage.append(buffer.at(i));
            } else {
                halfPackage.append(buffer.at(i));
                if( halfPackage.contains(STX) && halfPackage.contains(ETX) ){
                    if( protocolManager.parsePackage(halfPackage) )
                        sendStateOk();
                    else
                        sendStateNoReceiver();
                    qDebug()<< QString(halfPackage);
                }
                halfPackage.clear();
            }
        }
    }
}



void ClientSocket::dataToClient( const QString &addr, const QString &data )
{
    qDebug() << data << addr;
//    if( data.contains( DATA_CMD_HEARTBEAT ) ){
//        sendHeartBeatPackage();
//    }

//    QByteArray package;
//    protocolManager.createStatusPackage( package, OK );
//    protocolManager.createEmptyPackage(package);
//    protocolManager.insertCommand_DFC( package,"Hi I am client It is nice to share bytes ...","13",false );
//    sendPackageToHost( package );
//=================== Test capabilities of the server  When you use this test please comment the code above =======================
//    sendDataToHostFromAllSocketsByProtocol();

}

void ClientSocket::handleCurrentConfiguration(int reactionTimeOut, int charTimeOut, int pauseTime, int scanRate, int checkPollRate, int connectionRepeats)
{
    qDebug()<< QString("reactionTimeOut = %1, charTimeOut = %2, pauseTime = %3, scanRate = %4, checkPollRate = %5, connectionRepeats = %6")
               .arg(reactionTimeOut)
               .arg(charTimeOut)
               .arg(pauseTime)
               .arg(scanRate)
               .arg(checkPollRate)
               .arg(connectionRepeats);
}

void ClientSocket::handleDeliverClientInfo(const QString &clientAddr, ClientStates state)
{
    switch(state){
        case CLIENT_DISAPPEARED : qDebug()<<QString("Client with ID: %1 / status: %2 ").arg(clientAddr).arg("client disappeared"); break;
        case CLIENT_PRESENT :qDebug()<<QString("Client with ID: %1 / status: %2 ").arg(clientAddr).arg("client present");break;
        case CLIENT_NOT_PRESENT: qDebug()<<QString("Client with ID: %1 / status: %2 ").arg(clientAddr).arg("addressed client not present");break;
        default: qDebug()<<QString("Client with ID: %1 / status: %2 ").arg(clientAddr).arg("Unknown status"); break;
    }
}

void ClientSocket::handleStatusOk()
{
    qDebug()<<"status OK ...";
}

void ClientSocket::handleScannedMessage(ScanType type, BarcodeState state, const QString &scanedData)
{
    QString scanType;
    QString scanState;
    switch(type){
        case SCAN_AUTOMATIC : scanType = "auto"; break;
        case SCAN_MANUAL : scanType = "manual"; break;
        default: scanType = "error"; break;
    }
    sendStateOk();
    switch(state){
        case SUCCESSFULLY_SCANNED : scanState = "Barcode successfully scanned"; break;
        case TIMEOUT : scanState = "Timeout (ACT time exceeded)"; break;
        case CAMERA_NOT_AVAILABLE: scanState = "Camera not available/connected"; break;
        case CAMERA_INTERNAL_ERROR :  scanState = "Camera internal error"; break;
        case OCR_ERROR : scanState = " OCR Error (too dark, ...)"; break;
        case UNSPECIFIED_ERROR: scanState = "Unspecified Error"; break;
        default:  scanState = "Error"; break;
    }

    qDebug()<<scanedData<< QString("  %1  %2  ").arg(scanType).arg(scanState);
}

void ClientSocket::handleClientResetDone(const QString &hardwareV, const QString &softwareV, const QString &clientId)
{
    qDebug()<<"ResetDone: "<<hardwareV;
    sendStateOk();
}

void ClientSocket::handleKeyMessage(QList<KeyState> state, int keyTime, int timeSince)
{
    sendStateOk();
}

void ClientSocket::sendQueryConfiguration()
{
    QByteArray package;
    protocolManager.setProtocolAddress( QString::number( hostId!=0 ? hostId : HOST_ID ) );
    protocolManager.createEmptyPackage( package );
    protocolManager.insertCommand_QueryConfiguration(package,false);
    tcpSocket->write(package);
}

void ClientSocket::sendSetConfiguration()
{
    QByteArray package;
    protocolManager.setProtocolAddress( QString::number( hostId!=0 ? hostId : HOST_ID ) );
    protocolManager.createEmptyPackage( package );
    protocolManager.insertCommand_SetConfiguration(package, 100, 200, 300, 2, 3, 4, false);
    tcpSocket->write(package);
}

void ClientSocket::sendQueryClientInfo(const QString &clientAddr)
{
    QByteArray package;
    protocolManager.setProtocolAddress( QString::number( hostId!=0 ? hostId : HOST_ID ) );
    protocolManager.createEmptyPackage( package );
    protocolManager.insertCommand_QueryClientInfo(package, clientAddr, false);
    tcpSocket->write(package);
}

void ClientSocket::sendDisplayToClient(const QString &clientAddr, const QString &boxCount, bool red, bool green, bool blue )
{
    QByteArray package;
    protocolManager.setProtocolAddress( QString::number( hostId!=0 ? hostId : HOST_ID ) );
    protocolManager.createEmptyPackage( package );
    QString colors = QString("%1%2%3").arg( red ? '1' : '0').arg( green ? '1' : '0').arg( blue ? '1' : '0');
    colors.append("010");
    protocolManager.insertCommand_DTC(package,QString("E002%1iioo010%2").arg( boxCount ).arg(colors), clientAddr, false);
    tcpSocket->write(package);
//    sendPackageOnPieces( package );
//    sendPackageDoubleOnPieces( package );
}

void ClientSocket::sendPackageOnPieces( QByteArray &package )
{
    tcpSocket->write( package.mid(0,5) );
    tcpSocket->flush();
    tcpSocket->write( package.mid( 5, 5 ) );
    tcpSocket->flush();
    tcpSocket->write( package.mid( 10, package.size()-10 ) );
    tcpSocket->flush();
}

void ClientSocket::sendPackageDoubleOnPieces( QByteArray &package )
{
    QByteArray newPackage;
    newPackage.append(package);
    newPackage.append(package);

    tcpSocket->write( newPackage.mid(0,10) );
    tcpSocket->flush();
    tcpSocket->write( newPackage.mid( 10, 10 ) );
    tcpSocket->flush();
    tcpSocket->write( newPackage.mid( 20, newPackage.size()-20 ) );
    tcpSocket->flush();
}

void ClientSocket::sendDisplayConfiguration(const QString &clientAddr)
{
    QByteArray package;
    protocolManager.setProtocolAddress( QString::number( hostId!=0 ? hostId : HOST_ID ) );
    protocolManager.createEmptyPackage( package );
    protocolManager.insertCommand_DTC(package, QString("V00002016"), clientAddr, false);
    tcpSocket->write(package);
}

void ClientSocket::sendSpecialDisplay(const QString &clientAddr, const QString &screenMessage, int screenType)
{
    SpecialDisplayType type;
    switch(screenType){
        case 1: type = DISPLAY_EMPTY_CARTON ;       break;
        case 2: type = DISPLAY_CARTON_EVACUATION;   break;
        case 3: type = DISPLAY_ERROR ;              break;
    default:    type = DISPLAY_EMPTY_CARTON;        break;
    }
    QChar cT = (int) type;
    QByteArray package;
    protocolManager.setProtocolAddress( QString::number( hostId!=0 ? hostId : HOST_ID ) );
    protocolManager.createEmptyPackage( package );
    protocolManager.insertCommand_DTC(package, QString("F%1%2%3").arg(QString(cT)).arg(screenMessage).arg("01000000"), clientAddr, false);
    tcpSocket->write(package);
}

void ClientSocket::sendScan(const QString &clientAddr, int scanType, int scanTime)
{
    ScanType type;
    switch(scanType){
        case 1: type = SCAN_AUTOMATIC ;       break;
        case 2: type = SCAN_MANUAL;   break;
    default:    type = SCAN_MANUAL;        break;
    }
    char cT = (int) type;
    QString time = QString::number(scanTime).right(3);
    for(int i = time.size(); i < 3; i++){
        time.append('0');
    }
    QByteArray package;
    protocolManager.setProtocolAddress( QString::number( hostId!=0 ? hostId : HOST_ID ) );
    protocolManager.createEmptyPackage( package );
    protocolManager.insertCommand_DTC(package, QString("S%1%2").arg(cT).arg( time ), clientAddr, false);
    tcpSocket->write(package);
}

void ClientSocket::sendDisableDisplay(const QString &clientAddr, int typeD)
{
    DisplayMode type;
    switch(typeD){
        case 1: type = SCAN_NORMAL ;       break;
        case 2: type = SCAN_SPECIAL_PATTERN;   break;
    default:    type = SCAN_NORMAL;        break;
    }
    char cT = (int) type;
    QByteArray package;
    protocolManager.setProtocolAddress( QString::number( hostId!=0 ? hostId : HOST_ID ) );
    protocolManager.createEmptyPackage( package );
    protocolManager.insertCommand_DTC(package, QString("Z%1").arg(cT), clientAddr, false);
    tcpSocket->write(package);
}

void ClientSocket::sendClientReset( const QString &clientAddr )
{
    QByteArray command;
    protocolManager.insertCommand_ClientReset( command, CLIENT_NORMAL_OPERATION, false );

    QByteArray package;
    protocolManager.setProtocolAddress( QString::number( hostId!=0 ? hostId : HOST_ID ) );
    protocolManager.createEmptyPackage( package );
    protocolManager.insertCommand_DTC(package, QString(command), clientAddr, false);
    tcpSocket->write(package);
}

void ClientSocket::sendHostReset()
{
    QByteArray package;
    protocolManager.setProtocolAddress( QString::number( hostId!=0 ? hostId : HOST_ID ) );
    protocolManager.createEmptyPackage( package );
    protocolManager.insertCommand_ClientReset( package, CLIENT_NORMAL_OPERATION, false );
    tcpSocket->write(package);
}

void ClientSocket::sendEmptyPackage()
{
    QByteArray package;
    protocolManager.setProtocolAddress( QString::number( hostId!=0 ? hostId : HOST_ID ) );
    protocolManager.createEmptyPackage( package );
    tcpSocket->write(package);
}

void ClientSocket::sendStateOk()
{
    QByteArray package;
    protocolManager.setProtocolAddress( QString::number( hostId!=0 ? hostId : HOST_ID ) );
    protocolManager.createStatusPackage(package, OK);
    tcpSocket->write(package);
}

void ClientSocket::sendStateNoReceiver()
{
    QByteArray package;
    protocolManager.setProtocolAddress( QString::number( hostId!=0 ? hostId : HOST_ID ) );
    protocolManager.createStatusPackage(package, NO_RECEIVER);
    tcpSocket->write(package);
}

void ClientSocket::setHostIp(const QString &ip)
{
    this->hostIp = ip;
    int dotIndex = ip.lastIndexOf('.');
    QString id = ip.mid( dotIndex+1 );
    this->hostId = id.toInt();
}
