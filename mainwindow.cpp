#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDateTime>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // UART FPGA
    uart_fpga_setting = new SettingsDialog;
    uart_fpga_setting->setdefault(0,3);
    uart_fpga = new QSerialPort(this);
    connect(uart_fpga, SIGNAL(error(QSerialPort::SerialPortError)), this,
            SLOT(uart_fpga_handleError(QSerialPort::SerialPortError)));
    connect(uart_fpga, SIGNAL(readyRead()), this, SLOT(uart_fpga_readData()));


    ui->pushButton_MPR_Write->setShortcut(QKeySequence(Qt::Key_Space));

    radiobutton_block_select_setup();

    // ADS
    setupCombobox_ADS_cmd();
    setupCombobox_ADS_reg();
    // add ECG chart
     series = new QLineSeries();
     ecg_data_list1.append(0);
     series->append(0, ecg_data_list1.at(0));
     series2 = new QLineSeries();
     ecg_data_list2.append(0);
     series2->append(0, ecg_data_list2.at(0));

     for (int var = 1; var < 200; ++var) {
         ecg_data_list1.append(9000000);
         series->append(var, ecg_data_list1.at(var));
         ecg_data_list2.append(9000000);
         series2->append(var, ecg_data_list2.at(var));
     }
     series->setName("CH1");
     series2->setName("CH2");
    chart = new QChart();
    chart->addSeries(series);
    chart->addSeries(series2);

    chart->legend()->show();
    chart->createDefaultAxes();
    chart->setTitle("ECG");

    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    ui->gridLayout_ADS_waveform->addWidget(chartView);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_app_close_clicked()
{
    QMessageBox msgBox;
    msgBox.setText("Quit program.");
    msgBox.setInformativeText("Do you want to leave?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    int ret = msgBox.exec();
    switch (ret) {
      case QMessageBox::Yes:
          // Yes was clicked
        QApplication::quit();
          break;
      case QMessageBox::Cancel:
          // Cancel was clicked
          break;
      default:
          // should never be reached
          break;
    }
}

void MainWindow::on_pushButton_uart_fpga_setting_clicked()
{
    uart_fpga_setting->show();
}

void MainWindow::radiobutton_block_select_setup()
{
//    connect(ui->radioButton_b0 ,SIGNAL(clicked()),this,SLOT(block_select_handle()));
//    connect(ui->radioButton_b1 ,SIGNAL(clicked()),this,SLOT(block_select_handle()));
//    connect(ui->radioButton_b2 ,SIGNAL(clicked()),this,SLOT(block_select_handle()));
//    connect(ui->radioButton_b3 ,SIGNAL(clicked()),this,SLOT(block_select_handle()));
}



void MainWindow::on_pushButton_uart_fpga_connect_clicked()
{
    if (ui->pushButton_uart_fpga_connect->text()=="Connect") // have not connected
    {
        SettingsDialog::Settings p = uart_fpga_setting->settings();
        uart_fpga->setPortName(p.name);
        uart_fpga->setBaudRate(p.baudRate);
        uart_fpga->setDataBits(p.dataBits);
        uart_fpga->setParity(p.parity);
        uart_fpga->setStopBits(p.stopBits);
        uart_fpga->setFlowControl(p.flowControl);
        if (uart_fpga->open(QIODevice::ReadWrite)) {
            ui->pushButton_uart_fpga_connect->setToolTip("Click to disconnect UART.");
            ui->pushButton_uart_fpga_connect->setText("Disconnect");
            ui->pushButton_uart_fpga_setting->setEnabled(false);
            ui->statusBar->showMessage(QString("Connected to %1 : %2, %3, %4, %5, %6")
                              .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                              .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));
        } else {
            QMessageBox::critical(this, tr("Error"), uart_fpga->errorString());
            ui->statusBar->showMessage("Open error");
        }
    }
    else // connected already
    {
        QMessageBox msgBox;
        msgBox.setText("Disconnect serial port");
        msgBox.setInformativeText("Are you sure?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        int ret = msgBox.exec();
        switch (ret) {
          case QMessageBox::Yes:
            // Yes was clicked--> disconnect
            if (uart_fpga->isOpen())
                uart_fpga->close();
            //console->setEnabled(false);
            ui->pushButton_uart_fpga_connect->setText("Connect");
            ui->pushButton_uart_fpga_connect->setToolTip("Click to connect UART.");
            ui->pushButton_uart_fpga_setting->setEnabled(true);
            ui->statusBar->showMessage("UART fpga disconnected");
              break;
          case QMessageBox::Cancel:
            // Cancel was clicked
            break;
          default:
            // should never be reached
            break;
        }
    }
}

void MainWindow::uart_fpga_readData()
{
    while (uart_fpga->waitForReadyRead(50)){
            //qDebug("data come 1");
            //raw_data->raw_data->append(uart_fpga->readAll());
    }
    QByteArray data = uart_fpga->readAll();
    QString str = QString(data);
    if (uart_fpga->error() == QSerialPort::ReadError) {
        qDebug("Failed to read from port "+uart_fpga->portName().toLatin1()+", error: "+uart_fpga->errorString().toLatin1());
    } else if (uart_fpga->error() == QSerialPort::TimeoutError && str.isEmpty()) {
        qDebug("No data was currently available for reading from port "+uart_fpga->portName().toLatin1());
    }

    //qDebug("Data successfully received from port "+uart_fpga->portName().toLatin1());
    ui->statusBar->showMessage("Data length "+QString::number(str.length(),10));


    // process the received data
    if (data.length()>1)
    {
        str = data.toHex(' ');
    }

    ui->plainTextEdit_console->clear();
    ui->plainTextEdit_console->insertPlainText(str);

    // process ADS ECG data from RDATAC function
    // first char is 0x66 = 'f'
    // 3 next byte is Status
    // 3 next byte is Channel 1
    // last 3 byte is Channel 2
    if (data.length()>=10)
    {
        if (data.at(0)==0x66)
        {
            int ecg1 = 0;
            ecg1 = (data.at(4)<<16)+(data.at(5)<<8)+data.at(6);
            int ecg2 = 0;
            ecg2 = (data.at(7)<<16)+(data.at(8)<<8)+data.at(9);
            ecg_data_list1.removeFirst();
            ecg_data_list1.append(ecg1);
            ecg_data_list2.removeFirst();
            ecg_data_list2.append(ecg2);
            for (int var = 0; var < ecg_data_list1.length(); var++) {
                series->replace(var,var, ecg_data_list1.at(var));
                series2->replace(var,var, ecg_data_list2.at(var));
            }
        }
    }
    // process data from MPR
    else if (data.length()>=3) {
        if ((data.at(0)==0x46)&(data.at(1)==0x00)) // received first touch status registor
        {
            ui->label_mpr_ch0_state->setText((data.at(2)&0x01)?"1":"0");
            ui->label_mpr_ch1_state->setText((data.at(2)&0x02)?"1":"0");
            ui->label_mpr_ch2_state->setText((data.at(2)&0x04)?"1":"0");
            ui->label_mpr_ch3_state->setText((data.at(2)&0x08)?"1":"0");
            ui->label_mpr_ch4_state->setText((data.at(2)&0x10)?"1":"0");
            ui->label_mpr_ch5_state->setText((data.at(2)&0x20)?"1":"0");
            ui->label_mpr_ch6_state->setText((data.at(2)&0x40)?"1":"0");
            ui->label_mpr_ch7_state->setText((data.at(2)&0x80)?"1":"0");
            // read next registor
            readMPR("01");

        }
        else if ((data.at(0)==0x52)&(data.at(1)==0x01)) // received second touch status registor
        {
            ui->label_mpr_ch8_state->setText((data.at(2)&0x01)?"1":"0");
            ui->label_mpr_ch9_state->setText((data.at(2)&0x02)?"1":"0");
            ui->label_mpr_ch10_state->setText((data.at(2)&0x04)?"1":"0");
            ui->label_mpr_ch11_state->setText((data.at(2)&0x08)?"1":"0");
        }
    }
}

void MainWindow::uart_fpga_writeData(const QByteArray &data)
{
    uart_fpga->write(data);
}

void MainWindow::uart_fpga_handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
    QMessageBox::critical(this, QString("Critical Error"), uart_fpga->errorString());
    }
}

