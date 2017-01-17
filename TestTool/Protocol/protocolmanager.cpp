#include "protocolmanager.h"

ProtocolManager::ProtocolManager(QObject *parent) : QObject(parent)
{
    this->address = DEFAULT_ADDRESS;
    this->receivedPackageAddress = DEFAULT_ADDRESS;
    this->runningNumber = DEFAULT_RUNNING_NUMBER;
    this->receivedPackageRunningNumber = DEFAULT_RUNNING_NUMBER;
}

void ProtocolManager::createStatusPackage(QByteArray &package, ProtocolStates state)
{
    package.clear();
    appendHeader( package, false );
    char st = (int) state;
    package.append( st );
    appendFooter( package );
}

void ProtocolManager::setProtocolAddress( QString addr )
{
    if( fixAddressString( addr ) ){
        this->address = addr;
    } else {
        this->address = DEFAULT_ADDRESS;
    }
}

void ProtocolManager::setRunningNumber( char number )
{
    int n = number;
    if( n >= 48 && n <= 57 ){
        this->runningNumber = number;
    } else {
        this->runningNumber = '0';
    }
}

char ProtocolManager::getRunningNumber()
{
    return this->runningNumber;
}

void ProtocolManager::appendHeader( QByteArray &package, bool includeType )
{
    package.append( STX );
    for( int i = 0; i<4; i++){
        package.append( address.at(i) );
    }
    package.append( runningNumber );
    if( includeType ){
        package.append( TYPE );
    }
}

void ProtocolManager::appendFooter( QByteArray &package )
{
    package.append( EOT );
    package.append( FIRST_BYTE_FOR_NOCRC );
    package.append( SECOND_BYTE_FOR_NOCRC );
    package.append( ETX );
}

void ProtocolManager::appendSOH(QByteArray &command, bool append)
{
    if( append )
        command.append( SOH );
}

bool ProtocolManager::isDataContainsCommand(const QByteArray &command)
{
    switch( (int) command.at(0) ){
        case TERMINAL_CLIENT_SET_CONFIGURATION             :;
        case TERMINAL_CLIENT_DATA_TO_CLIENT                :;
        case TERMINAL_CLIENT_QUERY_CLIENT_INFO             :;
        case CLIENT_TERMINAL_SEND_CONFIGURATION            :;
        case CLIENT_TERMINAL_INFO                          :;
        case CLIENT_TERMINAL_DATA_FROM_CLIENT              :;
        case TERMINAL_DISPLAY_CHANGE_DEVICE_STATE          :;
        case TERMINAL_DISPLAY_SPECIAL_DISPLAY              :;
        case TERMINAL_DISPLAY_SCAN                         :;
        case TERMINAL_DISPLAY_DISABLE_DISPLAY              :;
        case DISPLAY_TERMINAL_KEY_MESSAGE                  :;
        case DISPLAY_TERMINAL_SCANNED_MESSAGE              :;
        case HOST_CLIENT_RESET                             :;
        case CLIENT_HOST_NEW_RESTART                       :;
        case CLIENT_HOST_RESET_DONE                        :;
        case CLIENT_HOST_ERROR_MESSAGE                     :return true; break;

        default: /*qDebug()<<"There is just a message!";*/ return false;
    }
}

void ProtocolManager::appendIntToCommand(QByteArray &command, int value, int numberOfBytes, int maxValue, int minValue, int defaultValue)
{
    QString valueStr = "";
    if( value >= minValue && value <= maxValue ){
        valueStr = QString::number(value);
    } else {
        value = defaultValue;
        valueStr = QString::number(value);
    }

    if( valueStr.size() < numberOfBytes ){
        for( int i = valueStr.size(); i < numberOfBytes; i++){
            valueStr.insert( 0, '0' );
        }
    }

    for( int i = 0; i < valueStr.size(); i++){
        command.append( valueStr.at(i) );
    }
}

void ProtocolManager::appendStrToCommand(QByteArray &command, QString &str, int numberOfBytes)
{
    str = str.left( numberOfBytes );
    if( str.size() < numberOfBytes ){
        for( int i = str.size(); i < numberOfBytes; i++){
            str.insert( 0, '0' );
        }
    }
    for( int i = 0; i < str.size(); i++){
        command.append( str.at(i) );
    }
}

void ProtocolManager::appendStrToCommand( QByteArray &command, const QString &str )
{
    for( int i = 0; i < str.size(); i++){
        command.append( str.at(i) );
    }
}

void ProtocolManager::appendDotsToCommand(QByteArray &command, const int numberOfDots)
{
    for( int i = 0; i < numberOfDots; i++){
        command.append( '.' );
    }
}

void ProtocolManager::encodeCommand( QByteArray &command )
{
    //ENCODE every single char by inclementing his value with 40h
    for(int i=0; i < command.size();i++){
        if( (int) command.at(i) <= (int) ENQ ){
            char single = (int) command.at(i) + ENCODING_STEP;
            command.remove(i,1);
            command.insert(i, ENQ);
            command.insert(i+1,single);
            i++;
        }
    }
}

void ProtocolManager::decodeCommand(QByteArray &command)
{
    //DECODE every single char by decrease his value with 40h
    for(int i=0; i < command.size();i++){
        if( (int) command.at(i) == (int) ENQ ){
            char single = (int) command.at( i+1 ) - ENCODING_STEP;
            command.remove(i,2);
            command.insert(i,single);
        }
    }
}

