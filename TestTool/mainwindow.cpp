#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    displayCounter = 0;
    socketConnected = false;
    this->startTimer( 2000 );
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    if( socketConnected ){


        switch( displayCounter ){
            case 0:
                socket.sendDisplayToClient( "2", "22", false, true, false );
            break;
            case 1:
                socket.sendScan("2", 1, 50 );
            break;
            case 2:
                socket.sendScan("2", 2, 50 );
            break;
            case 3:
                socket.sendSpecialDisplay( "2", "", 1);
            break;
            case 4:
                socket.sendSpecialDisplay( "2", "Test Error", 3);
            break;
            case 5:
                socket.sendDisableDisplay( "2", 1 );
            break;
        }

        if( displayCounter == 5){
            displayCounter = 0;
        } else {
            displayCounter ++;
        }
    }

}

void MainWindow::on_pushButton_restartSocket_clicked()
{
    socket.restartTheSocket();
    socketConnected = true;
}

void MainWindow::on_pushButton_abortSocket_clicked()
{
    socket.abortSocketConnection();
    socketConnected = false;
}

void MainWindow::on_pushButton_QueryConfiguration_clicked()
{
    socket.sendQueryConfiguration();
}

void MainWindow::on_pushButton_setConfiguration_clicked()
{
    socket.sendSetConfiguration();
}

void MainWindow::on_pushButton_QueryClientInfo_clicked()
{
    socket.sendQueryClientInfo( ui->lineEdit_ClientAddr->text() );
}

void MainWindow::on_pushButton_SetHost_clicked()
{
    socket.setHostIp( ui->lineEdit_HostIp->text() );
    socket.hostPort = ui->lineEdit_HostPort->text();
}

void MainWindow::on_pushButton_displayToClient_clicked()
{
    QString number = ui->lineEdit_BoxCount->text();
    if( number.isEmpty() ){
        number = " 0";
    } else if( number.size() == 1 ){
        number.prepend(' ');
    }
    socket.sendDisplayToClient( ui->lineEdit_ClientAddr->text(), number, ui->checkBox_R->isChecked(), ui->checkBox_G->isChecked(), ui->checkBox_B->isChecked() );
}

void MainWindow::on_pushButton_displayConf_clicked()
{
    socket.sendDisplayConfiguration( ui->lineEdit_ClientAddr->text() );
}

void MainWindow::on_pushButton_specialDisplay_clicked()
{
    socket.sendSpecialDisplay( ui->lineEdit_ClientAddr->text(), ui->lineEdit_screenMessage->text(), ui->comboBox_screenType->currentIndex() + 1);
}

void MainWindow::on_pushButton_scan_clicked()
{
    socket.sendScan(ui->lineEdit_ClientAddr->text(), ui->comboBox_scanType->currentIndex() +1, ui->spinBox_scanTime->value() );
}

void MainWindow::on_pushButton_DisableDisplay_clicked()
{
    socket.sendDisableDisplay( ui->lineEdit_ClientAddr->text(), ui->comboBox_DisDisplayType->currentIndex() + 1 );
}

void MainWindow::on_pushButton_ClientReset_clicked()
{
    socket.sendClientReset( ui->lineEdit_ClientAddr->text() );
}

void MainWindow::on_pushButton_HostReset_clicked()
{
    socket.sendHostReset();
}

void MainWindow::on_pushButton_SendEmptyPackage_clicked()
{
    socket.sendEmptyPackage();
}
