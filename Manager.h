#ifndef MANAGER_H
#define MANAGER_H
#include <vector>
#include <string>
#include "BaseProduct.h"

class Manager {
private:
    std::vector<BaseProduct*> catalog;
public:
    Manager() = default;
    Manager(const Manager&) = delete;
    Manager& operator=(const Manager&) = delete;
    ~Manager();

    bool addProduct(BaseProduct* p);
    void displayAll() const;
    bool deleteProduct(int id);
    bool saveData() const;
    void loadData();
    BaseProduct* getProductById(int id);
    const BaseProduct* getProductById(int id) const;
    const std::vector<BaseProduct*>& getProducts() const;
    double getInventoryValue() const;
    double calculateTotal() const { return getInventoryValue(); }
    bool isEmpty() const;
};
#endif