#include "Manager.h"
#include "Electronics.h"
#include <fstream>
#include <sstream>

using namespace std;

Manager::~Manager() {
    for (auto p : catalog) delete p; 
    catalog.clear();
}

void Manager::addProduct(BaseProduct* p) { 
    catalog.push_back(p);
    saveData();
}

void Manager::displayAll() const {
    if (catalog.empty()) {
        cout << "Inventory is currently empty. Please ask an admin to add products.\n";
        return;
    }
    for (auto p : catalog) p->displayDetails(); 
}

void Manager::deleteProduct(int id) {
    for (auto it = catalog.begin(); it != catalog.end(); ++it) {
        if ((*it)->getId() == id) {
            delete *it;
            catalog.erase(it);
            saveData();
            return;
        }
    }
}

double Manager::calculateTotal() const {
    double total = 0;
    for (auto p : catalog) total += p->getPrice();
    return total;
}

bool Manager::isEmpty() const {
    return catalog.empty();
}

void Manager::saveData() {
    ofstream out("catalog.txt");
    for (auto p : catalog) {
        out << p->getId() << "," << p->getName() << "," << p->getPrice() << "\n";
    }
}

void Manager::loadData() {
    ifstream in("catalog.txt");
    if (!in) return;
    string name, line; int id; double price;
    while (getline(in, line)) {
        stringstream ss(line);
        ss >> id; ss.ignore();
        getline(ss, name, ',');
        ss >> price;
        catalog.push_back(new Electronics(id, name, price, 12));
    }
} // Properly closed loadData here

// Defined separately, not inside loadData
BaseProduct* Manager::getProductById(int id) {
    for (auto p : catalog) {
        if (p->getId() == id) return p;
    }
    return nullptr;
}

const vector<BaseProduct*>& Manager::getProducts() const {
    return catalog;
}
