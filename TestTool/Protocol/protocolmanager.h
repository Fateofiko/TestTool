/**
  * @file
  * @brief Protocol for communication between clients and host. Documented in 'Interface iWACS - PickCenter Touch_V002'
  * */
#ifndef PROTOCOLMANAGER_H
#define PROTOCOLMANAGER_H

#include <QObject>
#include <QVariant>
#include <QDebug>

#define ASCII_NULL 0x00
#define SOH 0x01            ///< Separator for the commands in the data section of the protocol
#define STX 0x02            ///< Start byte
#define ETX 0x03            ///< Termination byte
#define EOT 0x04            ///< End of transmission byte
#define ENCODING_STEP 0x40  ///< Encoding the characters in the data field of the protocol
#define APPEND_NEXT_COMMAND_PACKAGE_POSITION 4  ///< Append next command at the end of the package
#define RUNNING_NUMBER_POSITION 5               ///< Position of running number field
#define ADDRESS_START_POSITION 1                ///< Start position of IP address field
#define ADDRESS_LENGTH 4                        ///< 4 bytes for IP address

//Custom text commands
/**
  * 'CMD' at the begining of data field in 'Data to client' and 'Data from client' commands
  * indicates a custom command that is not contained in the list of commands
  * in 'Interface iWACS - PickCenter Touch_V002' document
  */
#define CMD "CMD"
#define FIELD_STATUS "STATUS"       ///< Status field in a custom CMD command from the Host to accept or reject client registration.
#define FIELD_ID "ID"               ///< Client id in a custom CMD command.
#define FIELD_LIGHT "LIGHT"         ///< Light field in a custom CMD command to change client settings.
#define FIELD_SCANNER "SCANNER"     ///< Scanner field in a custom CMD command to change client settings.

#define DATA_CMD_HEARTBEAT "HEARTBEAT"
#define DATA_CMD_REG "REG"                          ///< Custom command from a client to the Host to initiate registration process.
#define DATA_CMD_REG_DONE "REG_DONE"                ///< Custom command from a client to the Host.
#define DATA_CMD_REG_RESULT "REG_RESULT"            ///< Custom command from the Host to a client with registration status.
#define DATA_CMD_CHANGE_SETTINGS "CHANGE_SETTINGS"  ///< Custom command to change light and scanner settings.
#define DATA_STATUS_OK_RESTART "OK_R"               ///< Status data from the Host to a client when accepting registration.
#define DATA_STATUS_OK "OK"                         ///< Status data from the Host to a client when accepting registration.
#define DATA_STATUS_FAILURE "FAIL"                  ///< Status data from the Host to a client when rejecting registration.

static const char TYPE = 'D';
static const char FIRST_BYTE_FOR_NOCRC = 'N';
static const char SECOND_BYTE_FOR_NOCRC = 'C';
static const QString DEFAULT_ADDRESS = "0000";
static const char DEFAULT_RUNNING_NUMBER = '0';

//Helpfull struct for led handling if the logic changes
//struct LedColor
//{
//    LedColor(){
//        setAllColorsToNull();
//    }

//    LedColor( int red, int green, int blue )
//    {
//        if( ( red < 0 || red > 255 ) || ( green < 0 || green > 255 ) || ( blue < 0 || blue > 255 ) ){
//            setAllColorsToNull();
//        } else {
//            this->red = red;
//            this->green = green;
//            this->blue = blue;
//        }
//    }

//    void setAllColorsToNull()
//    {
//        this->red = NULL;
//        this->green = NULL;
//        this->blue = NULL;
//    }

//    unsigned char red;
//    unsigned char green;
//    unsigned char blue;

//};

/**
 * @brief Protocol states
 */
enum ProtocolStates
{
    OK = 48,                                  ///< ‘0’ Message ok and accepted
    NO_RECEIVER = 50,                         ///< ‘2’ No buffer on receiver side. The sender has to repeat the message with the same data but with a new running number R.
    A_LOT_OF_MESSAGES_IN_SINGLE_PACKAGE = 55, ///< ‘7’ more than 50 messages in one frame, some messages are lost.
    SAME_RUNNING_NUMBER = 56                  ///< ‘8’ Message with the same running number as before received
};

/**
 * @brief Client states
 */
enum ClientStates
{
    CLIENT_DISAPPEARED = 48,         ///< ‘0’: client disappeared
    CLIENT_PRESENT = 49,             ///< ‘1’: client present
    CLIENT_NOT_PRESENT = 57          ///< ‘9’: addressed client not present
};

/**
 * @brief Client reset states
 */
