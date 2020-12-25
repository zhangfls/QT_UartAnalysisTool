#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    updateMainStyle("styleDefault.qss");

    QMenuBar *menuBar = ui->menuBar;
    QMenu *pFile = menuBar->addMenu("主题设置");

    QDir dir1("/qss/");
    QFileInfoList list;
    QStringList filters;
    if(dir1.exists())
    {
        dir1.setFilter(QDir::Files); //加过滤器，选择前缀为name的文件
        filters << "*qss";
        dir1.setNameFilters(filters);
        list = dir1.entryInfoList();
    }
    else
    {
        qDebug()<<"qss不在盘根目录";
        QDir dir2("qss/");
        if(dir2.exists())
        {
            dir2.setFilter(QDir::Files); //加过滤器，选择前缀为name的文件
            dir2.setNameFilters(filters);
            list = dir2.entryInfoList();
        }
        else
        {
            qDebug()<<"qss不在程序根目录";
        }
    }

    qDebug()<<"文件数量:"<<QString::number(list.size());
    if(list.size()>1)
    {
        QFileInfo fileInfo;
        QList<QAction*> actions;
        foreach (fileInfo, list)
        {
            qDebug()<<fileInfo.fileName();
            actions.append(new QAction(fileInfo.fileName()));
        }
        for(QAction *ac:actions)
        {
            pFile->addActions(actions);
            connect(ac,&QAction::triggered,
                        [=] ()
                        {
                            updateMainStyle(ac->text());
                        }
                        );
        }
    }

    //变量初始化
    uartRecOvertimeCount = 0.1;
    rec_buf_len = 0;
    send_buf_len = 0;

    //设置uart接收缓冲超时定时器
    uartRecDataTimer = new QTimer(this);
    uartRecDataTimer->stop();
    uartRecDataTimer->setInterval(uartRecOvertimeCount*1000);                     //设置定时周期，单位：毫秒
    uartRecDataTimer->setSingleShot(true);                                        //设置为单次触发
    connect(uartRecDataTimer,SIGNAL(timeout()),this,SLOT(uartRec_timeout()));     //设置槽

    PortConfigureInit();    //配置串口初始化

    GetAveriablePort();     //查找可用的串口

    IniParamInit(); //读取参数文件,配置参数

    CmdListInit();  //指令栏初始化
}

MainWindow::~MainWindow()
{
    delete ui;
}

//发送数据
void MainWindow::sendButtonClick(QString command)
{
    //未打开串口则不准发送
    if(ui->openSerialButton->text() == "打开串口")
    {
        QMessageBox::warning(NULL, "警告", "未打开可用串口，无法发送数据！\r\n\r\n");
        return;
    }

    //获取发送的命令，并选择在结尾加上换行，AT的命令结尾必须有回车换行
    if(ui->changeLineCheckBox->isChecked())
    {
        command += "\r\n";
    }

    if(ui->timeZoneCheckBox->isChecked())
    {
         curDateTime = QDateTime::currentDateTime();
         ui->uartReadPlain->insertPlainText("\r\n"+curDateTime.toString("[yyyy-MM-dd hh:mm:ss]")+"SEND:"+command);
    }

    send_buf_len += command.length();
    ui->TXLenLabel->setText(QString::number(send_buf_len)+"bytes");

    serial->write(command.toLatin1());
}


bool MainWindow::SaveUartParam(void)
{
    QSettings *configIni = new QSettings("/qss/param.ini", QSettings::IniFormat);

    configIni->setIniCodec("UTF-8");

    if(configIni == NULL)
        return false;

    //波特率
    configIni->setValue("uartParam/BaudRate",ui->rateBox->currentText());

    //数据位
    configIni->setValue("uartParam/DataBit",ui->dataBox->currentText());

    //奇偶校验位
    configIni->setValue("uartParam/Parity",ui->checkBox->currentText());

    //停止位
    configIni->setValue("uartParam/StopBit",ui->stopBox->currentText());

    delete  configIni;

    return true;
}

//根据配置文件初始化参数
void MainWindow::IniParamInit(void)
{
    QSettings *configIni = new QSettings("/qss/param.ini", QSettings::IniFormat);

    configIni->setIniCodec("UTF-8");

    if(configIni == NULL)
        return;

    //波特率
    QString  baudRate = configIni->value("uartParam/BaudRate").toString();
    ui->rateBox->setCurrentText(baudRate);

    //数据位
    QString  dataBit = configIni->value("uartParam/DataBit").toString();
    ui->dataBox->setCurrentText(dataBit);

    //奇偶校验位
    QString  parity = configIni->value("uartParam/Parity").toString();
    qDebug()<<"parity:"<<parity;
    ui->checkBox->setCurrentText(parity);

    //停止位
    QString  stopBit = configIni->value("uartParam/StopBit").toString();
    ui->stopBox->setCurrentText(stopBit);

    delete  configIni;
}

