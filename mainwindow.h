#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include "uart/settingsdialog.h"
#include <QTimer>
#include <QtCharts>
#include <QtCharts/QLineSeries>
#include <QDebug>
#include <QThread>

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
    void on_pushButton_app_close_clicked();

    void on_pushButton_uart_fpga_setting_clicked();

    void on_pushButton_uart_fpga_connect_clicked();

    void uart_fpga_readData();

    void uart_fpga_handleError(QSerialPort::SerialPortError error);




    void on_pushButton_MPR_Write_clicked();

    void on_pushButton_MPR_Read_clicked();

    void on_pushButton_test_FPGA_clicked();

    void on_pushButton_MPR_AllDigital_clicked();

    void on_pushButton_MPR_AllAnalog_clicked();

    void on_pushButton_MPR_StartStream_clicked();

    void on_pushButton_MPR_StopStream_clicked();

    void on_pushButton_ADS_Write_clicked();

    void on_pushButton_ADS_Read_clicked();

    void on_pushButton_ADS_cmd_clicked();

    void on_pushButton_ADS_RDATAC_clicked();

    void on_pushButton_ADS_autoconfig_clicked();

private:
    Ui::MainWindow *ui;
    SettingsDialog *uart_fpga_setting;
    QSerialPort *uart_fpga;
    char flag_kind_ER;

    int flag_data_pattern;

    QLineSeries *series;
    QLineSeries *series2;
    QChartView *chartView;
    QChart *chart;

    QList<int> ecg_data_list1;
    QList<int> ecg_data_list2;

    void uart_fpga_writeData(const QByteArray &data);
    void radiobutton_block_select_setup();

    void setupCombobox_ADS_cmd();
    QString decode_ADS_cmd(QString cmd);
    void setupCombobox_ADS_reg();
    QString decode_ADS_reg(QString cmd);

    void configADS(QString reg,QString value);
    void configMPR(QString reg,QString value);

    void readMPR(QString reg);
    void readMPR_2(QString reg);
    void on_pushButton_MPR_autoconfig_clicked();


};

#endif // MAINWINDOW_H
