#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGlobal>
#include <QDebug>
#include <QDate>
#include <QTimer>
#include <QElapsedTimer>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
#include <QFileDialog>
#include <QSettings>    //ini读写

#include <letterformwindow.h>

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

    QSerialPort *serial;            //全局的串口对象
    QDateTime curDateTime;          //当前时间
    qint32 rec_buf_len;             //接收累计长度
    qint32 send_buf_len;            //发送累计长度

    QTimer * sysTimer;              //系统定时器
    QTimer * uartRecDataTimer;      //串口接收定时器
    QElapsedTimer  * fTimeCounter;    //串口计时器


private slots:

    //uart
    QString sendUartData(QString data,bool isHex,bool hasTimerStamp,bool isAT);

    void uartRec_timeout () ;      //定时溢出处理槽函数

    void SysStateDeal();

    bool openTextByIODevice(const QString &aFileName);

    bool saveTextByIODevice(const QString &aFileName);

    void ReadData();

    void GetAveriablePort();

    void on_overTimeRecEdit_returnPressed();

    void PortConfigureInit();

    void updateMainStyle(QString style);

    void insertDataToPlain();  //将串口数据填入面板

    void CmdListInit();

    void IniParamInit();

    bool SaveUartParam();

    void MenuBarInit();

    void receiveFont(QFont font);   //接收传递过来的字体

    //事件槽
    void on_openSerialButton_clicked();
    void on_sendDataButton_clicked();
    void on_clearSendButton_clicked();
    void on_clearRecButton_clicked();
    void on_saveDataButton_clicked();
    void on_readLogButton_clicked();
    void on_refreshPortButton_clicked();
    void sendButtonClick(QString str,bool isChecked);
    void on_paramSaveButton_clicked();

private:

    Ui::MainWindow *ui;

    letterFormWindow *letterFormUi = NULL;  //字体窗口

    QByteArray uart_rec_ss;            //串口接收数据
    qreal uartRecOvertimeCount;     //串口接收超时计数
    QSettings *configIni;           //配置文件

    bool isCmdPlainExpand;          //命令面板窗口是否展开

};

#endif // MAINWINDOW_H