enum ClientResetStates
{
    CLIENT_NORMAL_OPERATION = 48,                   ///< ‘0’: normal operation
    CLIENT_ENABLE_MULTIPLE_MESSAGES = 49,           ///< ‘1’: enables multiple messages per frame for the non-polled protocol.
    CLIENT_ACTIVATE_BOOT_LOADER = 57                ///< ‘9’: activates the boot loader.
};

/**
 * @brief Client new restart states
 */
enum ClientNewRestartStates
{
    CLIENT_POWER_ON = 48,                           ///< ‘0’ Power On
    CLIENT_SOFTWARE_RESET = 55,                     ///< ‘7’ Software Reset
    CLIENT_UNKNOWN_REASON = 57                      ///< ‘9’ unknown reason
};

/**
 * @brief Key states
 */
enum KeyState
{
    CLIENT_KEY_INACTIVE = 48,                       ///< ‘0’: Key is released (inactive)
    CLIENT_KEY_PRESSED = 49,                        ///< ‘1’: Key is pressed
    CLIENT_KEY_HELD_PRESSED = 50,                   ///< ‘2’: Key held pressed
    CLIENT_KEY_RELEASED = 51                        ///< ‘3’: Key released
};

/**
 * @brief Keys
 */
enum Key
{
    CLIENT_KEY_CONFIRM = 48,                        ///< ‘0’: confirm
    CLIENT_KEY_PLUS = 49,                           ///< ‘1’: +
    CLIENT_KEY_MINUS = 50,                          ///< ‘2’: -
    CLIENT_KEY_FUNCTION = 51                        ///< ‘3’: function
};

/**
 * @brief Scan types
 */
enum ScanType
{
    SCAN_AUTOMATIC = 48,                            ///< ‘0’: automatic scan
    SCAN_MANUAL = 49                               ///< ‘1’: manual scan
};

/**
 * @brief Barcode scan states
 */
enum BarcodeState
{
    SUCCESSFULLY_SCANNED = 48,                    ///< ‘0’ Barcode successfully scanned
    TIMEOUT = 53,                                 ///< ‘5‘ Timeout (ACT time exceeded)
    CAMERA_NOT_AVAILABLE = 54,                    ///< ‘6’ Camera not available/connected
    CAMERA_INTERNAL_ERROR = 55,                   ///< ‘7’ Camera internal error
    OCR_ERROR = 56,                               ///< ‘8’ OCR Error (too dark, ...)
    UNSPECIFIED_ERROR = 57                        ///< ‘9‘ Unspecified Error
};

/**
 * @brief Display modes
 */
enum DisplayMode
{
    SCAN_NORMAL = 48,                            ///< ‘0’: normal operational mode
    SCAN_SPECIAL_PATTERN = 49                    ///< ‘1’: the display shows a special pattern, which shows the user that a special condition exists where he is not allowed to work with this display.
};

/**
 * @brief Light modes
 */
enum LightMode
{
    LIGHT_OFF = 48,                            ///< ‘0’: The digit/light/LED is switched off.
    LIGHT_ON = 49,                             ///< ‘1’: The digit/light/LED is switched on.
    LIGHT_BLINKING = 50,                       ///< ‘2’: The digit/light/LED is blinking
    LIGHT_OPPOSITE_BLINKING = 51               ///< ‘3’: The digit/light/LED is blinking with the opposite phase as a digit in mode ‘2’.
};

/**
 * @brief Special display types
 */
enum SpecialDisplayType
{
    DISPLAY_EMPTY_CARTON = 48,              ///< ‘0’: Empty carton
    DISPLAY_CARTON_EVACUATION = 49,         ///< ‘1’: Carton evacuation
    DISPLAY_ERROR = 90                      ///< ‘Z‘: Error display
};

/**
 * @brief Commands
 * All commands are represented by decimal values of thear chars in the ASCII table.
 *
 * TERMINAL COMMANDS
    Direction: iWACS <-> Terminal
    Right (from iWACS to Terminal), Left (from Terminal to iWACS)
    Overview Telegrams:
    CMD  DIR   NAME
    V    R     Set Configuration
    V?   R     Query Configuration
    >    R     Data to client
    I    R     Query Client Info
    v    L     Configuration
    i    L     Client Info
    <    L     Data from client

DISPLAY COMMANDS
    Direction: Terminal <-> Display
    Right (from Terminal to Display), Left (from Display to Terminal)
    Overview Telegrams:
    CMD  DIR  NAME
    V    R    Set Configuration
    E    R    Display
    F    R    Special Display
    S    R    Query Scan
    Z    R    Disable Display
    k    L    Key Message
    s    L    Scanned Message

CLIENT COMMANDS
    A Client can be either a terminal or one of the components it manages. (Display, Led, Camera). Client
    commands are used to initialize the connection between the different systems.
    The general flow of operation is the following:
    The client has done a new restart of the firmware (e.g. power was cut) and is now continuously sending the
    ‘n’ message until the client receives a ‘R’ command from the host. After the ‘R’ command is received it is
    answered by an ‘r’ command.
    Direction: Host <-> Client
    Right (from Host to Client), Left (from Client to Host)
    Overview Telegrams:
    CMD  DIR  NAME
    R    R    Reset
    n    L    New Restart
    r    L    Reset Done
    f    L    Error Message

 * WARNINGS!!!
 * QUERY_CONFIGURATION and TERMINAL_DISPLAY_SET_CONFIGURATION_TO_DISPLAY starts with same simbol as SET_CONFIGURATION,
 * you can use SET_CONFIGURATION number for the three commands, but to distinguish them use the size of each command,
 * the size is fix number of bytes and it is unique for each command.
 */
