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
