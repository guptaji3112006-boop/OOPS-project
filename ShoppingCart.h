#ifndef SHOPPINGCART_H
#define SHOPPINGCART_H

#include <algorithm>
#include <cctype>
#include <cmath>
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
    std::string coupon;

    struct Discount {
        double percent;
        std::string name;
    };

    static std::string normalize(const std::string& s) {
        size_t start = 0, end = s.size();
        while (start < end && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
        while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;
        std::string out = s.substr(start, end - start);
        std::transform(out.begin(), out.end(), out.begin(),
                       [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
        return out;
    }

    Discount computeDiscount() const {
        const double subtotal = getSubtotal();
        if (coupon == "SAVE10") return {10.0, "SAVE10 coupon"};
        if (coupon == "SAVE20" && subtotal >= 1000.0) return {20.0, "SAVE20 coupon"};
        if (subtotal >= 1000.0) return {15.0, "Automatic 15% discount"};
        if (subtotal >= 500.0) return {10.0, "Automatic 10% discount"};
        return {0.0, "No discount"};
    }

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

    // Coupon yaad rakhta hai; discount har baar taaza hisaab se nikalta hai.
    void applyDiscount(const std::string& c) { coupon = normalize(c); }

    double getDiscountPercent() const { return computeDiscount().percent; }
    std::string getDiscountName() const { return computeDiscount().name; }

    double getDiscountAmount() const {
        double amount = getSubtotal() * computeDiscount().percent / 100.0;
        return std::round(amount * 100.0) / 100.0;
    }

    double getTotal() const {
        return getSubtotal() - getDiscountAmount();
    }

    void display() const {
        std::cout << "\n--- YOUR CART ---\n";
        for (const auto& item : items) {
            std::cout << item.product->getName() << " x " << item.quantity
                      << " = ₹" << std::fixed << std::setprecision(2)
                      << item.product->getPrice() * item.quantity << "\n";
        }
        std::cout << "Subtotal: ₹" << getSubtotal() << "\n"
                  << "Discount (" << getDiscountName() << "): -₹"
                  << getDiscountAmount() << "\n"
                  << "Total: ₹" << getTotal() << "\n";
    }

    void clear() {
        items.clear();
        coupon.clear();
    }
};

#endif