enum Commands
{
//TERMINAL COMMANDS

    TERMINAL_CLIENT_SET_CONFIGURATION = 86,   ///< 'V' - Set Configuration
//    QUERY_CONFIGURATION = 86,   ///< 'V?' Query Configuration
    TERMINAL_CLIENT_DATA_TO_CLIENT = 62,   ///< '>' Data to client
    TERMINAL_CLIENT_QUERY_CLIENT_INFO = 73,   ///<  'I' Query Client Info
    CLIENT_TERMINAL_SEND_CONFIGURATION = 118,   ///<  'v' Configuration
    CLIENT_TERMINAL_INFO = 105,   ///< 'i' Client Info
    CLIENT_TERMINAL_DATA_FROM_CLIENT = 60,   ///< '<' Data from client

//DISPLAY COMMANDS

//    TERMINAL_DISPLAY_SET_CONFIGURATION_TO_DISPLAY = 86,   ///< Set Configuration
    TERMINAL_DISPLAY_CHANGE_DEVICE_STATE = 69,   ///<  'E' Display
    TERMINAL_DISPLAY_SPECIAL_DISPLAY = 70,   ///< 'F' Special Display
    TERMINAL_DISPLAY_SCAN = 83,   ///< 'S' Query Scan
    TERMINAL_DISPLAY_DISABLE_DISPLAY = 90,   ///< 'Z' Disable Display
    DISPLAY_TERMINAL_KEY_MESSAGE = 107,   ///< 'k' Key Message
    DISPLAY_TERMINAL_SCANNED_MESSAGE = 115,   ///< 's' Scanned Message

//CLIENT COMMANDS

    HOST_CLIENT_RESET = 82,   ///< 'R' Reset
    CLIENT_HOST_NEW_RESTART = 110,   ///< 'n' New Restart
    CLIENT_HOST_RESET_DONE = 114,   ///< 'r' Reset Done
    CLIENT_HOST_ERROR_MESSAGE = 102   ///< 'f' Error Message

};

/**
 * @brief The ProtocolManager class is used for communication between clients, Host and iWACS.
 * The protocol is documented in 'Interface iWACS - PickCenter Touch_V002'.
 */
