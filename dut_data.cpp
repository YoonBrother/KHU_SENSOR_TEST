#include "dut_data.h"

dut_data::dut_data()
{
    sram_data = new QByteArray();
    raw_data = new QByteArray();
    selected_block = 0;
    data_avarible = false;
}

dut_data::~dut_data()
{

}

int dut_data::data_classify()
{
    bool b;
    int ret = 0;
    sram_data->clear();
    if (raw_data->at(0) == 'R') {
        sram_data->append(raw_data->mid(raw_data->length()-1,1));
        data_avarible = true;
        ret = 1;
    } else if (raw_data->at(0) == 'W') {
        sram_data->append(raw_data->mid(raw_data->length()-1,2));
        data_avarible = true;
        ret = 2;
    } else if (raw_data->at(0) == 't') {
        iTID[0] = raw_data->mid(1,4).toHex().toUInt();
        iTID[1] = raw_data->mid(5,4).toHex().toUInt(&b,16);
        iTID[2] = raw_data->mid(9,4).toHex().toUInt(&b,16);
        iTID[3] = raw_data->mid(13,4).toHex().toUInt(&b,16);
        iTID[4] = raw_data->mid(17,4).toHex().toUInt(&b,16);
        iTID[5] = raw_data->mid(21,4).toHex().toUInt(&b,16);
        iTID[6] = raw_data->mid(25,4).toHex().toUInt(&b,16);
        iTID[7] = raw_data->mid(29,4).toHex().toUInt(&b,16);
        iTID[8] = raw_data->mid(33,4).toHex().toUInt(&b,16);
        iTID[9] = raw_data->mid(37,4).toHex().toUInt(&b,16);
        iTID[10] = raw_data->mid(41,4).toHex().toUInt(&b,16);
        iTID[11] = raw_data->mid(45,4).toHex().toUInt(&b,16);
        qDebug("vang " + raw_data->mid(45,4).toUInt(&b,16));
        qDebug((raw_data->toHex()));
        ret = 3;
    } else  {
        sram_data->append(raw_data->mid(raw_data->length()-1,0));
        data_avarible = true;
        ret = 4;
    }

    //sram_data = raw_data;
    //data_avarible = true;
    return ret;
}

void dut_data::convert_data_to_image(QImage* img)
{
    char cdata,cmask;
    // data array
    for (int x = 0; x < 16; x++)
    {
        for (int y = 0; y < 256; y++)
        {
            if ((selected_block*16*256+x*256+y)<=(raw_data->length()-1)){
                cdata = raw_data->at(selected_block*16*256+x*256+y);
            }
            else {
                cdata = 0;
            }
            cmask = 0x01;
            for (int b = 0; b<8;b++)
            {
                if ((cdata&cmask)==cmask)
                {
                    img->setPixel(16*b+x, 255-y, qRgb(255, 255, 255));
                }
                else
                {
                    img->setPixel(16*b+x, 255-y, qRgb(0, 0, 0));
                }
                cmask = cmask<<1;
            }
        }
    }

}
