#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <ConnectionTools/clientsocket.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    void on_pushButton_restartSocket_clicked();

    void on_pushButton_abortSocket_clicked();

    void on_pushButton_QueryConfiguration_clicked();

    void on_pushButton_setConfiguration_clicked();

    void on_pushButton_QueryClientInfo_clicked();

    void on_pushButton_SetHost_clicked();

private:
    Ui::MainWindow *ui;
    ClientSocket socket;
};

#endif // MAINWINDOW_H