bool ProtocolManager::executeCommands(QList<QByteArray> &commands)
{
    for( int i = 0; i < commands.size(); i++ ){
        QByteArray nextCommand = commands.at(i);
        char command = nextCommand.at(0);
        switch( command ){
            case TERMINAL_CLIENT_SET_CONFIGURATION :
                if( nextCommand.size() == 2 ){
                    //To Do emit QUERY_CONFIGURATION
                    qDebug()<<"QUERY_CONFIGURATION";
                    execCommand_QueryConfiguration(nextCommand);
                } else if( nextCommand.size() == 25 ){
                    //To Do emit TERMINAL_CLIENT_SET_CONFIGURATION
                    qDebug()<<"TERMINAL_CLIENT_SET_CONFIGURATION";
                    execCommand_SetConfiguration(nextCommand);
                } else {
                    //To Do emit TERMINAL_DISPLAY_SET_CONFIGURATION_TO_DISPLAY
                    qDebug()<<"TERMINAL_DISPLAY_SET_CONFIGURATION_TO_DISPLAY";
                    execCommand_DisplaySetConf(nextCommand);
                }
            break;
            case TERMINAL_CLIENT_DATA_TO_CLIENT                :
                //qDebug()<<"TERMINAL_CLIENT_DATA_TO_CLIENT";
                execCommand_DTC(nextCommand);
            break;
            case TERMINAL_CLIENT_QUERY_CLIENT_INFO             :
                qDebug()<<"TERMINAL_CLIENT_QUERY_CLIENT_INFO";
                execCommand_QueryClientInfo(nextCommand);
            break;
            case CLIENT_TERMINAL_SEND_CONFIGURATION            :
                qDebug()<<"CLIENT_TERMINAL_SEND_CONFIGURATION";
                execCommand_CurrentConfiguration(nextCommand);
            break;
            case CLIENT_TERMINAL_INFO                          :
                qDebug()<<"CLIENT_TERMINAL_INFO";
                execCommand_ClientInfo(nextCommand);
            break;
            case CLIENT_TERMINAL_DATA_FROM_CLIENT              :
                //qDebug()<<"CLIENT_TERMINAL_DATA_FROM_CLIENT";
                execCommand_DFC( nextCommand );
            break;
            case TERMINAL_DISPLAY_CHANGE_DEVICE_STATE          :
                qDebug()<<"TERMINAL_DISPLAY_CHANGE_DEVICE_STATE";
                execCommand_Display( nextCommand );
            break;
            case TERMINAL_DISPLAY_SPECIAL_DISPLAY              :
                qDebug()<<"TERMINAL_DISPLAY_SPECIAL_DISPLAY";
                execCommand_SpecialDisplay( nextCommand );
            break;
            case TERMINAL_DISPLAY_SCAN                   :
                qDebug()<<"TERMINAL_DISPLAY_SCAN";
                execCommand_Scan(nextCommand);
            break;
            case TERMINAL_DISPLAY_DISABLE_DISPLAY              :
                qDebug()<<"TERMINAL_DISPLAY_DISABLE_DISPLAY";
                execCommand_DisableDisplay(nextCommand);
            break;
            case DISPLAY_TERMINAL_KEY_MESSAGE                  :
                qDebug()<<"DISPLAY_TERMINAL_KEY_MESSAGE";
                execCommand_KeyMessage(nextCommand);
            break;
            case DISPLAY_TERMINAL_SCANNED_MESSAGE              :
                qDebug()<<"DISPLAY_TERMINAL_SCANNED_MESSAGE";
                execCommand_ScannedMessage(nextCommand);
            break;
            case HOST_CLIENT_RESET                             :
                qDebug()<<"HOST_CLIENT_RESET";
                execCommand_ClientReset(nextCommand);
            break;
            case CLIENT_HOST_NEW_RESTART                       :
                qDebug()<<"CLIENT_HOST_NEW_RESTART";
                execCommand_ClientNewRestart(nextCommand);
            break;
            case CLIENT_HOST_RESET_DONE                        :
                qDebug()<<"CLIENT_HOST_RESET_DONE";
                execCommand_ClientResetDone(nextCommand);
            break;
            case CLIENT_HOST_ERROR_MESSAGE                     :
                qDebug()<<"CLIENT_HOST_ERROR_MESSAGE";
                execCommand_ClientErrorMessage(nextCommand);
            break;

            default: qDebug()<<"There is no sutch command!"; return false;
        }
    }
    return true;
}

bool ProtocolManager::defineStatus(const char status)
{
    switch( status ){
        case OK :
            emit statusOk();
        break;
        case NO_RECEIVER :
            emit statusNoReceiver();
        break;
        case A_LOT_OF_MESSAGES_IN_SINGLE_PACKAGE :
            emit statusMessagesOverFlow();
        break;
        case SAME_RUNNING_NUMBER :
            emit statusRunningNumberError();
        break;
        default:
            qDebug()<<"No such status"; return false;
    }
    return true;
}

bool ProtocolManager::fixAddressString( QString &address )
{
    if( address.size() < 4 ){
        for( int i = address.size(); i < 4; i++){
            address.insert( 0, '0' );
        }
    } else if( address.size() > 4 ){
        qDebug()<<"Address is longer than four characters ...";
        return false;
    }
    return true;
}

