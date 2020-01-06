#ifndef DUT_DATA_H
#define DUT_DATA_H

#include <QImage>

class dut_data
{
public:
    dut_data();
    ~dut_data();
    QByteArray *raw_data;
    int selected_block;
    bool data_avarible;
    void convert_data_to_image(QImage* img);
    int data_classify();


    uint iTID[12];
    int iWER[17];
    int iSER[17];


private:
    QByteArray *sram_data;

};

#endif // DUT_DATA_H