void MainWindow::configMPR(QString reg, QString value)
{
    QByteArray addr = QByteArray::fromHex(reg.toUtf8());
    QByteArray data = QByteArray::fromHex(value.toUtf8());
    QString str = "W"; // Write data to a register of MPR121
    QByteArray dout = str.toLocal8Bit();
    dout.append(addr);
    dout.append(data);
    qDebug() << "Config MPR: " << dout <<" length: "<< dout.length();;
    uart_fpga_writeData(dout);
    QThread::msleep(50);
}

void MainWindow::readMPR(QString reg)
{
    QByteArray addr = QByteArray::fromHex(reg.toUtf8());
    QByteArray number = QByteArray::fromHex("01");
    QString str = "R"; // Read value of a register of MPR121
    QByteArray dout = str.toLocal8Bit();
    dout.append(addr);
    dout.append(number);
    qDebug() << "Request to READ out from MPR: " << str <<" length: "<< str.toLatin1().length();
    uart_fpga_writeData(dout);
}

void MainWindow::on_pushButton_MPR_Write_clicked()
{
    QByteArray addr = QByteArray::fromHex(ui->lineEdit_register_address->text().toUtf8());
    QByteArray data = QByteArray::fromHex(ui->lineEdit_data_in->text().toUtf8());
    QString str = "W"; // Write data to a register of MPR121
    QByteArray dout = str.toLocal8Bit();
    dout.append(addr);
    dout.append(data);
    qDebug() << "Write out to MPR: " << dout <<" length: "<< dout.length();;
    uart_fpga_writeData(dout);
}