void ProtocolManager::checkValueForDots(QString &value)
{
    if( value.contains('.'))
        value = "-1";
}

void ProtocolManager::turnValueToDots( QString &value , int numberOfDots )
{
    if( value.toInt() < 0 ){
        value.clear();
        for( int i = 0; i < numberOfDots; i++){
            value.append('.');
        }
    }

}

void ProtocolManager::insertCommandToPackage(QByteArray &package, const QByteArray &command)
{
    if( package.isEmpty() ){
        package.append( command );
    } else if( package.size() >= PACKAGE_EMPTY_FRAME_SIZE ){
        package.insert( ( package.size() - APPEND_NEXT_COMMAND_PACKAGE_POSITION ), command );
    }
}

void ProtocolManager::createEmptyPackage( QByteArray &package )
{
    package.clear();
    appendHeader( package, true );
    appendFooter( package );
}

bool ProtocolManager::parsePackage( const QByteArray &package)
{
    bool isStatusPackage = false;
    QByteArray dataArray;
    if( package.at(0) != STX || package.at(package.size()-1) != ETX ){
        return false;
    } else {
        int dataStart = 0;
        int dataEND = 0;
        if( package.contains( TYPE ) ){
            dataStart = package.indexOf( TYPE );
            dataEND = package.indexOf( EOT )-1;
            if( dataStart != 6 ){
                qDebug() << "The position of field type is not correct. There is an error in the package frame.";
                return false;
            }
        } else {
            dataStart = RUNNING_NUMBER_POSITION;
            dataEND = package.indexOf( EOT )-1;
            isStatusPackage = true;
            if( package.size() != 11){
                qDebug() << "There is an error in the package frame.";
                return false;
            }
        }
        this->receivedPackageAddress = package.mid( ADDRESS_START_POSITION, ADDRESS_LENGTH );
        this->receivedPackageRunningNumber = package.at( RUNNING_NUMBER_POSITION );
        dataArray = package.mid( dataStart + 1, dataEND - dataStart );
        if( dataArray.isEmpty() ){
            qDebug() << "The package is empty with no data field.";
            emit emptyPackageReceived();
            return true;
        }

    }

    if( !isStatusPackage ){
        decodeCommand( dataArray );
        QList< QByteArray > commandsList = dataArray.split( SOH );
        if( !executeCommands( commandsList ) )
            return false;
    } else {
        if( !defineStatus( dataArray.at(0) ) )
            return false;
    }

    return true;
}

int ProtocolManager::getPackageFrameAddr(const QByteArray &package)
{
    return QString( package.mid( ADDRESS_START_POSITION, ADDRESS_LENGTH ) ).toInt();
}

bool ProtocolManager::isPackageContainsSecondAddr(const QByteArray &package)
{
    int cmd = getPackageFirstCommand(package);
    switch( cmd ){
        case TERMINAL_CLIENT_DATA_TO_CLIENT     :;
//        case TERMINAL_CLIENT_QUERY_CLIENT_INFO  :;
//        case CLIENT_TERMINAL_INFO               :;
        case CLIENT_TERMINAL_DATA_FROM_CLIENT   :return true; break;
        default: return false;
    }
}

bool ProtocolManager::isPackageContainsCustomCmd(const QByteArray &package)
{
    QString data = getDataArrayFromEncodedPackage( package, 0, 9 );
    if(data.contains(CMD))
        return true;
    else
        return false;
}

bool ProtocolManager::switchAddresses(const QByteArray &package, QByteArray &newPack)
{
    if( package.size() < 17 ){
        qDebug()<<"Can't switch the addresses!";
        return false;
    }
    QByteArray firstAddr = package.mid( 1, 4 );
    QByteArray secondAddr = package.mid( 8, 4);
    newPack = package;
    newPack.replace(1, secondAddr.size(), secondAddr);
    newPack.replace(8, firstAddr.size(), firstAddr);
    return true;
}

int ProtocolManager::getPackageFirstCommand(const QByteArray &package)
{
    QString command = getDataArrayFromEncodedPackage( package, 0, 1);
    if( command.isEmpty() )
        return 0;
    return command.at(0).unicode();
}

QString ProtocolManager::getPackageSecondAddress(const QByteArray &package)
{
    QString addr = getDataArrayFromEncodedPackage( package, 1, 4);
    if( addr.isEmpty() )
        return "";
    return addr;
}

QString ProtocolManager::getDataArrayFromEncodedPackage(const QByteArray &package, int startChar, int numberOfChars)
{
    QByteArray dataArray;
    int dataStart = 0;
    int dataEND = 0;
    if( package.contains( TYPE ) ){
        dataStart = package.indexOf( TYPE );
        dataEND = package.indexOf( EOT )-1;
    } else {
        return "";
    }
    dataArray = package.mid( dataStart + 1, dataEND - dataStart );
    decodeCommand( dataArray );
    return QString( dataArray.mid( startChar, numberOfChars ) );
}

//================================== COMMANDS CREATION ================================================

// TERMINAL COMMANDS
bool ProtocolManager::insertCommand_DFC( QByteArray &package, const QString &data, QString clientAddr, bool includeSOH )
{
    QByteArray command;
    if( includeSOH ){
        command.append( SOH );
    }
    char dataFromClient = CLIENT_TERMINAL_DATA_FROM_CLIENT;
    command.append( dataFromClient );

    if( !fixAddressString( clientAddr ) )
        return false;
    for( int i = 0; i < clientAddr.size(); i++){
        command.append( clientAddr.at(i) );
    }

    for( int i = 0; i < data.size(); i++){
        command.append( data.at(i) );
    }
    encodeCommand( command );

    insertCommandToPackage( package, command );
    return true;
}

