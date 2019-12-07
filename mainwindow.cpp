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
     QLineSeries *series = new QLineSeries();
     series->append(0, 6);
     series->append(2, 4);
     series->append(3, 8);
     series->append(7, 4);
     series->append(10, 5);
    chart = new QChart();
    chart->legend()->hide();
    chart->addSeries(series);
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

    qDebug("Data successfully received from port "+uart_fpga->portName().toLatin1());
    ui->statusBar->showMessage("Data length "+QString::number(str.length(),10));


    // process the received data
    if (data.length()>1)
    {
        str = data.toHex(' ');
    }

    ui->plainTextEdit_console->clear();
    ui->plainTextEdit_console->insertPlainText(str);
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

//void MainWindow::on_pushButton_DUT_SRAM_Read_clicked()
//{
//    QString str = "U"; // Read all data from memory (DUT)
//    uart_fpga_writeData(str.toLocal8Bit());
//    flag_kind_ER = 'R';
//}





//void MainWindow::on_pushButton_DUT_SRAM_Write_00_clicked()
//{
//    QString str = "0"; // Write data to memory (DUT)
//    uart_fpga_writeData(str.toLocal8Bit());
//    flag_data_pattern = 0b00000000;
//    flag_kind_ER = 'W';
//}



void MainWindow::on_pushButton_MPR_Write_clicked()
{
    bool ok = false;
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
    QByteArray state = QByteArray::fromHex("01");
    QByteArray period = QByteArray::fromHex("01"); // 10 mini second
    QString str = "S"; // config MPR121 to start Streamming data
    QByteArray dout = str.toLocal8Bit();
    dout.append(state);
    dout.append(period);
    qDebug() << "Set MPR start streamming";
    uart_fpga_writeData(dout);
}

void MainWindow::on_pushButton_MPR_StopStream_clicked()
{
    QByteArray state = QByteArray::fromHex("00");
    QByteArray number = QByteArray::fromHex("00");
    QString str = "S"; // config MPR121 to stop Streamming data
    QByteArray dout = str.toLocal8Bit();
    dout.append(state);
    dout.append(number);
    qDebug() << "Set MPR stop streamming";
    uart_fpga_writeData(dout);
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
