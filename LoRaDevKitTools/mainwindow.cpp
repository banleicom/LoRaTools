#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtWidgets>
#include <QMessageBox>
#include <sys/time.h>
#include <QSerialPortInfo>
#include<windows.h>
#define TARGET_NAME   "LoRaDevKitTools "
#define VER           "V1.0.0 "
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_status(new QLabel),
    lora_serial(new QSerialPort(this))
{
    ui->setupUi(this);
    setWindowTitle(tr(TARGET_NAME)+tr("Tools"));
    ui->Button_open_com->setEnabled(false);
    ui->Button_clear_com->setEnabled(true);
    ui->send_Button->setEnabled(false);
    ui->groupBox_setting->setEnabled(false);

    ui->statusBar->addWidget(m_status);
    /*设置ACSII按钮为单选状态*/
    QButtonGroup *read_pButtonGroup = new QButtonGroup(this);
    read_pButtonGroup->addButton(ui->read_ASCII);
    read_pButtonGroup->addButton(ui->read_Hex);
    QButtonGroup *write_pButtonGroup = new QButtonGroup(this);
    write_pButtonGroup->addButton(ui->send_ASCII);
    write_pButtonGroup->addButton(ui->send_Hex);
    ui->read_ASCII->setChecked(true);
    ui->send_ASCII->setChecked(true);

    ui->read_textEdit->setFocusPolicy(Qt::NoFocus);
    QObject::connect(lora_serial, &QSerialPort::readyRead, this, &MainWindow::readData);

    InitCombobox();
    showStatusMessage(tr("欢迎使用本软件"));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_Button_open_com_clicked(bool checked)
{
    /*打开串口*/
    if (!lora_serial->isOpen())
    {
        /* 判断是否有可用串口 */
        if(ui->comboBox_com->count() != 0)
        {
            /* 串口已经关闭，现在来打开串口 */
            /* 设置串口名称 */
            lora_serial->setPortName(ui->comboBox_com->currentText());
            /* 设置波特率 */
            lora_serial->setBaudRate(QSerialPort::Baud9600);
            /* 设置数据位数 */
            lora_serial->setDataBits(QSerialPort::Data8);
            /* 设置奇偶校验 */
            lora_serial->setParity(QSerialPort::NoParity);
            /* 设置停止位 */
            lora_serial->setStopBits(QSerialPort::OneStop);
            /* 设置流控制 */
            lora_serial->setFlowControl(QSerialPort::NoFlowControl);
            /* 打开串口 */
            lora_serial->open(QIODevice::ReadWrite);
            /* 注册回调函数 */

            /*UI*/
            lora_serial->readAll();
            ui->Button_open_com->setText(tr("关闭串口"));
            ui->Button_open_com->setStyleSheet(QString("QPushButton{color:red}"));
            ui->Button_clear_com->setEnabled(false);
            ui->send_Button->setEnabled(true);
            ui->groupBox_setting->setEnabled(true);

            showStatusMessage(tr("Connected to %1").arg(ui->comboBox_com->currentText()));

        }else{
            qDebug()<<"没有可用串口，请重新常识扫描串口";
            // 警告对话框
            QMessageBox::warning(this,tr("警告"),tr("没有可用串口，请重新尝试扫描串口！"),QMessageBox::Ok);
        }

    }else{
        /* 关闭串口 */
        lora_serial->close();
        ui->Button_open_com->setText(tr("打开串口"));
        ui->Button_open_com->setStyleSheet(QString("QPushButton{color:black}"));
        ui->Button_clear_com->setEnabled(true);
        ui->send_Button->setEnabled(false);
        ui->groupBox_setting->setEnabled(false);
        showStatusMessage(tr("Unconnected"));
        qDebug()<<"串口在打开状态，串口关闭";
    }
}

void MainWindow::on_Button_clear_com_clicked(bool checked)
{
    /* 查找可用串口 */
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        QSerialPort serial;
        serial.setPort(info);
        /* 判断端口是否能打开 */
        if(serial.open(QIODevice::ReadWrite))
        {
            int isHaveItemInList = 0;
            /* 判断是不是已经在列表中了 */
            for(int i=0; i<ui->comboBox_com->count(); i++)
            {
                /* 如果，已经在列表中了，那么不添加这一项了就 */
                if(ui->comboBox_com->itemText(i) == serial.portName())
                {
                    isHaveItemInList++;
                }
            }

            if(isHaveItemInList == 0)
            {
                ui->comboBox_com->addItem(serial.portName());
            }
            serial.close();
        }
    }
    ui->Button_open_com->setEnabled(true);
}