bool ProtocolManager::insertCommand_DTC( QByteArray &package, const QString &data, QString clientAddr, bool includeSOH )
{
    QByteArray command;
    if( includeSOH ){
        command.append( SOH );
    }
    char dataToClient = TERMINAL_CLIENT_DATA_TO_CLIENT;
    command.append( dataToClient );

    if( !fixAddressString( clientAddr ) )
        return false;
    for( int i = 0; i < clientAddr.size(); i++){
        command.append( clientAddr.at(i) );
    }

    for( int i = 0; i < data.size(); i++){
        command.append( data.at(i) );
    }
    encodeCommand( command );

    insertCommandToPackage( package, command );
    return true;
}

void ProtocolManager::insertCommand_SetConfiguration(QByteArray &package, int reactionTimeOut, int charTimeOut, int pauseTime, int scanRate, int checkPollRate, int connectionRepeats, bool includeSOH)
{
    QByteArray command;
    appendSOH( command, includeSOH );
    char setConf = TERMINAL_CLIENT_SET_CONFIGURATION;
    command.append( setConf );

    appendIntToCommand( command, reactionTimeOut, 5, 20000, 100, 1000 );
    appendIntToCommand( command, charTimeOut, 5, 20000, 100, 300 );
    appendIntToCommand( command, pauseTime, 5, 20000, 100, 0 );

    appendIntToCommand( command, scanRate, 3, 255, 2, 250 );
    appendIntToCommand( command, checkPollRate, 3, 255, 2, 10 );
    appendIntToCommand( command, connectionRepeats, 3, 255, 0, 10 );

    encodeCommand( command );
    insertCommandToPackage( package, command );
}

void ProtocolManager::insertCommand_QueryConfiguration(QByteArray &package, bool includeSOH)
{
    QByteArray command;
    appendSOH( command, includeSOH );
    char queryConf = TERMINAL_CLIENT_SET_CONFIGURATION;
    command.append( queryConf );
    command.append('?');
    encodeCommand( command );
    insertCommandToPackage( package, command );
}

bool ProtocolManager::insertCommand_QueryClientInfo(QByteArray &package, QString clientAddr, bool includeSOH)
{
    QByteArray command;
    appendSOH( command, includeSOH );
    char queryClient = TERMINAL_CLIENT_QUERY_CLIENT_INFO;
    command.append( queryClient );

    if( !fixAddressString( clientAddr ) )
        return false;
    appendStrToCommand( command, clientAddr, 4 );

    encodeCommand( command );
    insertCommandToPackage( package, command );
    return true;
}

void ProtocolManager::insertCommand_CurrentConfiguration(QByteArray &package, int reactionTimeOut, int charTimeOut, int pauseTime, int scanRate, int checkPollRate, int connectionRepeats, bool includeSOH)
{
    QByteArray command;
    appendSOH( command, includeSOH );
    char sendConf = CLIENT_TERMINAL_SEND_CONFIGURATION;
    command.append( sendConf );

    appendIntToCommand( command, reactionTimeOut, 5, 20000, 100, 1000 );
    appendIntToCommand( command, charTimeOut, 5, 20000, 100, 300 );
    appendIntToCommand( command, pauseTime, 5, 20000, 100, 0 );

    appendIntToCommand( command, scanRate, 3, 255, 2, 250 );
    appendIntToCommand( command, checkPollRate, 3, 255, 2, 10 );
    appendIntToCommand( command, connectionRepeats, 3, 255, 0, 10 );

    encodeCommand( command );
    insertCommandToPackage( package, command );
}

bool ProtocolManager::insertCommand_ClientInfo(QByteArray &package, QString clientAddr, ClientStates state, bool includeSOH)
{
    QByteArray command;
    appendSOH( command, includeSOH );
    char clientInfo = CLIENT_TERMINAL_INFO;
    command.append( clientInfo );

    if( !fixAddressString( clientAddr ) )
        return false;
    appendStrToCommand( command, clientAddr, 4 );
    char st = (int) state;
    command.append( st );

    encodeCommand( command );
    insertCommandToPackage( package, command );
    return true;
}

// DISPLAY COMMANDS
void ProtocolManager::insertCommand_DisplaySetConf(QByteArray &package, QList<KeyState> keysState, int ledBlinkTime, int displayBlinkTime, bool includeSOH)
{
    QByteArray command;
    appendSOH( command, includeSOH );
    char set = TERMINAL_CLIENT_SET_CONFIGURATION;
    command.append( set );

    for( int i = 0; i < keysState.size(); i++){
        char ks = (int) keysState.at(i);
        command.append(ks);
    }

    appendIntToCommand(command, ledBlinkTime, 2, 99, 0, 0 );
    appendIntToCommand(command, displayBlinkTime, 2, 99, 0, 0 );

    encodeCommand( command );
    insertCommandToPackage( package, command );
}

