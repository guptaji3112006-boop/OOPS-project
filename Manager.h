#ifndef MANAGER_H
#define MANAGER_H
#include <vector>
#include <string>
#include "BaseProduct.h"

class Manager {
private:
    std::vector<BaseProduct*> catalog; 
public:
    ~Manager(); 
    void addProduct(BaseProduct* p);
    void displayAll() const; 
    void deleteProduct(int id);
    void saveData(); 
    void loadData();
    BaseProduct* getProductById(int id);
    double calculateTotal() const;
    bool isEmpty() const;
};
#endif
