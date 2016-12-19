#include "clientsocket.h"

ClientSocket::ClientSocket(QObject *parent) : QObject(parent)
{
    createSocket();
    startTimer( 5000 );
    counterHeartbeatDrops = 0;
    autoRestart = true;

    connect( &protocolManager, SIGNAL( executed_DTC(QString,QString)), this, SLOT( dataToClient(QString,QString) ) );
    connect( &protocolManager, SIGNAL( statusOk()), this, SLOT( handleStatusOk() ) );
    connect( &protocolManager, SIGNAL( executed_CurrentConfiguration(int,int,int,int,int,int)), this, SLOT( handleCurrentConfiguration(int,int,int,int,int,int)) );
    connect( &protocolManager, SIGNAL( executed_ClientInfo(QString,ClientStates)), this, SLOT( handleDeliverClientInfo(QString,ClientStates)) );
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
    if( tcpSocket && tcpSocket->state() != 0 ){
        if( counterHeartbeatDrops == MAX_HEARTBEAT_DROPS )
            tcpSocket->abort();
        else
            counterHeartbeatDrops ++;
    }
}

void ClientSocket::sendHeartBeatPackage()
{
        counterHeartbeatDrops = 0;
        QByteArray package;
        protocolManager.setProtocolAddress( QString::number( HOST_ID ) );
        protocolManager.createEmptyPackage( package );
        protocolManager.insertCommand_DFC( package, QString("%1:%2;%3:%4").arg(CMD).arg(DATA_CMD_HEARTBEAT)
                                                                          .arg(FIELD_STATUS).arg( DATA_STATUS_OK ), QString::number( HOST_ID ), false);
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
  QHostAddress address( QString( HOST_IP ) );
  quint16 port = HOST_PORT;
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
                    protocolManager.parsePackage(halfPackage);
                }
                halfPackage.clear();
            }
        }
    }
}



void ClientSocket::dataToClient( const QString &addr, const QString &data )
{
    qDebug() << data << addr;
    if( data.contains( DATA_CMD_HEARTBEAT ) ){
        sendHeartBeatPackage();
    }

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

void ClientSocket::sendQueryConfiguration()
{
    QByteArray package;
    protocolManager.setProtocolAddress( QString::number( HOST_ID ) );
    protocolManager.createEmptyPackage( package );
    protocolManager.insertCommand_QueryConfiguration(package,false);
    tcpSocket->write(package);
}

void ClientSocket::sendSetConfiguration()
{
    QByteArray package;
    protocolManager.setProtocolAddress( QString::number( HOST_ID ) );
    protocolManager.createEmptyPackage( package );
    protocolManager.insertCommand_SetConfiguration(package, 100, 200, 300, 2, 3, 4, false);
    tcpSocket->write(package);
}

void ClientSocket::sendQueryClientInfo(const QString &clientAddr)
{
    QByteArray package;
    protocolManager.setProtocolAddress( QString::number( HOST_ID ) );
    protocolManager.createEmptyPackage( package );
    protocolManager.insertCommand_QueryClientInfo(package, clientAddr, false);
    tcpSocket->write(package);
}
