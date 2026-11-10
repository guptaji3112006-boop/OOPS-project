#ifndef BASEPRODUCT_H
#define BASEPRODUCT_H

#include <string>
#include <iostream>

class BaseProduct {
private:
    int id;
    std::string name;
    double price;

public:
    BaseProduct(int i, std::string n, double p) : id(i), name(n), price(p) {}
    virtual ~BaseProduct() {}

   
    virtual void displayDetails() = 0; 

    int getId() const { return id; }
    std::string getName() const { return name; }
    double getPrice() const { return price; }
    void setPrice(double p) { if (p >= 0) price = p; }
    void setName(std::string n) { name = n; }
};

#endif