class ProtocolManager : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Default constructor
     * @param parent
     */
    explicit ProtocolManager(QObject *parent = 0);

    /**
     * @brief Decodes a command.
     * @param command Received command bytes.
     */
    void decodeCommand( QByteArray &command );

    /**
     * @brief Creates empty package.
     *
     * Creates empty package with header, default address, default running number and package footer.
     * The package don't contains commands.
     *
     * @param package Bytes to append package data
     */
    void createEmptyPackage(QByteArray &package);

    /**
     * @brief Creates status package to inform for the client status
     * @param package Bytes to append package data.
     * @param state Protocl state
     */
    void createStatusPackage( QByteArray &package, ProtocolStates state );

    /**
     * @brief Sets address
     * @param addr Four char string. Default is "0000"
     */
    void setProtocolAddress( QString addr );

    /**
     * @brief Sets running number
     * @param number
     */
    void setRunningNumber( char number = '0' );

    /**
     * @brief Parses a package
     * @param package received bytes
     * @return true on success
     */
    bool parsePackage( const QByteArray &package );
    /**
     * @brief getPackageFrameAddr
     * @param package
     * @return
     */
    int getPackageFrameAddr( const QByteArray &package );
    /**
     * @brief getPackageFirstCommand
     * @param package
     * @return
     */
    int getPackageFirstCommand( const QByteArray &package );
    /**
     * @brief getPackageSecondAddress
     * @param package
     * @return
     */
    QString getPackageSecondAddress( const QByteArray &package );
    /**
     * @brief getDataFromPackage
     * @param package
     * @return
     */
    QString getDataArrayFromEncodedPackage( const QByteArray &package, int startChar, int numberOfChars );

    /**
     * @brief isPackageContainsSecondAddr
     * @param package
     * @return
     */
    bool isPackageContainsSecondAddr(const QByteArray &package);

    // TERMINAL COMMANDS
    /**
     * @brief Constructs and inserts "Data from client" command into a package.
     *
     * This command is for sending text from client to host.
     * You can use this command to send custom text commands with data to host.
     * (Example: QString data = " CHECK_MY_ID( or just and enum number for the command ) : client position; LIGHT_SOURCE : true; ... next commands ...";)
     *
     * @param package Package bytes to insert constructed command.
     * @param data Command data
     * @param clientAddr Client address
     * @param includeSOH Set includeSOH to true , when you want to insert more than one command to empty package.
     * @return false if the clientAddr is not valid
     * @see execCommand_DFC()
     * @see executed_DFC()
     */
    bool insertCommand_DFC(QByteArray &package, const QString &data, QString clientAddr, bool includeSOH );

    /**
     * @brief Constructs and inserts "Data to client" command into a package.
     *
     * This command is for sending text to a client.
     * You can use this command to send custom text commands with data to client.
     *
     * @param package Package bytes to insert constructed command.
     * @param data Command data
     * @param clientAddr Client address
     * @param includeSOH Set includeSOH to true , when you want to insert more than one command to empty package.
     * @return false if the clientAddr is not valid
     * @see execCommand_DTC()
     * @see executed_DTC()
     */
    bool insertCommand_DTC(QByteArray &package, const QString &data, QString clientAddr, bool includeSOH );

    /**
     * @brief Constructs and inserts "Set configuration" command into a package.
     *
     * This command if for setting client configuration.
     * .
     * @param package Package bytes to insert constructed command.
     * @param reactionTimeOut
     * @param charTimeOut
     * @param pauseTime
     * @param scanRate
     * @param checkPollRate
     * @param connectionRepeats
     * @param includeSOH Set includeSOH to true , when you want to insert more than one command to empty package.
     * @see execCommand_SetConfiguration()
     * @see executed_SetConfiguration()
     */
    void insertCommand_SetConfiguration( QByteArray &package, int reactionTimeOut, int charTimeOut, int pauseTime, int scanRate, int checkPollRate, int connectionRepeats, bool includeSOH );

    /**
     * @brief Constructs and inserts "Query configuration" command into a package.
     *
     *
     * @param package Package bytes to insert constructed command.
     * @param includeSOH Set includeSOH to true , when you want to insert more than one command to empty package.
     * @see execCommand_QueryConfiguration()
     * @see executed_QueryConfiguration()
     */
    void insertCommand_QueryConfiguration( QByteArray &package, bool includeSOH );

    /**
     * @brief Constructs and inserts "Query client info" command into a package.
     *
     * This command is to query the state of a client with a distinct address.
     *
     * @param package Package bytes to insert constructed command.
     * @param clientAddr Address of the client
     * @param includeSOH Set includeSOH to true , when you want to insert more than one command to empty package.
     * @return
     * @see execCommand_QueryClientInfo()
     * @see executed_QueryClientInfo()
     */
    bool insertCommand_QueryClientInfo( QByteArray &package, QString clientAddr, bool includeSOH );

    /**
     * @brief Constructs and inserts "Current configuration" command into a package.
     *
     * his command gives information about client configuration.
     *
     * @param package Package bytes to insert constructed command.
     * @param reactionTimeOut
     * @param charTimeOut
     * @param pauseTime
     * @param scanRate
     * @param checkPollRate
     * @param connectionRepeats
     * @param includeSOH Set includeSOH to true , when you want to insert more than one command to empty package.
     * @see insertCommand_SetConfiguration()
     * @see execCommand_CurrentConfiguration()
     * @see executed_CurrentConfiguration()
     */
    void insertCommand_CurrentConfiguration( QByteArray &package, int reactionTimeOut, int charTimeOut, int pauseTime, int scanRate, int checkPollRate, int connectionRepeats, bool includeSOH );

    /**
     * @brief Constructs and inserts "Client info" command into a package.
     *
     * This command gives client information.
     *
     * @param package Package bytes to insert constructed command.
     * @param clientAddr Address of the client
     * @param state Client state
     * @param includeSOH Set includeSOH to true , when you want to insert more than one command to empty package.
     * @return false if clientAddr is not valid
     * @see insertCommand_QueryClientInfo()
     * @see execCommand_ClientInfo()
     * @see executed_ClientInfo()
     */
    bool insertCommand_ClientInfo( QByteArray &package, QString clientAddr, ClientStates state, bool includeSOH );

    // DISPLAY COMMANDS
    /**
     * @brief Constructs and inserts "Display Set configuration" command into a package.
     *
     * @param package Package bytes to insert constructed command.
     * @param key
     * @param keyState
     * @param ledBlinkTime
     * @param displayBlinkTime
     * @param includeSOH Set includeSOH to true , when you want to insert more than one command to empty package.
     * @see execCommand_DisplaySetConf()
     * @see executed_DisplaySetConf()
     */
    void insertCommand_DisplaySetConf( QByteArray &package, Key key, KeyState keyState, int ledBlinkTime, int displayBlinkTime, bool includeSOH );

    /**
     * @brief Constructs and inserts "Display" command into a package.
     *
     * @param package Package bytes to insert constructed command.
     * @param lengthOfData
     * @param data
     * @param elModeRed
     * @param elModeGreen
     * @param elModeBlue
     * @param includeSOH Set includeSOH to true , when you want to insert more than one command to empty package.
     * @return true on successful insert
     * @see execCommand_Display()
     * @see executed_Display()
     */
    bool insertCommand_Display(QByteArray &package, int lengthOfData, const QString &data, LightMode elModeRed, LightMode elModeGreen, LightMode elModeBlue, bool includeSOH );

    /**
     * @brief Constructs and inserts "Special display" command into a package.
     *
     * @param package Package bytes to insert constructed command.
     * @param type
     * @param data
     * @param lModeRed
     * @param lModeGreen
     * @param lModeBlue
     * @param elModeRed
     * @param elModeGreen
     * @param elModeBlue
     * @param dwn
     * @param up
     * @param includeSOH Set includeSOH to true , when you want to insert more than one command to empty package.
     * @see execCommand_SpecialDisplay()
     * @see executed_SpecialDisplay()
     *
     */
    void insertCommand_SpecialDisplay( QByteArray &package, SpecialDisplayType type, const QString &data, LightMode lModeRed, LightMode lModeGreen, LightMode lModeBlue, LightMode elModeRed, LightMode elModeGreen, LightMode elModeBlue, LightMode dwn, LightMode up, bool includeSOH );

    /**
     * @brief Constructs and inserts Scan command into a package.
     *
     * @param package Package bytes to insert constructed command.
     * @param type
     * @param scanningTime
     * @param includeSOH Set includeSOH to true , when you want to insert more than one command to empty package.
     * @see execCommand_Scan()
     * @see executed_Scan()
     */
    void insertCommand_Scan( QByteArray &package, ScanType type, int scanningTime, bool includeSOH );

    /**
     * @brief Constructs and inserts "Disable dispaly" command into a package.
     *
     * @param package Package bytes to insert constructed command.
     * @param mode
     * @param includeSOH Set includeSOH to true , when you want to insert more than one command to empty package.
     * @see execCommand_DisableDisplay()
     * @see executed_DisableDisplay()
     */
    void insertCommand_DisableDisplay( QByteArray &package, DisplayMode mode, bool includeSOH );

    /**
     * @brief Constructs and inserts "Key message" command into a package.
     *
     * @param package Package bytes to insert constructed command.
     * @param key
     * @param state
     * @param includeSOH Set includeSOH to true , when you want to insert more than one command to empty package.
     * @see execCommand_KeyMessage()
     * @see executed_KeyMessage()
     */
    void insertCommand_KeyMessage( QByteArray &package, Key key, KeyState state, bool includeSOH ); //To do ask about the command

    /**
     * @brief Constructs and inserts "Scanned message" command into a package.
     *
     * @param package Package bytes to insert constructed command.
     * @param type
     * @param state
     * @param scannedData
     * @param includeSOH Set includeSOH to true , when you want to insert more than one command to empty package.
     * @see execCommand_ScannedMessage()
     * @see executed_ScannedMessage()
     */
    void insertCommand_ScannedMessage(QByteArray &package, ScanType type, BarcodeState state, const QString &scannedData, bool includeSOH );

    // CLIENT COMMANDS
    /**
     * @brief Constructs and inserts "Client reset" command into a package.
     *
     * @param package Package bytes to insert constructed command.
     * @param state
     * @param includeSOH Set includeSOH to true , when you want to insert more than one command to empty package.
     * @see execCommand_ClientReset()
     * @see executed_ClientReset()
     */
    void insertCommand_ClientReset( QByteArray &package, ClientResetStates state, bool includeSOH );

    /**
     * @brief Constructs and inserts "Client new restart" command into a package.
     *
     * @param package Package bytes to insert constructed command.
     * @param state
     * @param includeSOH Set includeSOH to true , when you want to insert more than one command to empty package.
     * @see execCommand_ClientNewRestart()
     * @see executed_ClientNewRestart()
     */
    void insertCommand_ClientNewRestart( QByteArray &package, ClientNewRestartStates state, bool includeSOH );

    /**
     * @brief Constructs and inserts "Client reset done" command into a package.
     *
     * @param package Package bytes to insert constructed command.
     * @param hardwareV
     * @param softwareV
     * @param clientId
     * @param includeSOH Set includeSOH to true , when you want to insert more than one command to empty package.
     * @see execCommand_ClientResetDone()
     * @see executed_ClientResetDone()
     */
    void insertCommand_ClientResetDone(QByteArray &package, int hardwareV, int softwareV, QString &clientId, bool includeSOH );

    /**
     * @brief Constructs and inserts "Client error message" command into a package.
     *
     * @param package Package bytes to insert constructed command.
     * @param errorNumber
     * @param errorSpecs
     * @param includeSOH Set includeSOH to true , when you want to insert more than one command to empty package.
     * @see execCommand_ClientErrorMessage()
     * @see executed_ClientErrorMessage()
     */
    void insertCommand_ClientErrorMessage( QByteArray &package, int errorNumber, int errorSpecs, bool includeSOH );

