#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_restartSocket_clicked()
{
    socket.restartTheSocket();
}

void MainWindow::on_pushButton_abortSocket_clicked()
{
    socket.abortSocketConnection();
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
    socket.sendDisplayToClient( ui->lineEdit_ClientAddr->text(), number );
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