bool ProtocolManager::insertCommand_Display(QByteArray &package, int lengthOfData, const QString &data, LightMode elModeRed, LightMode elModeGreen, LightMode elModeBlue, bool includeSOH)
{
    QByteArray command;
    appendSOH( command, includeSOH );
    char display = TERMINAL_DISPLAY_CHANGE_DEVICE_STATE;
    command.append( display );
    command.append( '0' ); // Type of the Display (always ‘0’, reserved for future use)‘0’: Standard Pick

    //Validate the length of data, and append the value in package
    QString dataLength = QString::number( lengthOfData );
    dataLength = dataLength.right( 2 );
    for( int i = dataLength.size(); i < 2; i++ ){
        dataLength.insert( 0, '0' );
    }
    command.append(dataLength.at(0));
    command.append(dataLength.at(1));

    if( dataLength.toInt() != data.size() )
        return false;

    appendStrToCommand( command, data );
    appendDotsToCommand( command, data.size() ); //Save bytes for DMODE
    appendDotsToCommand( command, data.size() ); //Save bytes for DPMODE
    appendDotsToCommand( command, 3 ); //Save bytes for LMODE

    //Mode of each external LED
    char red = (int) elModeRed;
    command.append(red);
    char green = (int) elModeGreen;
    command.append(green);
    char blue = (int) elModeBlue;
    command.append(blue);

    appendDotsToCommand( command, 1 ); //Save bytes for DWN
    appendDotsToCommand( command, 1 ); //Save bytes for UP
    appendDotsToCommand( command, 1 ); //Save bytes for ZC

    encodeCommand( command );
    insertCommandToPackage( package, command );
    return true;
}

void ProtocolManager::insertCommand_SpecialDisplay(QByteArray &package, SpecialDisplayType type, const QString &data, LightMode lModeRed, LightMode lModeGreen, LightMode lModeBlue, LightMode elModeRed, LightMode elModeGreen, LightMode elModeBlue, LightMode dwn, LightMode up, bool includeSOH)
{
    QByteArray command;
    appendSOH( command, includeSOH );
    char sDisplay = TERMINAL_DISPLAY_SPECIAL_DISPLAY;
    command.append( sDisplay );

    char t = (int) type;
    command.append( t );

    appendStrToCommand( command, data );

    //Mode of each internal LED
    char redI = (int) lModeRed;
    command.append(redI);
    char greenI = (int) lModeGreen;
    command.append(greenI);
    char blueI = (int) lModeBlue;
    command.append(blueI);

    //Mode of each external LED
    char redE = (int) elModeRed;
    command.append(redE);
    char greenE = (int) elModeGreen;
    command.append(greenE);
    char blueE = (int) elModeBlue;
    command.append(blueE);

    //Mode of up and dwn LED
    char d = (int) dwn;
    command.append(d);
    char u = (int) up;
    command.append(u);

    encodeCommand( command );
    insertCommandToPackage( package, command );
}

void ProtocolManager::insertCommand_Scan(QByteArray &package, ScanType type, int scanningTime, bool includeSOH)
{
    QByteArray command;
    appendSOH( command, includeSOH );
    char scan = TERMINAL_DISPLAY_SCAN;
    command.append( scan );

    char t = (int) type;
    command.append(t);
    appendIntToCommand(command, scanningTime, 3, 999, 0, 0 );

    encodeCommand( command );
    insertCommandToPackage( package, command );
}

void ProtocolManager::insertCommand_DisableDisplay(QByteArray &package, DisplayMode mode, bool includeSOH)
{
    QByteArray command;
    appendSOH( command, includeSOH );
    char display = TERMINAL_DISPLAY_DISABLE_DISPLAY;
    command.append( display );

    char m = (int) mode;
    command.append(m);

    encodeCommand( command );
    insertCommandToPackage( package, command );
}

void ProtocolManager::insertCommand_KeyMessage(QByteArray &package, QList< KeyState > state, int keyTime, int timeSince, bool includeSOH)
{
    QByteArray command;
    appendSOH( command, includeSOH );
    char keyMessage = DISPLAY_TERMINAL_KEY_MESSAGE;
    command.append( keyMessage );

    for(int i = 0; i < state.size(); i++){
        char st = (int) state.at(i);
        command.append(st);
    }

    appendIntToCommand(command, keyTime, 3, 999, 1, 15);
    appendIntToCommand(command, timeSince, 3, 999, 1, 15);

    encodeCommand( command );
    insertCommandToPackage( package, command );
}

void ProtocolManager::insertCommand_ScannedMessage(QByteArray &package, ScanType type, BarcodeState state, const QString &scannedData, bool includeSOH)
{
    QByteArray command;
    appendSOH( command, includeSOH );
    char scannedMessage = DISPLAY_TERMINAL_SCANNED_MESSAGE;
    command.append( scannedMessage );

    char t = (int) type;
    command.append(t);

    char st = (int) state;
    command.append(st);

    appendStrToCommand( command, scannedData );

    encodeCommand( command );
    insertCommandToPackage( package, command );
}

// CLIENT COMMANDS
void ProtocolManager::insertCommand_ClientReset(QByteArray &package, ClientResetStates state, bool includeSOH)
{
    QByteArray command;
    appendSOH( command, includeSOH );
    char reset = HOST_CLIENT_RESET;
    command.append( reset );

    char st = (int) state;
    command.append(st);

    encodeCommand( command );
    insertCommandToPackage( package, command );
}