void MainWindow::on_pushButton_MPR_Read_clicked()
{
    QByteArray addr = QByteArray::fromHex(ui->lineEdit_register_address->text().toUtf8());
    QByteArray number = QByteArray::fromHex("01");
    QString str = "R"; // Read value of a register of MPR121
    QByteArray dout = str.toLocal8Bit();
    dout.append(addr);
    dout.append(number);
    qDebug() << "Request to READ out from MPR: " << str <<" length: "<< str.toLatin1().length();
    uart_fpga_writeData(dout);
}

void MainWindow::on_pushButton_test_FPGA_clicked()
{
    QByteArray addr = QByteArray::fromHex(ui->lineEdit_register_address->text().toLatin1());
    QByteArray number = QByteArray::fromHex("01");
    QString str = "T"+addr+number;
    qDebug() << "Request to test char from FPGA: " << str <<" length: "<< str.toLatin1().length();
    uart_fpga_writeData(str.toLocal8Bit());
}

void MainWindow::on_pushButton_MPR_AllDigital_clicked()
{
    QByteArray addr = QByteArray::fromHex("01");
    QByteArray number = QByteArray::fromHex("01");
    QString str = "B"; // config MPR121 all channel to Binary (on/off level only)
    QByteArray dout = str.toLocal8Bit();
    dout.append(addr);
    dout.append(number);
    qDebug() << "Set MPR channel to binary";
    uart_fpga_writeData(dout);
}

void MainWindow::on_pushButton_MPR_AllAnalog_clicked()
{
    QByteArray addr = QByteArray::fromHex("01");
    QByteArray number = QByteArray::fromHex("01");
    QString str = "A"; // config MPR121 all channel to Analog
    QByteArray dout = str.toLocal8Bit();
    dout.append(addr);
    dout.append(number);
    qDebug() << "Set MPR channel to analog";
    uart_fpga_writeData(dout);
}