//初始化命令列表
void MainWindow::CmdListInit()
{
    QStringList headerText;
    headerText<<"HEX"<<"数据"<<"发送";
    ui->tableWidget->setColumnCount(headerText.count());

    for(int i = 0;i<headerText.count();i++)
    {
        ui->tableWidget->setHorizontalHeaderItem(i,new QTableWidgetItem(headerText.at(i)));
    }

    ui->tableWidget->setColumnWidth(0,30);
    ui->tableWidget->setColumnWidth(1,200);
    ui->tableWidget->setColumnWidth(2,60);

    for(int i=0;i<100;i++)
    {
        ui->tableWidget->insertRow(i);

        QCheckBox *box = new QCheckBox();
        ui->tableWidget->setCellWidget(i,0,box);

        QLineEdit *line = new QLineEdit();
        ui->tableWidget->setCellWidget(i,1,line);

        QPushButton *button = new QPushButton();
        button->setText("点击");
        ui->tableWidget->setCellWidget(i,2,button);
        connect(button,&QPushButton::clicked,
                    [=] ()
                    {
                        sendButtonClick(line->text());
                    }
                    );
    }
}

//配置串口初始化
void MainWindow::PortConfigureInit()
{
    //填入串口选项
    ui->rateBox->addItem("9600","9600");
    ui->rateBox->addItem("19200","19200");
    ui->rateBox->addItem("38400","38400");
    ui->rateBox->addItem("57600","57600");
    ui->rateBox->addItem("115200","115200");
    ui->rateBox->addItem("921600","921600");

    ui->dataBox->addItem("8",8);
    ui->dataBox->addItem("7",7);

    ui->checkBox->addItem("无校验",0);
    ui->checkBox->addItem("奇校验",1);
    ui->checkBox->addItem("偶校验",2);

    ui->stopBox->addItem("1位",1);
    ui->stopBox->addItem("2位",2);

    //设置时间输入框只允许使用数字
    ui->overTimeRecEdit->setValidator(new QRegExpValidator(QRegExp("^([0-9]{1,4}(.[0-9]{1,3})?)$")));
    ui->overTimeRecEdit->setText(QString::number(uartRecOvertimeCount));
}

//串口开关按钮
void MainWindow::on_openSerialButton_clicked()
{
    //尝试打开串口
    if(ui->openSerialButton->text() == tr("打开串口"))
    {
        if(ui->portBox->currentText() == "" )
        {
            QMessageBox::warning(NULL, "警告", "无可开启串口！\r\n\r\n");
            return;
        }

        serial = new QSerialPort;
        //设置串口名
        serial->setPortName(ui->portBox->currentText());
        //打开串口
        serial->open(QIODevice::ReadWrite);
        //设置波特率
        serial->setBaudRate(ui->rateBox->currentText().toInt());
        //设置数据位
        switch (ui->dataBox->currentData().toInt())
        {
            case 8:
                serial->setDataBits(QSerialPort::Data8);
                break;
            case 7:
                serial->setDataBits(QSerialPort::Data7);
                break;
            default:
                break;
        }
        //设置校验位
        switch (ui->checkBox->currentIndex())
        {
            case 0:
                serial->setParity(QSerialPort::NoParity);
                break;
            case 1:
                serial->setParity(QSerialPort::EvenParity);
                break;
            case 2:
                serial->setParity(QSerialPort::OddParity);
                break;
            default:
                break;
        }
        //设置停止位
        switch(ui->stopBox->currentIndex())
        {
            case 0:
                serial->setStopBits(QSerialPort::OneStop);
                break;
            case 1:
                serial->setStopBits(QSerialPort::TwoStop);
                break;
            default:
                break;
        }
        //设置流控制
        serial->setFlowControl(QSerialPort::NoFlowControl); //设置为无流控制

        //关闭设置菜单使能
        ui->portBox->setEnabled(false);
        ui->dataBox->setEnabled(false);
        ui->checkBox->setEnabled(false);
        ui->stopBox->setEnabled(false);
        ui->rateBox->setEnabled(false);
        ui->openSerialButton->setText("关闭串口");

        fTimeCounter.restart();

        //连接信号和槽函数，串口有数据可读时，调用ReadData()函数读取数据并处理。
        QObject::connect(serial,&QSerialPort::readyRead,this,&MainWindow::ReadData);
    }
    else
    {
        uartRecDataTimer->stop () ; //定时器停止

        if(serial->isOpen())       //原先串口打开，则关闭串口
        {
            serial->close();
        }

        //释放串口
        delete serial;
        serial = NULL;

        //恢复使能
        ui->portBox->setEnabled(true);
        ui->rateBox->setEnabled(true);
        ui->dataBox->setEnabled(true);
        ui->checkBox->setEnabled(true);
        ui->stopBox->setEnabled(true);
        ui->openSerialButton->setText("打开串口");
    }
}