void ProtocolManager::insertCommand_ClientNewRestart(QByteArray &package, ClientNewRestartStates state, bool includeSOH)
{
    QByteArray command;
    appendSOH( command, includeSOH );
    char newRestart = CLIENT_HOST_NEW_RESTART;
    command.append( newRestart );

    char st = (int) state;
    command.append(st);
    command.append('0');

    encodeCommand( command );
    insertCommandToPackage( package, command );
}

void ProtocolManager::insertCommand_ClientResetDone(QByteArray &package, QString hardwareV, QString softwareV, QString clientId, bool includeSOH)
{
    QByteArray command;
    appendSOH( command, includeSOH );
    char resetDone = CLIENT_HOST_RESET_DONE;
    command.append( resetDone );

    appendStrToCommand( command, hardwareV, 2 );
    command.append( "00" );
    appendStrToCommand( command, softwareV, 2 );
    command.append( "00" );
    appendStrToCommand( command, clientId, 4 );
    command.append( "0000" );

    encodeCommand( command );
    insertCommandToPackage( package, command );
}

void ProtocolManager::insertCommand_ClientErrorMessage(QByteArray &package, int errorNumber, int errorSpecs, bool includeSOH)
{
    QByteArray command;
    appendSOH( command, includeSOH );
    char errorMessage = CLIENT_HOST_ERROR_MESSAGE;
    command.append( errorMessage );

    appendIntToCommand( command, errorNumber, 2, 99, 0, 0 );
    appendIntToCommand( command, errorSpecs, 2, 99, 0, 0 );

    encodeCommand( command );

    insertCommandToPackage( package, command );
}


//======================================================================================================
//================================== COMMANDS EXECUTION ================================================

// TERMINAL COMMANDS

void ProtocolManager::execCommand_DFC(const QByteArray &command)
{
    if(command.length() < 5) {
        qWarning() << "execCommand_DFC() - Not a valid command length!";
        return;
    }
    char cmd = command.at(0);
    QString addr = QString( command.mid( 1, 4 ) );
    QByteArray data = command.mid( 5, command.size() - 5 );

    QString textData = QString( data );
    //qDebug() << cmd << addr << textData ;

    if( isDataContainsCommand( data ) ){
        QList<QByteArray> commands;
        commands.append(data);
        executeCommands(commands);
    } else {
        emit executed_DFC( addr, textData );
    }
}

void ProtocolManager::execCommand_DTC(const QByteArray &command)
{
    if(command.length() < 5) {
        qWarning() << "execCommand_DTC() - Not a valid command length!";
        return;
    }
    char cmd = command.at(0);
    QString addr = QString( command.mid( 1, 4 ) );
    QByteArray data = command.mid( 5, command.size() - 5 );

    QString textData = QString(data);
    //qDebug() << cmd << addr << textData ;

    if( isDataContainsCommand( data ) ){
        QList<QByteArray> commands;
        commands.append(data);
        executeCommands(commands);
    } else {
        emit executed_DTC( addr, textData );
    }
}

void ProtocolManager::execCommand_SetConfiguration(const QByteArray &command)
{
    if(command.length() < 24) {
        qWarning() << "execCommand_SetConfiguration() - Not a valid command length!";
        return;
    }
    char cmd = command.at(0);
    QString rto = QString( command.mid( 1, 5 ) );
    checkValueForDots(rto);
    QString cto = QString( command.mid( 6, 5 ) );
    checkValueForDots(cto);
    QString pp = QString( command.mid( 11, 5 ) );
    checkValueForDots(pp);
    QString sr = QString( command.mid( 16, 3 ) );
    checkValueForDots(sr);
    QString cpr = QString( command.mid( 19, 3 ) );
    checkValueForDots(cpr);
    QString mr = QString( command.mid( 22, 3) );
    checkValueForDots(mr);
    qDebug() << cmd << rto << cto<< pp<< sr<< cpr<< mr;
    emit executed_SetConfiguration( rto.toInt(), cto.toInt(), pp.toInt(), sr.toInt(), cpr.toInt(), mr.toInt() );
}

void ProtocolManager::execCommand_QueryConfiguration(const QByteArray &command)
{
    if(command.length() < 2) {
        qWarning() << "execCommand_QueryConfiguration() - Not a valid command length!";
        return;
    }
    char cmd = command.at(0);
    char questionMark = command.at(1);
    qDebug() << cmd << questionMark ;
    emit executed_QueryConfiguration();
}

void ProtocolManager::execCommand_QueryClientInfo(const QByteArray &command)
{
    if(command.length() < 5) {
        qWarning() << "execCommand_QueryClientInfo() - Not a valid command length!";
        return;
    }
    char cmd = command.at(0);
    QString addr = QString( command.mid( 1, 4 ) );
    qDebug() << cmd << addr ;
    emit executed_QueryClientInfo( addr );
}

void ProtocolManager::execCommand_CurrentConfiguration(const QByteArray &command)
{
    if(command.length() < 24) {
        qWarning() << "execCommand_CurrentConfiguration() - Not a valid command length!";
        return;
    }
    char cmd = command.at(0);
    QString rto = QString( command.mid( 1, 5 ) );
    QString cto = QString( command.mid( 6, 5 ) );
    QString pp = QString( command.mid( 11, 5 ) );
    QString sr = QString( command.mid( 16, 3 ) );
    QString cpr = QString( command.mid( 19, 3 ) );
    QString mr = QString( command.mid( 22, 3) );
    qDebug() << cmd << rto << cto<< pp<< sr<< cpr<< mr;
    emit executed_CurrentConfiguration( rto.toInt(), cto.toInt(), pp.toInt(), sr.toInt(), cpr.toInt(), mr.toInt() );
}

