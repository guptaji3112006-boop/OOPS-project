#ifndef ELECTRONICS_H
#define ELECTRONICS_H
#include "BaseProduct.h"

class Electronics : public BaseProduct {
private:
    int warranty;
public:
    Electronics(int i, const std::string& n, double p, int w)
        : BaseProduct(i, n, p), warranty(w) {}

    void displayDetails() const override {
        std::cout << "[Electronics] ID: " << getId() << " | Name: " << getName()
                  << " | Price: ₹" << getPrice() << " | Warranty: " << warranty << "mo" << std::endl;
    }

    std::string getType() const override { return "Electronics"; }
    std::string getExtra() const override { return std::to_string(warranty); }
};
#endif