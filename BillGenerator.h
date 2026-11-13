#ifndef BILLGENERATOR_H
#define BILLGENERATOR_H

#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>
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
private:
     static const size_t WIDTH = 44;

     // UTF-8 me ₹ jaise akshar 3 bytes ke hote hain, isliye bytes nahi, akshar gino
     static size_t displayLen(const std::string &s)
     {
          size_t n = 0;
          for (unsigned char c : s)
               if ((c & 0xC0) != 0x80)
                    ++n;
          return n;
     }

     // Text ko width me fit karta hai; lamba ho to kaat ke "..." laga deta hai
     static std::string fit(const std::string &s, size_t width)
     {
          if (displayLen(s) <= width)
               return s;
          std::string out;
          size_t n = 0;
          for (size_t i = 0; i < s.size() && n + 3 < width;)
          {
               unsigned char c = static_cast<unsigned char>(s[i]);
               size_t len = c >= 0xF0 ? 4 : c >= 0xE0 ? 3 : c >= 0xC0 ? 2 : 1;
               out += s.substr(i, len);
               i += len;
               ++n;
          }
          return out + "...";
     }

     static std::string padRight(const std::string &s, size_t width)
     {
          std::string t = fit(s, width);
          size_t len = displayLen(t);
          return t + std::string(width > len ? width - len : 0, ' ');
     }

     static std::string padLeft(const std::string &s, size_t width)
     {
          size_t len = displayLen(s);
          return std::string(width > len ? width - len : 0, ' ') + s;
     }

     static std::string number(double v)
     {
          std::ostringstream o;
          o << std::fixed << std::setprecision(2) << v;
          return o.str();
     }

     static std::string money(double v) { return "\xE2\x82\xB9" + number(v); } // ₹

     // label left me, value right me, total WIDTH akshar
     static std::string row(const std::string &label, const std::string &value)
     {
          size_t used = displayLen(label) + displayLen(value);
          size_t gap = used < WIDTH ? WIDTH - used : 1;
          return label + std::string(gap, ' ') + value + "\n";
     }

public:
     static bool generate(const ShoppingCart &cart, const CustomerInfo &customer,
                          const std::string &cardNumber,
                          const std::string &fileName = "receipt.txt")
     {
          std::ofstream bill(fileName);
          if (!bill)
               return false;

          std::time_t now = std::time(nullptr);
          char dateText[32] = "";
          if (const std::tm *t = std::localtime(&now))
               std::strftime(dateText, sizeof(dateText), "%Y-%m-%d %H:%M:%S", t);

          const std::string line(WIDTH, '-');
          const std::string heavy(WIDTH, '=');

          bill << "=============== E-SHOP BILL ===============\n";
          bill << "Date: " << dateText << "\n";
          bill << "Customer: " << customer.name << "\n";
          bill << "Phone:    " << customer.phone << "\n";
          bill << "Email:    " << customer.email << "\n";
          bill << line << "\n";
          bill << padRight("Product", 24) << padLeft("Qty", 8) << padLeft("Price", 12) << "\n";
          for (const auto &item : cart.getItems())
          {
               bill << padRight(item.product->getName(), 24)
                    << padLeft(std::to_string(item.quantity), 8)
                    << padLeft(number(item.product->getPrice() * item.quantity), 12) << "\n";
          }
          bill << line << "\n";
          bill << row("Subtotal:", money(cart.getSubtotal()));
          bill << row("Discount (" + cart.getDiscountName() + "):", "-" + money(cart.getDiscountAmount()));
          bill << row("TOTAL:", money(cart.getTotal()));
          std::string lastFour = cardNumber.size() >= 4 ? cardNumber.substr(cardNumber.size() - 4) : std::string();
          bill << row("Card:", "****" + lastFour);
          bill << heavy << "\n";
          bill << "Thank you, " << customer.name << "! Visit us again.\n";

          bill.flush();
          return bill.good();
     }
};

#endif