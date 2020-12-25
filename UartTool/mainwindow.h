#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGlobal>
#include <QDebug>
#include <QDate>
#include <QTimer>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
#include <QFileDialog>

//ini读写
#include <QSettings>

//引入qt中串口通信需要的头文件
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

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
    void on_openSerialButton_clicked();

    void ReadData();

    void GetAveriablePort();

    void on_sendDataButton_clicked();

    void on_clearSendButton_clicked();

    void on_clearRecButton_clicked();

    void uartRec_timeout () ;      //定时溢出处理槽函数

    bool openTextByIODevice(const QString &aFileName);

    bool saveTextByIODevice(const QString &aFileName);

    void on_saveDataButton_clicked();

    void on_overTimeRecEdit_returnPressed();

    void on_readLogButton_clicked();

    void on_refreshPortButton_clicked();

    void PortConfigureInit();

    void updateMainStyle(QString style);

    void insertDataToPlain();

    void CmdListInit();

    void sendButtonClick(QString str);

    void IniParamInit();

    bool SaveUartParam();

    void on_paramSaveButton_clicked();

private:
    Ui::MainWindow *ui;

    QSerialPort *serial;            //全局的串口对象

    QDateTime curDateTime;          //当前时间

    QTimer * uartRecDataTimer;      //串口接收定时器
    QTime  fTimeCounter;            //串口计时器

    QString uart_rec_ss;            //串口接收数据

    qreal uartRecOvertimeCount;

    qint32 rec_buf_len;

    qint32 send_buf_len;
};

#endif // MAINWINDOW_H