void MainWindow::on_pushButton_MPR_StartStream_clicked()
{
    // config sample rate

    // AFE Configuration 1 Register (0x5C) = 1101 0000
    // FFI = 11 Encoding 3 – Sets samples taken to 34 for first filter
    configMPR("5C","d0");
    // AFE Configuration 2 Register (0x5D), = 001 11 101 = 3d
    // SFI = 11 Encoding 3 – Number of samples is set to 18 for second filter
    // ESI = 101 Encoding 5 – Period set to 32 ms
    configMPR("5D","3d");
    configMPR("5D","3d");
    configMPR("5D","3d");
    configMPR("5D","3d");
    // start all channel both proximity and measurement
    // Electrode Configuration Register (ECR, 0x5E)
    configMPR("5E","3f");
    configMPR("5E","3f");
    configMPR("5E","3f");
    configMPR("5E","3f");
}

void MainWindow::on_pushButton_MPR_StopStream_clicked()
{
    configMPR("5E","00");
}
/*********** For ADS command *************************/
void MainWindow::setupCombobox_ADS_cmd()
{
    //WAKEUP 	Wake-up from standby mode (02h)
    //STANDBY Enter standby mode (04h)
    //RESET 	Reset the device (06h)
    //START 	Start or restart (synchronize) conversions (08h)
    //STOP 	Stop conversion (0Ah)
    ////OFFSETCAL Channel offset calibration (1Ah)

    //SDATAC Stop Read Data Continuously mode 0001 0001 (11h)
    ui->comboBox_ADS_cmd->addItem("WAKEUP");
    ui->comboBox_ADS_cmd->addItem("STANDBY");
    ui->comboBox_ADS_cmd->addItem("START");
    ui->comboBox_ADS_cmd->addItem("STOP");
    ui->comboBox_ADS_cmd->addItem("RESET");
    ui->comboBox_ADS_cmd->addItem("RDATAC");
    ui->comboBox_ADS_cmd->addItem("SDATAC");
    ui->comboBox_ADS_cmd->addItem("RDATA");
    ui->comboBox_ADS_cmd->setCurrentIndex(6);
}
QString MainWindow::decode_ADS_cmd(QString cmd)
{
    QString hex="";
    if (cmd=="WAKEUP") hex = "02";
    if (cmd=="STANDBY") hex = "04";
    if (cmd=="RESET") hex = "06";
    if (cmd=="START") hex = "08";
    if (cmd=="STOP") hex = "0A";
    if (cmd=="RDATAC") hex = "10";
    if (cmd=="SDATAC") hex = "11";
    if (cmd=="RDATA") hex = "12";
    return hex;
}

/*********** For ADS register *************************/
void MainWindow::setupCombobox_ADS_reg()
{
    //WAKEUP 	Wake-up from standby mode (02h)
    //STANDBY Enter standby mode (04h)
    //RESET 	Reset the device (06h)
    //START 	Start or restart (synchronize) conversions (08h)
    //STOP 	Stop conversion (0Ah)
    ////OFFSETCAL Channel offset calibration (1Ah)

    //SDATAC Stop Read Data Continuously mode 0001 0001 (11h)
    ui->comboBox_ADS_reg->addItem("ID");
    ui->comboBox_ADS_reg->addItem("CONFIG1");
    ui->comboBox_ADS_reg->addItem("CONFIG2");
    ui->comboBox_ADS_reg->addItem("LOFF");
    ui->comboBox_ADS_reg->addItem("CH1SET");
    ui->comboBox_ADS_reg->addItem("CH2SET");
    ui->comboBox_ADS_reg->addItem("RLD_SENS");
    ui->comboBox_ADS_reg->addItem("LOFF_SENS");
    ui->comboBox_ADS_reg->addItem("LOFF_STAT");
    ui->comboBox_ADS_reg->addItem("RESP1");
    ui->comboBox_ADS_reg->addItem("RESP2");
    ui->comboBox_ADS_reg->addItem("GPIO");
    ui->comboBox_ADS_reg->setCurrentIndex(1);
}
QString MainWindow::decode_ADS_reg(QString cmd)
{
    QString hex="";
    if (cmd=="ID") hex = "00";
    if (cmd=="CONFIG1") hex = "01";
    if (cmd=="CONFIG2") hex = "02";
    if (cmd=="LOFF") hex = "03";
    if (cmd=="CH1SET") hex = "04";
    if (cmd=="CH2SET") hex = "05";
    if (cmd=="RLD_SENS") hex = "06";
    if (cmd=="LOFF_SENS") hex = "07";
    if (cmd=="LOFF_STAT") hex = "08";
    if (cmd=="RESP1") hex = "09";
    if (cmd=="RESP2") hex = "0A";
    if (cmd=="GPIO") hex = "0B";
    return hex;
}

