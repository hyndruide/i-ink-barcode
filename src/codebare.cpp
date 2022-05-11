#include "codebare.hpp"



int _ean_number_A[10] = {
        0b0001101, //0
        0b0011001, //1
        0b0010011, //2
        0b0111101, //3
        0b0100011, //4
        0b0110001, //5
        0b0101111, //6
        0b0111011, //7
        0b0110111, //8
        0b0001011 //9
    };
int _ean13type[10] = {
    0b000000,
    0b001011,
    0b001101,
    0b001110,
    0b010011,
    0b011001,
    0b011100,
    0b010101,
    0b010110,
    0b011010
};

unsigned int reverseBits(unsigned int num)
{
    unsigned int NO_OF_BITS = 7;
    unsigned int reverse_num = 0;
    int i;
    for (i = 0; i < NO_OF_BITS; i++) {
        if ((num & (1 << i)))
            reverse_num |= 1 << ((NO_OF_BITS - 1) - i);
    }
    return reverse_num;
}

unsigned int invertBits(int num)
{
    int mask = 0b1111111;
    return num ^ mask;
}

BarCode::BarCode(eanTypes eantype)
{
    _eantype = eantype;
}

BarCode::~BarCode()
{
}

int BarCode::getNumberValue(int number, bool type)
{
    int num = _ean_number_A[number];
    if (type==0){
        return num;
    }
    num = invertBits(num);
    num = reverseBits(num);
    return num;
}

int BarCode::getNumberValueC(int number)
{
    int num = _ean_number_A[number];
    return invertBits(num);
}

int BarCode::getType()
{
    int n = (int) _ean[0] - 48;
    while (n >= 10)
        n /= 10;
    _type = _ean13type[n];
}

void BarCode::EanToBit(String ean, int* out)
{
    if (_eantype==Ean13)
    {
        _ean = ean;
        getType();
        out[0] = 0b101;
        int j = 1;
        for (int i = 5; i >= 0; i-=1) {
            char val = ean[j];
            out[j] = getNumberValue((int)val - 48,(_type & 1 << i) != 0);
            j++;
        }

        out[7] = 0b01010;
        for(int j = 8; j <= 12; j++) {
            char val = ean[j-1];
            Serial.print(val);
            out[j] = getNumberValueC((int)val - 48);
        }
        char val = ean[12];
        out[13] = getNumberValueC((int)val - 48);
        out[14] = 0b101;
    }
}