//==================================================================================================================
    QString receivedPackageAddress; ///< Address of the received package.
    char receivedPackageRunningNumber; ///< Running number of received package

private:
    QString address; ///< Address
    char runningNumber; ///< Runing number

    /**
     * @brief Apend header to the package
     * The address in the header of the package is default address, and the runnung number is default too
     *
     * @param package Package to use
     * @param includeType
     */
    void appendHeader(QByteArray &package, bool includeType);

    /**
     * @brief Append footer to the package
     * Append the EOT, CRC and ETX characters
     *
     * @param package
     */
    void appendFooter( QByteArray &package );

    /**
     * @brief Append SOH to a command.
     * @param command Command to use
     * @param append
     */
    void appendSOH( QByteArray &command, bool append );

    /**
     * @brief Append Int value to a command.
     *
     * @param command Command to use
     * @param value Value for append to given command
     * @param numberOfBytes Number of bytes
     * @param maxValue Max possible value
     * @param minValue Min possible value
     * @param defaultValue Default value to use if given value is not in [min, max] range.
     */
    void appendIntToCommand( QByteArray &command, int value, int numberOfBytes, int maxValue, int minValue, int defaultValue );

    /**
     * @brief Append string value to a command.
     *
     * @param command Command to use
     * @param str
     * @param numberOfBytes
     */
    void appendStrToCommand( QByteArray &command, QString &str, int numberOfBytes );

    /**
     * @brief Append string value to a command.
     * @param command Command to use
     * @param str
     */
    void appendStrToCommand( QByteArray &command, const QString &str );

    /**
     * @brief Append dots to a command.
     * @param command Command to use
     * @param numberOfDots
     */
    void appendDotsToCommand( QByteArray &command, const int numberOfDots );

    /**
     * @brief Encodes a command.
     * @param command bytes of a command
     */
    void encodeCommand( QByteArray &command );

    /**
     * @brief Executes a list of commands
     * @param commands List of bytes of received commands
     * @return
     */
    bool executeCommands( QList< QByteArray > &commands );

    /**
     * @brief Defines status
     * @param status
     * @return true
     */
    bool defineStatus(const char status );

    /**
     * @brief Fixes address string
     * @param address
     * @return false if given address is longer.
     */
    bool fixAddressString( QString &address );

    void checkValueForDots( QString &value );

    void turnValueToDots(QString &value, int numberOfDots);

    // TERMINAL COMMANDS
    /**
     * @brief Executes received  "Data from client" command.
     * @param command Received command bytes.
     * @see executed_DFC()
     * @see insertCommand_DFC()
     */
    void execCommand_DFC( const QByteArray &command );

    /**
     * @brief Executes received  "Data to client" command.
     *
     * @param command Received command bytes.
     * @see executed_DTC()
     * @see insertCommand_DTC()
     */
    void execCommand_DTC( const QByteArray &command );

    /**
     * @brief Executes received  "Set configuration" command.
     *
     * @param command Received command bytes.
     * @see executed_SetConfiguration()
     * @see insertCommand_SetConfiguration()
     */
    void execCommand_SetConfiguration( const QByteArray &command );

    /**
     * @brief Executes received "Query configuration" command.
     *
     * @param command Received command bytes.
     * @see executed_QueryConfiguration()
     * @see insertCommand_QueryConfiguration()
     */
    void execCommand_QueryConfiguration( const QByteArray &command );

    /**
     * @brief Executes received "Query client info" command.
     *
     * @param command Received command bytes.
     * @see executed_QueryClientInfo()
     * @see insertCommand_QueryClientInfo()
     */
    void execCommand_QueryClientInfo( const QByteArray &command );

    /**
     * @brief Executes received "Current configuration" command.
     *
     * @param command Received command bytes.
     * @see executed_CurrentConfiguration()
     * @see insertCommand_CurrentConfiguration()
     */
    void execCommand_CurrentConfiguration( const QByteArray &command );

    /**
     * @brief Executes received "Client Info" command.
     *
     * @param command Received command bytes.
     * @see executed_ClientInfo()
     * @see insertCommand_ClientInfo()
     */
    void execCommand_ClientInfo( const QByteArray &command );

    // DISPLAY COMMANDS
    /**
     * @brief Executes received "Display set configuration" command.
     *
     * @param command Received command bytes.
     * @see executed_DisplaySetConf()
     * @see insertCommand_DisplaySetConf()
     */
    void execCommand_DisplaySetConf( const QByteArray &command );

    /**
     * @brief Executes received "Display" command.
     *
     * @param command Received command bytes.
     * @see executed_Display()
     * @see insertCommand_Display()
     */
    void execCommand_Display(const QByteArray &command);

    /**
     * @brief Executes received "Special Display" command.
     *
     * @param command Received command bytes.
     * @see executed_SpecialDisplay()
     * @see insertCommand_SpecialDisplay()
     */
    void execCommand_SpecialDisplay( const QByteArray &command );

    /**
     * @brief Executes received "Scan" command.
     *
     * @param command Received command bytes.
     * @see executed_Scan()
     * @see insertCommand_Scan()
     */
    void execCommand_Scan( const QByteArray &command );

    /**
     * @brief Executes received "Disable display" command.
     *
     * @param command Received command bytes.
     * @see executed_DisableDisplay()
     * @see insertCommand_DisableDisplay()
     */
    void execCommand_DisableDisplay( const QByteArray &command );

    /**
     * @brief Executes received "Key message" command.
     *
     * @param command Received command bytes.
     * @see executed_KeyMessage()
     * @see insertCommand_KeyMessage()
     */
    void execCommand_KeyMessage( const QByteArray &command );

    /**
     * @brief Executes received "Scanned message" command.
     * @param command Received command bytes.
     */
    void execCommand_ScannedMessage( const QByteArray &command );

    // CLIENT COMMANDS

    /**
     * @brief Executes received "Client reset" command.
     *
     * @param command Received command bytes.
     * @see executed_ClientReset()
     * @see insertCommand_ClientReset()
     */
    void execCommand_ClientReset( const QByteArray &command );

    /**
     * @brief Executes received "Client new restart" command.
     *
     * @param command Received command bytes.
     * @see executed_ClientNewRestart()
     * @see insertCommand_ClientNewRestart()
     */
    void execCommand_ClientNewRestart( const QByteArray &command );

    /**
     * @brief Executes received "Client reset done" command.
     *
     * @param command Received command bytes.
     * @see executed_ClientResetDone()
     * @see insertCommand_ClientResetDone()
     */
    void execCommand_ClientResetDone( const QByteArray &command );

    /**
     * @brief Executes received "Client error message" command.
     *
     * @param command Received command bytes.
     * @see executed_ClientErrorMessage()
     * @see insertCommand_ClientErrorMessage()
     */
    void execCommand_ClientErrorMessage( const QByteArray &command );

