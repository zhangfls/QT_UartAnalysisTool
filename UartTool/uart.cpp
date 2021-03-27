#include "uart.h"

//发送串口数据
QString MainWindow::sendUartData(QString data,bool isHex,bool hasTimerStamp,bool isAT)
{
    curDateTime = QDateTime::currentDateTime();

    QString command = "";
    QString showBuff = "";

    command.append(data);

    if(isAT){
        command.append("\r\n");
    }

    if(hasTimerStamp){
//        QString tempStr = "\r\n"+curDateTime.toString("[yyyy-MM-dd hh:mm:ss]")+"S:";
        QString tempStr = "\r\n"+curDateTime.toString("[hh:mm:ss]")+"S:";
        showBuff = tempStr + command;
    }

    //HEX格式发送
    if(isHex){
        //将接收到的HEX格式字符串转为HEX格式原始数据
        QByteArray arr;

        //分析字符串格式中是否带空格
        for(int i = 0;i<data.size();i++)
        {
            if(data[i]==' ')
                continue;   //跳过

            int num  = data.mid(i,2).toUInt(nullptr,16);
            i++;

            arr.append(num);
        }

        send_buf_len += arr.length();
        //发送HEX格式原始数据
        serial->write(arr);

        //返回HEX格式字符串显示到串口接收面板
        return showBuff;
    }
    //非HEX格式发送
    else {
        send_buf_len += data.length();
        serial->write(command.toLatin1());
        return showBuff;
    }
}

