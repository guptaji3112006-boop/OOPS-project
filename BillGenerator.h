#ifndef BILLGENERATOR_H
#define BILLGENERATOR_H

#include <ctime>
#include <fstream>
#include <iomanip>
#include <string>
#include "ShoppingCart.h"

struct CustomerInfo
{
     std::string name;
     std::string phone;
     std::string email;
};

class BillGenerator
{
public:
     static bool generate(const ShoppingCart &cart, const CustomerInfo &customer,
                          const std::string &cardNumber,
                          const std::string &fileName = "receipt.txt")
     {
          std::ofstream bill(fileName);
          if (!bill)
               return false;

          std::time_t now = std::time(nullptr);
          bill << "=============== E-SHOP BILL ===============\n";
          bill << "Date: " << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S") << "\n";
          bill << "Customer: " << customer.name << "\n";
          bill << "Phone:    " << customer.phone << "\n";
          bill << "Email:    " << customer.email << "\n";
          bill << "--------------------------------------------\n";
          bill << std::left << std::setw(22) << "Product"
               << std::right << std::setw(8) << "Qty"
               << std::setw(12) << "Price" << "\n";
          for (const auto &item : cart.getItems())
          {
               bill << std::left << std::setw(22) << item.product->getName()
                    << std::right << std::setw(8) << item.quantity
                    << std::setw(12) << std::fixed << std::setprecision(2)
                    << item.product->getPrice() * item.quantity << "\n";
          }
          bill << "--------------------------------------------\n";
          bill << "Subtotal:                       ₹" << cart.getSubtotal() << "\n";
          bill << "Discount (" << cart.getDiscountName() << "):          -₹"
               << cart.getDiscountAmount() << "\n";
          bill << "TOTAL:                         ₹" << cart.getTotal() << "\n";
          bill << "Card:                           ****"
               << cardNumber.substr(cardNumber.size() - 4) << "\n";
          bill << "============================================\n";
          bill << "Thank you, " << customer.name << "! Visit us again.\n";
          return true;
     }
};

#endif
