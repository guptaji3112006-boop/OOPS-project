#include "Manager.h"
#include "Electronics.h"
#include "Clothing.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace std;

namespace {
vector<string> split(const string& s, char delim) {
    vector<string> parts;
    string item;
    stringstream ss(s);
    while (getline(ss, item, delim)) parts.push_back(item);
    return parts;
}
}

Manager::~Manager() {
    for (auto p : catalog) delete p;
    catalog.clear();
}

bool Manager::addProduct(BaseProduct* p) {
    if (!p) return false;
    if (p->getName().empty() || p->getPrice() <= 0 ||
        p->getName().find('|') != string::npos ||
        getProductById(p->getId()) != nullptr) {
        delete p;
        return false;
    }
    catalog.push_back(p);
    return saveData();
}

void Manager::displayAll() const {
    if (catalog.empty()) {
        cout << "Inventory is currently empty. Please ask an admin to add products.\n";
        return;
    }
    for (auto p : catalog) p->displayDetails();
}

bool Manager::deleteProduct(int id) {
    for (auto it = catalog.begin(); it != catalog.end(); ++it) {
        if ((*it)->getId() == id) {
            delete *it;
            catalog.erase(it);
            return saveData();
        }
    }
    return false;
}

double Manager::getInventoryValue() const {
    double total = 0;
    for (auto p : catalog) total += p->getPrice();
    return total;
}

bool Manager::isEmpty() const { return catalog.empty(); }

bool Manager::saveData() const {
    ofstream out("catalog.txt");
    if (!out) return false;
    out << fixed << setprecision(2);
    for (auto p : catalog) {
        out << p->getType() << '|' << p->getId() << '|' << p->getName()
            << '|' << p->getPrice() << '|' << p->getExtra() << '\n';
    }
    return out.good();
}

void Manager::loadData() {
    ifstream in("catalog.txt");
    if (!in) return;

    for (auto p : catalog) delete p;
    catalog.clear();

    string line;
    while (getline(in, line)) {
        if (line.empty()) continue;
        try {
            auto f = split(line, '|');
            if (f.size() == 5) {
                int id = stoi(f[1]);
                double price = stod(f[3]);
                if (f[2].empty() || price <= 0 || getProductById(id)) continue;
                if (f[0] == "Electronics")
                    catalog.push_back(new Electronics(id, f[2], price, stoi(f[4])));
                else if (f[0] == "Clothing")
                    catalog.push_back(new Clothing(id, f[2], price, f[4]));
            } else {
                auto g = split(line, ',');
                if (g.size() != 3) continue;
                int id = stoi(g[0]);
                double price = stod(g[2]);
                if (g[1].empty() || price <= 0 || getProductById(id)) continue;
                catalog.push_back(new Electronics(id, g[1], price, 12));
            }
        } catch (const exception&) {
        }
    }
}

BaseProduct* Manager::getProductById(int id) {
    for (auto p : catalog)
        if (p->getId() == id) return p;
    return nullptr;
}

const BaseProduct* Manager::getProductById(int id) const {
    for (auto p : catalog)
        if (p->getId() == id) return p;
    return nullptr;
}

const vector<BaseProduct*>& Manager::getProducts() const {
    return catalog;
}