void MainWindow::readData()
{
    uart_buffer.append(lora_serial->readAll());
    if(uart_buffer[0]==0xEA && uart_buffer[1]==0xEB)
    {
        //pos=Find_EaEb(uart_buffer);
        memcpy(&host_data,uart_buffer.data(),HOST_NUM);
        uart_buffer.clear();
        Par2Com(host_data);
        showStatusMessage(tr("Lora parameter Get!"));
    }else{
        QString curText=ui->read_textEdit->toPlainText();
        if(ui->read_ASCII->isChecked())
        {
            //ui->read_textEdit->append(uart_buffer);
            curText.append(uart_buffer);
            ui->read_textEdit->setPlainText(curText);
        }else{
            curText.append(uart_buffer.toHex());
            ui->read_textEdit->setPlainText(curText);
        }
        ui->read_textEdit->moveCursor(QTextCursor::End);
        uart_buffer.clear();
    }
}

void MainWindow::writeData(const QByteArray &data)
{
    lora_serial->write(data);
}

void MainWindow::showStatusMessage(const QString &message)
{
    QString time = QDateTime::currentDateTime().toString("hh:mm:ss");
    m_status->setText(time + QString("---") + message);
}

void MainWindow::InitCombobox()
{
    QStringList freq_list;
    QStringList BW_list;
    QStringList power_list;
    QStringList SF_list;
    QStringList CR_list;
    QStringList mode_list;
    freq_list<<"EU868"<<"CN470"<<"EU433"<<"AU915";
    power_list<<"14"<<"15"<<"16"<<"17"<<"18"<<"19"<<"20"<<"21"<<"22";
    BW_list<<"125K"<<"250K"<<"500K"<<"20K";
    SF_list<<"SF7"<<"SF8"<<"SF9"<<"SF10"<<"SF11"<<"SF12";
    CR_list<<"4/5"<<"4/6"<<"4/7"<<"4/8";
    mode_list<<tr("只接收")<<tr("只发送")<<tr("连续发送")<<tr("休眠")<<tr("设置模式")<<"FS"<<tr("输出载频");
    //ui->comboBox_freq->addItems(freq_list);
    ui->comboBox_power->addItems(power_list);
    ui->comboBox_BW->addItems(BW_list);
    ui->comboBox_SF->addItems(SF_list);
    ui->comboBox_CR->addItems(CR_list);
    ui->comboBox_mode->addItems(mode_list);
}

void MainWindow::Par2Com(host_t &host_data)
{
    FQV_t fqv;
    //ui->comboBox_freq->setCurrentIndex(host_data.cfg[0]);
    ui->comboBox_power->setCurrentIndex(host_data.cfg[1]-14);
    ui->comboBox_BW->setCurrentIndex(host_data.cfg[2]);
    ui->comboBox_SF->setCurrentIndex(host_data.cfg[3]-7);
    ui->comboBox_CR->setCurrentIndex(host_data.cfg[4]-1);
    ui->comboBox_mode->setCurrentIndex(host_data.cfg[5]);
    fqv.Fqv_8[3]=host_data.cfg[6];
    fqv.Fqv_8[2]=host_data.cfg[7];
    fqv.Fqv_8[1]=host_data.cfg[8];
    fqv.Fqv_8[0]=host_data.cfg[9];
    ui->lineEdit_freq->setText(QString::number(fqv.Fqv_32));
    ui->lineEdit_time->setText(QString::number(host_data.time));
    ui->lineEdit_data->setText(QString("%1").arg((char*)&host_data.data));

}

bool MainWindow::Com2Par(host_t &host_data)
{
    bool ok;

    host_data.cfg[1]= ui->comboBox_power->currentIndex()+14;
    host_data.cfg[2]= ui->comboBox_BW->currentIndex();
    host_data.cfg[3]= ui->comboBox_SF->currentIndex()+7;
    host_data.cfg[4]= ui->comboBox_CR->currentIndex()+1;
    host_data.cfg[5]= ui->comboBox_mode->currentIndex();

    QString frq=ui->lineEdit_freq->text();
    if(frq.isEmpty() || !isDigitString(frq) || frq.length()!=9)// 为空 或 不是数字 或 位数不为9
    {
        QMessageBox::warning(this,tr("错误"),tr("频率格式错误"),
                             QMessageBox::Ok);
        return 0;
    }

    QString times=ui->lineEdit_time->text();
    if(!times.isEmpty() && !isDigitString(times))// 非空且非数字字符串
    {
        QMessageBox::warning(this,tr("错误"),tr("时间格式错误"),
                             QMessageBox::Ok);
        return 0;
    }
    if(times.isEmpty())
    {
       host_data.time=0;
    }else{
       host_data.time=ui->lineEdit_time->text().toInt(&ok);
    }

    QString s= ui->lineEdit_data->text();
    if((s.length()>20))
    {
        QMessageBox::warning(this,tr("错误"),tr("数据长度不得大于20字节"),
                             QMessageBox::Ok);
        return 0;
    }

    strcpy((char*)&host_data.data,s.toLatin1().data());
    return 1;
}

