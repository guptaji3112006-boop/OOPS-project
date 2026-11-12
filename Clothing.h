#ifndef CLOTHING_H
#define CLOTHING_H
#include "BaseProduct.h"

class Clothing : public BaseProduct
{
private:
    std::string size;

public:
    Clothing(int i, const std::string& n, double p, const std::string& s)
        : BaseProduct(i, n, p), size(s) {}

    void displayDetails() const override
    {
        std::cout << "[Clothing] ID: " << getId() << " | Name: " << getName()
                  << " | Price: ₹" << getPrice() << " | Size: " << size << std::endl;
    }

    std::string getType() const override { return "Clothing"; }
    std::string getExtra() const override { return size; }
};
#endif