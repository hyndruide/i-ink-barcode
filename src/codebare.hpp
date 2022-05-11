
#include <Arduino.h>

enum eanTypes{Ean13 = 13, Ean8 = 8, Ean128= 128};

class BarCode
{
private:
    eanTypes _eantype;
    String _ean;
    int _type;
    int getNumberValue(int number, bool type);
    int getNumberValueC(int number);
    int getType();
public:
    BarCode(eanTypes Type);
    ~BarCode();
    void EanToBit(String ean, int* out);
};

unsigned int reverseBits(unsigned int num);
unsigned int invertBits(int num);