//定时器触发打印串口数据
void MainWindow::uartRec_timeout()
{
    if(!uart_rec_ss.isEmpty())
    {
        ui->uartReadPlain->moveCursor(QTextCursor::End);            //光标移动到结尾

        insertDataToPlain();

        ui->uartReadPlain->moveCursor(QTextCursor::End);            //光标移动到结尾
        uart_rec_ss.clear();
        fTimeCounter.restart();
        ui->RXLenLabel->setText(QString::number(rec_buf_len)+"bytes");
    }
}

//读取串口接收消息
void MainWindow::ReadData()
{
    //串口可读数据长度
    int byteLen = serial->bytesAvailable();
    if(byteLen < 0)
    {
        return;
    }

    rec_buf_len += byteLen;
    uart_rec_ss.append(serial->readAll());  //读取数据

    //计时器超过最大间隔仍未填入数据，强制填入
    if(fTimeCounter.elapsed() >2000 && uart_rec_ss.size()>0)
    {
        ui->uartReadPlain->moveCursor(QTextCursor::End);        //光标移动到结尾
        insertDataToPlain();
        ui->uartReadPlain->moveCursor(QTextCursor::End);        //光标移动到结尾
        uart_rec_ss.clear();
        ui->RXLenLabel->setText(QString::number(rec_buf_len)+"bytes");
    }

    //定时器开始工作、定时器重启
    uartRecDataTimer->start();
}

//填入数据到面板
void MainWindow::insertDataToPlain()
{
    curDateTime = QDateTime::currentDateTime();

    if(ui->timeZoneCheckBox->isChecked())
    {
        ui->uartReadPlain->insertPlainText("\r\n"+curDateTime.toString("[yyyy-MM-dd hh:mm:ss]")+"R:");
        ui->uartReadPlain->moveCursor(QTextCursor::End);        //光标移动到结尾
        ui->uartReadPlain->insertPlainText(uart_rec_ss);
    }
    else
    {
        ui->uartReadPlain->insertPlainText(uart_rec_ss);
    }
}

//发送串口数据
void MainWindow::on_sendDataButton_clicked()
{
    //未打开串口则不准发送
    if(ui->openSerialButton->text() == "打开串口")
    {
        QMessageBox::warning(NULL, "警告", "未打开可用串口，无法发送数据！\r\n\r\n");
        return;
    }

    //获取发送的命令，并选择在结尾加上换行，AT的命令结尾必须有回车换行
    QString command = ui->uartWritePlain->toPlainText();
    if(ui->changeLineCheckBox->isChecked())
    {
        command += "\r\n";
    }

    if(ui->timeZoneCheckBox->isChecked())
    {
         curDateTime = QDateTime::currentDateTime();
         ui->uartReadPlain->insertPlainText("\r\n"+curDateTime.toString("[yyyy-MM-dd hh:mm:ss]")+"SEND:"+command);
    }

    send_buf_len += command.length();
    ui->TXLenLabel->setText(QString::number(send_buf_len)+"bytes");

    serial->write(command.toLatin1());
}

//清除发送
void MainWindow::on_clearSendButton_clicked()
{
    ui->uartWritePlain->clear();
}

//清除接收
void MainWindow::on_clearRecButton_clicked()
{
    ui->uartReadPlain->clear();

    send_buf_len = 0;
    rec_buf_len = 0;
    ui->TXLenLabel->setText("0bytes");
    ui->RXLenLabel->setText("0bytes");
}

//超时间隔设置
void MainWindow::on_overTimeRecEdit_returnPressed()
{
    if(ui->overTimeRecEdit->text().toFloat()>60)
    {
        QMessageBox::warning(NULL,"警告","超时时间不要超过1分钟");
        ui->overTimeRecEdit->setText("0.1");
        return;
    }
    uartRecOvertimeCount = ui->overTimeRecEdit->text().toFloat();
    ui->uartReadPlain->insertPlainText("设置超时时间为："+QString::number(uartRecOvertimeCount*1000)+"ms");
    uartRecDataTimer->setInterval(uartRecOvertimeCount*1000);                       //设置定时周期，单位：毫秒

    fTimeCounter.restart();
    uartRecDataTimer->start();
}