void ProtocolManager::execCommand_ClientInfo(const QByteArray &command)
{
    if(command.length() < 6) {
        qWarning() << "execCommand_ClientInfo() - Not a valid command length!";
        return;
    }
    char cmd = command.at(0);
    QString addr = QString( command.mid( 1, 4 ) );
    char state = command.mid( 5, 1 ).at(0) ;
    ClientStates st = static_cast<ClientStates>( ( (int) state ) );
    qDebug() << cmd << addr << state ;
    emit executed_ClientInfo( addr, st );
}

// DISPLAY COMMANDS

void ProtocolManager::execCommand_DisplaySetConf(const QByteArray &command)
{
    if(command.length() < 7) {
        qWarning() << "execCommand_DisplaySetConf() - Not a valid command length!";
        return;
    }
    char cmd = command.at(0);

    QString lbt = QString( command.mid( command.size() - 4, 2 ) );
    QString dbt = QString( command.mid( command.size() - 2, 2 ) );

    QByteArray buttonsStatus = command.mid( 1, command.size() - 5 );
    QList< KeyState > keysStatus;
    for( int i = 0; i < buttonsStatus.size(); i++ ){
        char ks = buttonsStatus.at(i);
        keysStatus << static_cast<KeyState>( ( (int) ks ) );
    }

    qDebug() << cmd << lbt << dbt;
    emit executed_DisplaySetConf( keysStatus, lbt.toInt(), dbt.toInt() );
}

void ProtocolManager::execCommand_Display(const QByteArray &command)
{
    if(command.length() < 13) {
        qWarning() << "execCommand_Display() - Not a valid command length!";
        return;
    }
    char cmd = command.at(0);

//    char t = command.at(1); //Not used type
    //Get the length of data
    QString dataLength = QString( command.mid( 2, 2 ));
    int length = dataLength.toInt();
    if(command.length() < (13 + ( length*3 ) ) ) {
        qWarning() << "execCommand_Display() - Not a valid command length!";
        return;
    }

    QString data = QString( command.mid( 4, length ));
    QString dMode = QString( command.mid( ( 4 + length ), length ));
    QString dpMode = QString( command.mid( ( 4 + ( 2 * length ) ), length ));
    int lastDataCharPosition = ( 4 + ( 3 * length ) )  - 1;

    char lRed = command.at( lastDataCharPosition + 1 );
    LightMode lRedMode = static_cast<LightMode>( ( (int) lRed ) );
    char lGreen = command.at(lastDataCharPosition + 2);
    LightMode lGreenMode = static_cast<LightMode>( ( (int) lGreen ) );
    char lBlue = command.at(lastDataCharPosition + 3);
    LightMode lBlueMode = static_cast<LightMode>( ( (int) lBlue ) );

    char eRed = command.at(lastDataCharPosition + 4);
    LightMode eRedMode = static_cast<LightMode>( ( (int) eRed ) );
    char eGreen = command.at(lastDataCharPosition + 5);
    LightMode eGreenMode = static_cast<LightMode>( ( (int) eGreen ) );
    char eBlue = command.at(lastDataCharPosition + 6);
    LightMode eBlueMode = static_cast<LightMode>( ( (int) eBlue ) );

    char dwn = command.at(lastDataCharPosition + 7);
    LightMode dwnMode = static_cast<LightMode>( ( (int) dwn ) );
    char up = command.at(lastDataCharPosition + 8);
    LightMode upMode = static_cast<LightMode>( ( (int) up ) );

    char zc = command.at(lastDataCharPosition + 9);
    LightMode zcMode = static_cast<LightMode>( ( (int) zc ) );

    qDebug() << cmd << data << dMode << dpMode << lRed << lGreen << lBlue << eRed << eGreen << eBlue << dwn << up << zc ;
    emit executed_Display( data, dMode, dpMode, lRedMode, lGreenMode, lBlueMode, eRedMode, eGreenMode, eBlueMode, dwnMode, upMode, zcMode );
}

void ProtocolManager::execCommand_SpecialDisplay(const QByteArray &command)
{
    if(command.length() < 10) {
        qWarning() << "execCommand_SpecialDisplay() - Not a valid command length!";
        return;
    }
    char cmd = command.at(0);

    char t = command.at(1);
    SpecialDisplayType displayType = static_cast<SpecialDisplayType>( ( (int) t ) );

    QString data = QString( command.mid( 2, command.size() - 10 ) );
    int lastDataCharPosition = ( 2 + ( command.size() - 10 ) ) - 1;

    char iRed = command.at( lastDataCharPosition + 1 );
    LightMode iRedMode = static_cast<LightMode>( ( (int) iRed ) );
    char iGreen = command.at(lastDataCharPosition + 2);
    LightMode iGreenMode = static_cast<LightMode>( ( (int) iGreen ) );
    char iBlue = command.at(lastDataCharPosition + 3);
    LightMode iBlueMode = static_cast<LightMode>( ( (int) iBlue ) );

    char eRed = command.at(lastDataCharPosition + 4);
    LightMode eRedMode = static_cast<LightMode>( ( (int) eRed ) );
    char eGreen = command.at(lastDataCharPosition + 5);
    LightMode eGreenMode = static_cast<LightMode>( ( (int) eGreen ) );
    char eBlue = command.at(lastDataCharPosition + 6);
    LightMode eBlueMode = static_cast<LightMode>( ( (int) eBlue ) );

    char dwn = command.at(lastDataCharPosition + 7);
    LightMode dwnMode = static_cast<LightMode>( ( (int) dwn ) );
    char up = command.at(lastDataCharPosition + 8);
    LightMode upMode = static_cast<LightMode>( ( (int) up ) );

    qDebug() << cmd << data << iRed << iGreen << iBlue << eRed << eGreen << eBlue << dwn << up ;
    emit executed_SpecialDisplay( displayType, data, iRedMode, iGreenMode, iBlueMode, eRedMode, eGreenMode, eBlueMode, dwnMode, upMode );
}

