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
    BaseProduct(int i, const std::string& n, double p) : id(i), name(n), price(p) {}
    virtual ~BaseProduct() {}

    virtual void displayDetails() const = 0;
    virtual std::string getType() const = 0;   // "Electronics" / "Clothing"
    virtual std::string getExtra() const = 0;  // warranty ya size, text me

    int getId() const { return id; }
    const std::string& getName() const { return name; }
    double getPrice() const { return price; }
    void setPrice(double p) { if (p > 0) price = p; }
    void setName(const std::string& n) { if (!n.empty()) name = n; }
};

#endif