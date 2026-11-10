#ifndef ELECTRONICS_H
#define ELECTRONICS_H
#include "BaseProduct.h"

class Electronics : public BaseProduct {
private:
    int warranty;
public:
    Electronics(int i, std::string n, double p, int w) : BaseProduct(i, n, p), warranty(w) {}
    
    void displayDetails() override {
        std::cout << "[Electronics] ID: " << getId() << " | Name: " << getName() 
                  << " | Price: ₹" << getPrice() << " | Warranty: " << warranty << "mo" << std::endl;
    }
};
#endif