void ProtocolManager::execCommand_Scan(const QByteArray &command)
{
    if(command.length() < 5) {
        qWarning() << "execCommand_Scan() - Not a valid command length!";
        return;
    }
    char cmd = command.at(0);

    char t = command.at(1);
    ScanType type = static_cast<ScanType>( ( (int) t ) );

    QString act = QString( command.mid( 2, 3 ) );

    qDebug() << cmd << t << act ;
    emit executed_Scan( type, act.toInt() );
}

void ProtocolManager::execCommand_DisableDisplay(const QByteArray &command)
{
    if(command.length() < 2) {
        qWarning() << "execCommand_DisableDisplay() - Not a valid command length!";
        return;
    }
    char cmd = command.at(0);

    char m = command.at(1);
    DisplayMode mode = static_cast<DisplayMode>( ( (int) m ) );

    qDebug() << cmd << m ;
    emit executed_DisableDisplay( mode );
}

void ProtocolManager::execCommand_KeyMessage(const QByteArray &command)
{
    if(command.length() < 7) {
        qWarning() << "execCommand_KeyMessage() - Not a valid command length!";
        return;
    }
    char cmd = command.at(0);

    int timeSince = QString( command.mid( command.size() - 3, 3) ).toInt();
    int keyTime = QString( command.mid( command.size() - 6, 3) ).toInt();
    QByteArray keyStates;
    keyStates.append( command.mid(  1, ( command.size() - 7)  ) );
    QList<KeyState> keyStatusList;
    QString list = "";
    for(int i = 0; i< keyStates.size(); i++){
        char ks = keyStates.at(i);
        list.append( ks );
        keyStatusList.append( static_cast<KeyState>( ( (int) ks ) ) );
    }

    qDebug() << cmd << list << keyTime << timeSince ;
    emit executed_KeyMessage( keyStatusList, keyTime, timeSince );
}

void ProtocolManager::execCommand_ScannedMessage(const QByteArray &command)
{
    if(command.length() < 3) {
        qWarning() << "execCommand_ScannedMessage() - Not a valid command length!";
        return;
    }
    char cmd = command.at(0);

    char st = command.at(1);
    ScanType scanT = static_cast<ScanType>( ( (int) st ) );
    char bs = command.at(2);
    BarcodeState barcodeS = static_cast<BarcodeState>( ( (int) bs ) );
    QString data = "";
    if( ( command.size() - 3 ) > 0 )
        data = QString( command.mid( 3, ( command.size() - 3 ) ) );

    qDebug() << cmd << st << bs << data ;
    emit executed_ScannedMessage( scanT, barcodeS, data );
}

// CLIENT COMMANDS

void ProtocolManager::execCommand_ClientReset(const QByteArray &command)
{
    if(command.length() < 2) {
        qWarning() << "execCommand_ClientReset() - Not a valid command length!";
        return;
    }
    char cmd = command.at(0);
    char state = command.at(1);
    ClientResetStates st = static_cast<ClientResetStates>( ( (int) state ) );
    qDebug() << cmd << state ;
    emit executed_ClientReset( st );
}

void ProtocolManager::execCommand_ClientNewRestart(const QByteArray &command)
{
    if(command.length() < 3) {
        qWarning() << "execCommand_ClientNewRestart() - Not a valid command length!";
        return;
    }
    char cmd = command.at(0);
    char state = command.at(1);
    ClientNewRestartStates st = static_cast<ClientNewRestartStates>( ( (int) state ) );
    qDebug() << cmd << state ;
    emit executed_ClientNewRestart( st );
}

void ProtocolManager::execCommand_ClientResetDone(const QByteArray &command)
{
    if(command.length() < 17) {
        qWarning() << "execCommand_ClientResetDone() - Not a valid command length!";
        return;
    }
    char cmd = command.at(0);
    QString hV = QString( command.mid( 1, 4 ) );
    QString sV = QString( command.mid( 5, 4 ) );
    QString id = QString( command.mid( 9, 8 ) );
    qDebug() << cmd << hV << sV << id ;
    emit executed_ClientResetDone( hV, sV, id );
}

void ProtocolManager::execCommand_ClientErrorMessage(const QByteArray &command)
{
    if(command.length() < 5) {
        qWarning() << "execCommand_ClientErrorMessage() - Not a valid command length!";
        return;
    }
    char cmd = command.at(0);
    QString errN = QString( command.mid( 1, 2 ) );
    QString errS = QString( command.mid( 3, 2 ) );
    qDebug() << cmd << errN << errS ;
    emit executed_ClientErrorMessage( errN.toInt(), errS.toInt() );
}


//======================================================================================================