void MainWindow::on_pushButton_ADS_Write_clicked()
{
    QString addrstr = decode_ADS_reg(ui->comboBox_ADS_reg->currentText());
    QByteArray addr = QByteArray::fromHex(addrstr.toUtf8());
    QByteArray data = QByteArray::fromHex(ui->lineEdit_din_ads->text().toUtf8());
    QString str = "w"; // Write data to a register of ADS1292
    QByteArray dout = str.toLocal8Bit();
    dout.append(addr);
    dout.append(data);
    qDebug() << "Write out to ADS: " << dout <<" length: "<< dout.length();;
    uart_fpga_writeData(dout);
}

void MainWindow::on_pushButton_ADS_Read_clicked()
{
    QString addrstr = decode_ADS_reg(ui->comboBox_ADS_reg->currentText());
    QByteArray addr = QByteArray::fromHex(addrstr.toUtf8());
    QByteArray number = QByteArray::fromHex(ui->lineEdit_din_ads->text().toUtf8());
    QString str = "r"; // Read value of a register of ADS1292
    QByteArray dout = str.toLocal8Bit();
    dout.append(addr);
    dout.append(number);
    qDebug() << "Request to READ out from ADS: " << dout <<" length: "<< dout.length();
    uart_fpga_writeData(dout);
}

void MainWindow::on_pushButton_ADS_cmd_clicked()
{
    QString cmd = decode_ADS_cmd(ui->comboBox_ADS_cmd->currentText());
    QByteArray cmdhex = QByteArray::fromHex(cmd.toUtf8());
    QByteArray dummy = QByteArray::fromHex("01");
    QString str = "c"; // a command is send to ADS1292
    QByteArray dout = str.toLocal8Bit();
    dout.append(cmdhex);
    dout.append(dummy);
    qDebug() << "command to ADS: " << dout <<" length: "<< dout.length();
    uart_fpga_writeData(dout);
}

void MainWindow::on_pushButton_ADS_RDATAC_clicked()
{
    QByteArray cmdhex = QByteArray::fromHex("10");
    QByteArray dummy = QByteArray::fromHex(ui->lineEdit_din_ads->text().toUtf8());
    QString str = "e"; // a command is send to ADS1292
    QByteArray dout = str.toLocal8Bit();
    dout.append(cmdhex);
    dout.append(dummy);
    qDebug() << "ADS read data continue: " << dout <<" length: "<< dout.length();
    uart_fpga_writeData(dout);
}

void MainWindow::configADS(QString reg,QString value)
{
    QString addrstr = decode_ADS_reg(reg);
    QByteArray addr = QByteArray::fromHex(addrstr.toUtf8());
    QByteArray data = QByteArray::fromHex(value.toUtf8());
    QString str = "w"; // Write data to a register of ADS1292
    QByteArray dout = str.toLocal8Bit();
    dout.append(addr);
    dout.append(data);
    qDebug() << "auto config ADS: " << dout <<" length: "<< dout.length();;
    uart_fpga_writeData(dout);
    QThread::msleep(50);
}

void MainWindow::on_pushButton_ADS_autoconfig_clicked()
{
    configADS("CONFIG1","01");
    configADS("CONFIG2","E0");
    configADS("LOFF","10");
    configADS("CH1SET","00");
    configADS("CH2SET","00");
    configADS("RLD_SENS","2C");
    configADS("LOFF_SENS","0E");
    configADS("LOFF_STAT","4F");
    configADS("RESP1","EA");
    configADS("RESP2","03");
    configADS("GPIO","00");
}