signals:
    // TERMINAL COMMANDS

    /**
     * @brief Signal is emmited when "Data from client" command is received.
     * @param addr
     * @param textData
     * @see execCommand_DFC()
     * @see insertCommand_DFC()
     */
    void executed_DFC( const QString &addr, const QString &textData );

    /**
     * @brief Signal is emmited when "Data to client" command is received.
     * @param addr
     * @param textData
     * @see execCommand_DTC()
     * @see insertCommand_DTC()
     */
    void executed_DTC( const QString &addr, const QString &textData );

    /**
     * @brief Signal is emmited when "Set configuration" command is received.
     *
     * @param reactionTimeOut
     * @param charTimeOut
     * @param pauseTime
     * @param scanRate
     * @param checkPollRate
     * @param connectionRepeats
     * @see execCommand_SetConfiguration()
     * @see insertCommand_SetConfiguration()
     */
    void executed_SetConfiguration( int reactionTimeOut, int charTimeOut, int pauseTime, int scanRate, int checkPollRate, int connectionRepeats );

    /**
     * @brief Signal is emmited when "Query configuration" command is received.
     *
     * @see execCommand_QueryConfiguration()
     * @see insertCommand_QueryConfiguration()
     */
    void executed_QueryConfiguration();

    /**
     * @brief Signal is emmited when "Query client info" command is received.
     *
     * @param clientAddr
     * @see execCommand_QueryClientInfo()
     * @see insertCommand_QueryClientInfo()
     */
    void executed_QueryClientInfo( const QString &clientAddr );

    /**
     * @brief Signal is emmited when "Current configuration" command is received.
     *
     * @param reactionTimeOut
     * @param charTimeOut
     * @param pauseTime
     * @param scanRate
     * @param checkPollRate
     * @param connectionRepeats
     * @see execCommand_CurrentConfiguration()
     * @see insertCommand_CurrentConfiguration()
     */
    void executed_CurrentConfiguration( int reactionTimeOut, int charTimeOut, int pauseTime, int scanRate, int checkPollRate, int connectionRepeats );

    /**
     * @brief Signal is emmited when "Client info" command is received.
     *
     * @param clientAddr
     * @param state
     * @see execCommand_ClientInfo()
     * @see insertCommand_ClientInfo()
     */
    void executed_ClientInfo( const QString &clientAddr, ClientStates state );

    // DISPLAY COMMANDS
    /**
     * @brief Signal is emmited when "Dispplay set configuration" command is received.
     *
     * @param key
     * @param keyState
     * @param ledBlinkTime
     * @param displayBlinkTime
     * @see execCommand_DisplaySetConf()
     * @see insertCommand_DisplaySetConf()
     */
    void executed_DisplaySetConf( Key key, KeyState keyState, int ledBlinkTime, int displayBlinkTime );

    /**
     * @brief Signal is emmited when "Display" command is received.
     *
     * @param data
     * @param elModeRed
     * @param elModeGreen
     * @param elModeBlue
     * @see execCommand_Display()
     * @see insertCommand_Display()
     */
    void executed_Display( const QString &data, LightMode elModeRed, LightMode elModeGreen, LightMode elModeBlue );

    /**
     * @brief Signal is emmited when "Special display" command is received.
     *
     * @param type
     * @param data
     * @param lModeRed
     * @param lModeGreen
     * @param lModeBlue
     * @param elModeRed
     * @param elModeGreen
     * @param elModeBlue
     * @param dwn
     * @param up
     * @see execCommand_SpecialDisplay()
     * @see insertCommand_SpecialDisplay()
     */
    void executed_SpecialDisplay( SpecialDisplayType type, const QString &data, LightMode lModeRed, LightMode lModeGreen, LightMode lModeBlue, LightMode elModeRed, LightMode elModeGreen, LightMode elModeBlue, LightMode dwn, LightMode up );

    /**
     * @brief Signal is emmited when "Scan" command is received.
     *
     * @param type
     * @param scanningTime
     * @see execCommand_Scan()
     * @see insertCommand_Scan()
     */
    void executed_Scan( ScanType type, int scanningTime );

    /**
     * @brief Signal is emmited when "Disable Display" command is received.
     *
     * @param mode
     * @see execCommand_DisableDisplay()
     * @see insertCommand_DisableDisplay()
     */
    void executed_DisableDisplay( DisplayMode mode );

    /**
     * @brief Signal is emmited when "Key message" command is received.
     *
     * @param key
     * @param state
     * @see execCommand_KeyMessage()
     * @see insertCommand_KeyMessage()
     */
    void executed_KeyMessage( Key key, KeyState state );

    /**
     * @brief Signal is emmited when "Scanned message" command is received.
     *
     * @param type
     * @param state
     * @param scanedData
     * @see execCommand_ScannedMessage()
     * @see insertCommand_ScannedMessage()
     */
    void executed_ScannedMessage( ScanType type, BarcodeState state, const QString &scanedData );

    // CLIENT COMMANDS
    /**
     * @brief Signal is emmited when "Client reset" command is received.
     *
     * @param state
     * @see execCommand_ClientReset()
     * @see insertCommand_ClientReset()
     */
    void executed_ClientReset( ClientResetStates state );

    /**
     * @brief Signal is emmited when "Client new restart" command is received.
     *
     * @param state
     * @see execCommand_ClientNewRestart()
     * @see insertCommand_ClientNewRestart()
     */
    void executed_ClientNewRestart( ClientNewRestartStates state );

    /**
     * @brief Signal is emmited when "Client reset done" command is received.
     *
     * @param hardwareV
     * @param softwareV
     * @param clientId
     * @see execCommand_ClientResetDone()
     * @see insertCommand_ClientResetDone()
     */
    void executed_ClientResetDone( int hardwareV, int softwareV, const QString &clientId );

    /**
     * @brief Signal is emmited when "Client error message" command is received.
     *
     * @param errorNumber
     * @param errorSpecs
     * @see execCommand_ClientErrorMessage()
     * @see insertCommand_ClientErrorMessage()
     */
    void executed_ClientErrorMessage( int errorNumber, int errorSpecs );


//Signals for status ==================================================
    /**
     * @brief
     */
    void statusOk();

    /**
     * @brief
     */
    void statusNoReceiver();

    /**
     * @brief
     */
    void statusMessagesOverFlow();

    /**
     * @brief
     */
    void statusRunningNumberError();
//=====================================================================
public slots:
};

#endif // PROTOCOLMANAGER_H
