#ifndef SHOPPINGCART_H
#define SHOPPINGCART_H

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include "BaseProduct.h"

struct CartItem {
    BaseProduct* product;
    int quantity;
};

class ShoppingCart {
private:
    std::vector<CartItem> items;
    double discountPercent = 0.0;
    std::string discountName = "No discount";

public:
    void addProduct(BaseProduct* product, int quantity = 1) {
        if (product == nullptr || quantity <= 0) return;

        for (auto& item : items) {
            if (item.product->getId() == product->getId()) {
                item.quantity += quantity;
                return;
            }
        }
        items.push_back({product, quantity});
    }

    bool empty() const { return items.empty(); }

    const std::vector<CartItem>& getItems() const { return items; }

    double getSubtotal() const {
        double subtotal = 0.0;
        for (const auto& item : items) {
            subtotal += item.product->getPrice() * item.quantity;
        }
        return subtotal;
    }

    void applyDiscount(const std::string& coupon) {
        const double subtotal = getSubtotal();
        discountPercent = 0.0;
        discountName = "No discount";

        if (coupon == "SAVE10") {
            discountPercent = 10.0;
            discountName = "SAVE10 coupon";
        } else if (coupon == "SAVE20" && subtotal >= 1000.0) {
            discountPercent = 20.0;
            discountName = "SAVE20 coupon";
        } else if (subtotal >= 1000.0) {
            discountPercent = 15.0;
            discountName = "Automatic 15% discount";
        } else if (subtotal >= 500.0) {
            discountPercent = 10.0;
            discountName = "Automatic 10% discount";
        }
    }

    double getDiscountAmount() const {
        return getSubtotal() * discountPercent / 100.0;
    }

    double getTotal() const {
        return getSubtotal() - getDiscountAmount();
    }

    double getDiscountPercent() const { return discountPercent; }
    std::string getDiscountName() const { return discountName; }

    void display() const {
        std::cout << "\n--- YOUR CART ---\n";
        for (const auto& item : items) {
            std::cout << item.product->getName() << " x " << item.quantity
                      << " = ₹" << std::fixed << std::setprecision(2)
                      << item.product->getPrice() * item.quantity << "\n";
        }
        std::cout << "Subtotal: ₹" << getSubtotal() << "\n"
                  << "Discount (" << discountName << "): -₹"
                  << getDiscountAmount() << "\n"
                  << "Total: ₹" << getTotal() << "\n";
    }

    void clear() {
        items.clear();
        discountPercent = 0.0;
        discountName = "No discount";
    }
};

#endif
