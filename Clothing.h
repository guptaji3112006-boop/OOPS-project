#ifndef CLOTHING_H
#define CLOTHING_H
#include "BaseProduct.h"

class Clothing : public BaseProduct
{
private:
    std::string size;

public:
    Clothing(int i, std::string n, double p, std::string s) : BaseProduct(i, n, p), size(s) {}

    void displayDetails() override
    {
        std::cout << "[Clothing] ID: " << getId() << " | Name: " << getName()
                  << " | Price: ₹" << getPrice() << " | Size: " << size << std::endl;
    }
};
#endif