//保存文本
void MainWindow::on_saveDataButton_clicked()
{
    QString curPath = QDir::currentPath();      //获取系统当前目录
    QString dlgTitle = "另存为一个文件";           //对话框标题
    QString filter = "文本文件(*.txt);;所有文件(*.*);;h文件(*.h)";    //文件过滤器
    QString aFileName = QFileDialog::getSaveFileName(this,dlgTitle,curPath,filter);
    if (aFileName.isEmpty())
        return;
    saveTextByIODevice(aFileName);
}

//用IODevice方式保存文本文件
bool MainWindow::saveTextByIODevice(const QString &aFileName)
{
    QFile aFile(aFileName);
    //aFile.setFileName(aFileName);
    if (!aFile.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    QString str = ui->uartReadPlain->toPlainText();     //整个内容作为字符串
    QByteArray strBytes = str.toUtf8();                 //转换为字节数组
    //QByteArray  strBytes=str.toLocal8Bit();
    aFile.write(strBytes,strBytes.length());            //写入文件
    aFile.close();
    ui->uartReadPlain->clear();
    send_buf_len = 0;
    rec_buf_len = 0;
    ui->TXLenLabel->setText("0bytes");
    ui->RXLenLabel->setText("0bytes");

    QMessageBox::information(NULL, "提示", "保存完成\r\n");
    return true;
}

//用IODevice方式打开文本文件
bool MainWindow::openTextByIODevice(const QString &aFileName)
{
    QFile aFile(aFileName);
    if (!aFile.exists())                //文件不存在
        return false;
    if (!aFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;
    ui->uartReadPlain->insertPlainText(QString(aFile.readAll()));
    aFile.close();
    return  true;
}

//打开文件
void MainWindow::on_readLogButton_clicked()
{
    QString curPath=QDir::currentPath();    //获取系统当前目录
    QString dlgTitle="打开文件";            //对话框标题
    QString filter="文本文件(*.txt);;程序文件(*.h *.cpp);;所有文件(*.*)"; //文件过滤器
    QString aFileName=QFileDialog::getOpenFileName(this,dlgTitle,curPath,filter);

    if (aFileName.isEmpty())
        return;
    openTextByIODevice(aFileName);
}

//刷新端口
void MainWindow::on_refreshPortButton_clicked()
{
    GetAveriablePort();
}

//查找可用串口,刷新串口信息
void MainWindow::GetAveriablePort()
{
     ui->uartReadPlain->moveCursor(QTextCursor::End);        //光标移动到结尾
     ui->uartReadPlain->insertPlainText("\r\n串口初始化：\r\n");

     //先清除所有串口列表
      ui->portBox->clear();

    foreach(const QSerialPortInfo &info,QSerialPortInfo::availablePorts())
    {
        QSerialPort serial;
        serial.setPort(info);

        if(serial.open(QIODevice::ReadWrite))
        {
            ui->uartReadPlain->insertPlainText("可用："+serial.portName()+"\r\n");
            ui->portBox->addItem(serial.portName());
            serial.close();
        }
        else
        {
            ui->uartReadPlain->insertPlainText("不可用："+serial.portName()+"\r\n");
        }
    }

    ui->uartReadPlain->moveCursor(QTextCursor::End);        //光标移动到结尾
}

//更新样式
void MainWindow::updateMainStyle(QString style)
{
    //QSS文件初始化界面样式
    QFile qss("qss/"+style);
    if(qss.open(QFile::ReadOnly))
    {
        qDebug()<<"qss load";
        QTextStream filetext(&qss);
        QString stylesheet = filetext.readAll();
        this->setStyleSheet(stylesheet);
        qss.close();
    }
    else
    {
        qDebug()<<"qss not load";
        qss.setFileName("/qss/"+style);
        if(qss.open(QFile::ReadOnly))
        {
            qDebug()<<"qss load";
            QTextStream filetext(&qss);
            QString stylesheet = filetext.readAll();
            this->setStyleSheet(stylesheet);
            qss.close();
        }
    }
}

void MainWindow::on_paramSaveButton_clicked()
{
    if(SaveUartParam())
    {
        QMessageBox::information(NULL,"保存参数","保存成功");
    }
}
