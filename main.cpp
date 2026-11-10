#include <iostream>
#include <vector>
#include <fstream>
#include <cctype>
#include "Manager.h"
#include "Electronics.h"
#include "Clothing.h"
#include "ShoppingCart.h"
#include "BillGenerator.h"

using namespace std;


bool validateCard(string card) {
    if (card.length() != 16) return false;
    int sum = 0; bool alt = false;
    for (int i = card.length() - 1; i >= 0; i--) {
        if (!isdigit(static_cast<unsigned char>(card[i]))) return false;
        int d = card[i] - '0';
        if (alt) { d *= 2; if (d > 9) d -= 9; }
        sum += d; alt = !alt;
    }
    return (sum % 10 == 0);
}

int main() {
    Manager shop;
    shop.loadData(); 
    int roleChoice;

    while (true) {
        cout << "\n--- ROLE SELECTION ---\n1. Admin (Manage Shop)\n2. Customer (Buy Items)\n3. Exit\nChoice: ";
        if (!(cin >> roleChoice) || roleChoice == 3) {
            shop.saveData(); // 
            break;
        }

        try {
            if (roleChoice == 1) { // ADMIN ROLE
                int adminOption;
                cout << "\n[Admin] 1.Add Product 2.Delete 3.View All 4.Back: ";
                cin >> adminOption;

                if (adminOption == 1) {
                    int type, id; string name; double price;
                    cout << "Type (1:Electronics, 2:Clothing): "; cin >> type;
                    cout << "ID Name Price: "; cin >> id >> name >> price;

                    if (type != 1 && type != 2) throw runtime_error("Invalid product type.");
                    if (price <= 0) throw runtime_error("Price must be greater than zero.");
                    if (shop.getProductById(id) != nullptr) throw runtime_error("Product ID already exists.");

                    if (type == 1) {
                        int war; cout << "Warranty (mo): "; cin >> war;
                        shop.addProduct(new Electronics(id, name, price, war));
                    } else {
                        string size; cout << "Size (S/M/L): "; cin >> size;
                        shop.addProduct(new Clothing(id, name, price, size));
                    }
                    cout << "Product Added!\n";
                } else if (adminOption == 2) {
                    int id; cout << "Enter ID to delete: "; cin >> id;
                    shop.deleteProduct(id);
                } else if (adminOption == 3) {
                    shop.displayAll();
                }
            } 
            else if (roleChoice == 2) { // CUSTOMER ROLE
                cout << "\nWelcome to E-Shop! We are happy to serve you.\n";
                if (shop.isEmpty()) {
                    cout << "Sorry, our inventory is empty right now. Please check again later.\n";
                    continue;
                }
                ShoppingCart cart;
                int custOption;

                while (true) {
                    cout << "\n[Customer] 1.View Catalog 2.Add to Cart 3.Checkout/Pay 4.Back: ";
                    cin >> custOption;

                    if (custOption == 1) {
                        shop.displayAll();
                    } 
                    else if (custOption == 2) {
                        int id, quantity;
                        cout << "Enter Product ID to add: "; cin >> id;
                        cout << "Quantity: "; cin >> quantity;
                        BaseProduct* p = shop.getProductById(id);
                        if (p && quantity > 0) {
                            cart.addProduct(p, quantity);
                            cout << p->getName() << " added to cart!\n";
                        } else {
                            cout << (p == nullptr ? "Product not found!\n" : "Quantity must be greater than zero.\n");
                        }
                    }
                    else if (custOption == 3) {
                        if (cart.empty()) {
                            cout << "Cart is empty!\n";
                            continue;
                        }

                        cout << "Apply coupon (SAVE10/SAVE20, or NONE): ";
                        string coupon; cin >> coupon;
                        cart.applyDiscount(coupon == "NONE" ? "" : coupon);
                        cart.display();

                        cout << "Proceed to payment? (1:Yes, 2:No): ";
                        int payChoice; cin >> payChoice;

                        if (payChoice == 1) {
                            CustomerInfo customer;
                            cin.ignore(1000, '\n');
                            cout << "Customer name: "; getline(cin, customer.name);
                            cout << "Phone number: "; getline(cin, customer.phone);
                            cout << "Email: "; getline(cin, customer.email);

                            if (customer.name.empty() || customer.phone.empty() ||
                                customer.email.empty()) {
                                throw runtime_error("All customer details are required.");
                            }

                            string card;
                            cout << "Enter 16-digit Card Number: "; cin >> card;
                            
                            if (!validateCard(card)) throw runtime_error("Payment Denied: Invalid Card!");

                            if (!BillGenerator::generate(cart, customer, card))
                                throw runtime_error("Could not create receipt.txt");

                            cout << "Payment Successful! Thank you, " << customer.name
                                 << ". receipt.txt generated.\n";
                            cart.clear();
                            break; // Exit to role selection
                        }
                    }
                    else break; // Back to role selection
                }
            }
        } catch (const exception& e) {
            cout << "Error: " << e.what() << endl;
            cin.clear();
            cin.ignore(1000, '\n');
        }
    }
    return 0;
}
