#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
//#include "serialthread.h"
class QLabel;

#define SREG_USE        10     // 已使用的S寄存器数量
#define SREG_MAX_NUM    15    // 总的S寄存器数量
#define HOST_NUM        SREG_MAX_NUM+26

#pragma pack (push, 1)
typedef struct
{
    uint8_t flg[2];
    uint8_t cfg[SREG_MAX_NUM];     //参数信息
    uint32_t time;      //时间
    uint8_t data[20];   //数据信息
} host_t;               //上位机数据帧
#pragma pack (pop)

typedef union
{
  uint32_t Fqv_32;
  uint8_t Fqv_8[4];
}FQV_t;             //频率分段存储

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

    ~MainWindow();
private:
    void writeData(const QByteArray &data);

private slots:
    void on_Button_open_com_clicked(bool checked);
    void on_Button_clear_com_clicked(bool checked);
    void on_send_Button_clicked();
    void readData();
    void on_clear_Button_clicked();

    void on_pushButton_read_clicked();

    void on_pushButton_write_clicked();

    void on_pushButton_flash_clicked();

    void on_pushButton_Factory_clicked();

    void on_pushButton_MCU_clicked();

    void on_pushButton_INF_clicked();

    void on_pushButton_help_clicked();

    void on_pushButton_About_clicked();

    void on_pushButton_Rssi_clicked();

private:
    void showStatusMessage(const QString &message);
    void InitCombobox();
    void Par2Com(host_t &);
    bool Com2Par(host_t &);

    Ui::MainWindow *ui;
    QLabel *m_status = nullptr;
    QSerialPort *lora_serial = nullptr;
    QByteArray uart_buffer;
    host_t host_data;
protected:
    void String2Hex(QString str, QByteArray &senddata);
    char ConvertHexChar(char ch);
    int Find_EaEb(QByteArray src);
    bool isDigitString(const QString& src);

};

#endif // MAINWINDOW_H