void MainWindow::on_send_Button_clicked()
{
    /*
    QByteArray data;
    if(ui->send_ASCII->isChecked())
    {
        data=ui->send_text->toPlainText().toUtf8();
    }else{
        String2Hex(ui->send_text->toPlainText(),data);
    }
   writeData(data);
   */
    QString str;
    if(ui->send_text->toPlainText().isEmpty())
    {
        return;
    }
    if(ui->send_ASCII->isChecked())
    {
        str=tr("AT+BGSEND=%1\r\n").arg(ui->send_text->toPlainText());
    }else{
        str=tr("AT+BGSENDB=%1\r\n").arg(ui->send_text->toPlainText());
    }
    writeData(str.toUtf8());
}

void MainWindow::on_clear_Button_clicked()
{
    ui->read_textEdit->clear();
}

void MainWindow::String2Hex(QString str, QByteArray &senddata)
{
    int hexdata,lowhexdata;
    int hexdatalen = 0;
    int len = str.length();
    senddata.resize(len/2);
    char lstr,hstr;
    for(int i=0; i<len; )
    {
        //char lstr,
        hstr=str[i].toLatin1();
        if(hstr == ' ')
        {
            i++;
            continue;
        }
        i++;
        if(i >= len)
            break;
        lstr = str[i].toLatin1();
        hexdata = ConvertHexChar(hstr);
        lowhexdata = ConvertHexChar(lstr);
        if((hexdata == 16) || (lowhexdata == 16))
            break;
        else
            hexdata = hexdata*16+lowhexdata;
        i++;
        senddata[hexdatalen] = (char)hexdata;
        hexdatalen++;
    }
    senddata.resize(hexdatalen);
}

char MainWindow::ConvertHexChar(char ch)
{
    if((ch >= '0') && (ch <= '9'))
        return ch-0x30;
    else if((ch >= 'A') && (ch <= 'F'))
        return ch-'A'+10;
    else if((ch >= 'a') && (ch <= 'f'))
        return ch-'a'+10;
    else return (-1);
}

int MainWindow::Find_EaEb(QByteArray src)
{
    int pos=0;
    for(int i=0;i<src.size()-2;i++)
    {
        if(src[i]==0xEA && src[i+1]==0xEB)
        {
            return pos;
        }
        pos++;
    }
    return -1;
}

bool MainWindow::isDigitString(const QString &src)
{
    const char *s = src.toUtf8().data();
    while(*s && *s>='0' && *s<='9')s++;
    return !bool(*s);
}

void MainWindow::on_pushButton_read_clicked()
{
    lora_serial->write("ATC?\r\n");
}

void MainWindow::on_pushButton_write_clicked()
{
    if(!Com2Par(host_data))
    {
        return;
    }
    QString str=tr("AT+BGCFG=%1,%2,%3,%4,%5,%6,%7,%8\r\n")
            .arg(ui->lineEdit_freq->text())
            .arg(QString::number(host_data.cfg[1]))
            .arg(QString::number(host_data.cfg[2]))
            .arg(QString::number(host_data.cfg[3]))
            .arg(QString::number(host_data.cfg[4]))
            .arg(QString::number(host_data.cfg[5]))
            .arg(QString::number(host_data.time))
            .arg(QString::fromUtf8((char*)&host_data.data));
    writeData(str.toUtf8());
}

void MainWindow::on_pushButton_flash_clicked()
{
    QString str=tr("AT+BGFLASH\r\n");
    writeData(str.toUtf8());
}

void MainWindow::on_pushButton_Factory_clicked()
{
    QString str=tr("AT+BGRESET\r\n");
    writeData(str.toUtf8());
}

void MainWindow::on_pushButton_MCU_clicked()
{
    QString str=tr("ATZ\r\n");
    writeData(str.toUtf8());
}

void MainWindow::on_pushButton_INF_clicked()
{
    QString str=tr("AT+VER=?\r\n");
    writeData(str.toUtf8());
}

void MainWindow::on_pushButton_help_clicked()
{
    QString str=tr("AT?\r\n");
    writeData(str.toUtf8());
}

void MainWindow::on_pushButton_About_clicked()
{
    QMessageBox::about(this, tr("关于"),
                       tr(TARGET_NAME)+tr(VER));

}

void MainWindow::on_pushButton_Rssi_clicked()
{
    QString str=tr("AT+RSSI=?\r\n");
    writeData(str.toUtf8());
}
