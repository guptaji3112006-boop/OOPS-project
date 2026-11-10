#ifndef USER_H
#define USER_H
#include <string>
#include <vector>
#include "BaseProduct.h"

class User {
protected:
    std::string username;
public:
    User(std::string u) : username(u) {}
    virtual ~User() {}
    virtual void showMenu() = 0;
};

class Customer : public User {
private:
    std::vector<BaseProduct*> cart; // Requirement: Dynamic Memory usage
public:
    Customer(std::string u) : User(u) {}
    ~Customer() { for(auto p : cart) delete p; }
    void addToCart(BaseProduct* p) { cart.push_back(p); }
    void showMenu() override;
    double getCartTotal();
};

class Admin : public User {
public:
    Admin(std::string u) : User(u) {}
    void showMenu() override;
};
#endif
