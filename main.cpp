#include <algorithm>
#include <cctype>
#include <iostream>
#include <stdexcept>
#include <string>
#include "Manager.h"
#include "Electronics.h"
#include "Clothing.h"
#include "ShoppingCart.h"
#include "BillGenerator.h"

using namespace std;

// Input khatam hone par (Ctrl+Z / Ctrl+D) program saaf tareeke se band karne ke liye
struct InputEnded {};

// ---------- input helpers ----------
static string trim(const string &s)
{
    size_t a = 0, b = s.size();
    while (a < b && isspace(static_cast<unsigned char>(s[a]))) ++a;
    while (b > a && isspace(static_cast<unsigned char>(s[b - 1]))) --b;
    return s.substr(a, b - a);
}

static string toUpper(string s)
{
    transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(toupper(c)); });
    return s;
}

static string readLine(const string &prompt)
{
    cout << prompt;
    string line;
    if (!getline(cin, line)) throw InputEnded();
    return trim(line);
}

static int readInt(const string &prompt)
{
    while (true)
    {
        string text = readLine(prompt);
        try
        {
            size_t used = 0;
            int value = stoi(text, &used);
            if (used == text.size()) return value;
        }
        catch (...) {}
        cout << "Please enter a whole number.\n";
    }
}

static double readDouble(const string &prompt)
{
    while (true)
    {
        string text = readLine(prompt);
        try
        {
            size_t used = 0;
            double value = stod(text, &used);
            if (used == text.size()) return value;
        }
        catch (...) {}
        cout << "Please enter a valid number.\n";
    }
}

static bool validateCard(const string &card)
{
    if (card.length() != 16) return false;
    int sum = 0;
    bool alt = false;
    for (int i = static_cast<int>(card.length()) - 1; i >= 0; i--)
    {
        if (!isdigit(static_cast<unsigned char>(card[i]))) return false;
        int d = card[i] - '0';
        if (alt)
        {
            d *= 2;
            if (d > 9) d -= 9;
        }
        sum += d;
        alt = !alt;
    }
    return sum % 10 == 0;
}

// ---------- Admin ----------
static void adminMenu(Manager &shop)
{
    while (true)
    {
        int option = readInt("\n[Admin] 1.Add Product 2.Delete 3.View All 4.Back: ");
        if (option == 4) return;

        try
        {
        if (option == 1)
        {
            int type = readInt("Type (1:Electronics, 2:Clothing): ");
            if (type != 1 && type != 2) throw runtime_error("Invalid product type.");

            int id = readInt("ID: ");
            if (id <= 0) throw runtime_error("ID must be a positive number.");
            if (shop.getProductById(id) != nullptr) throw runtime_error("Product ID already exists.");

            string name = readLine("Name: ");
            if (name.empty()) throw runtime_error("Product name is required.");
            if (name.find('|') != string::npos) throw runtime_error("The character | is not allowed.");

            double price = readDouble("Price: ");
            if (!(price > 0 && price < 1e9)) throw runtime_error("Price must be greater than zero.");

            // Product tabhi banta hai jab saari jaanch pass ho jaye (memory leak nahi)
            BaseProduct *product = nullptr;
            if (type == 1)
            {
                int warranty = readInt("Warranty (months): ");
                if (warranty < 0 || warranty > 600) throw runtime_error("Warranty must be 0 to 600 months.");
                product = new Electronics(id, name, price, warranty);
            }
            else
            {
                string size = readLine("Size (S/M/L): ");
                if (size.empty() || size.find('|') != string::npos) throw runtime_error("Invalid size.");
                product = new Clothing(id, name, price, size);
            }

            // addProduct() product ka malik ban jata hai; fail hone par khud delete karta hai.
            if (shop.addProduct(product)) cout << "Product Added!\n";
            else cout << "Could not add the product.\n";
        }
        else if (option == 2)
        {
            int id = readInt("Enter ID to delete: ");
            if (shop.deleteProduct(id)) cout << "Product deleted.\n";
            else cout << "Product not found, or the catalog could not be saved.\n";
        }
        else if (option == 3)
        {
            shop.displayAll();
        }
        else
        {
            cout << "Invalid option.\n";
        }
        }
        catch (const runtime_error &e)
        {
            cout << "Error: " << e.what() << endl;
        }
    }
}

// ---------- Customer ----------
static void customerMenu(Manager &shop)
{
    cout << "\nWelcome to E-Shop! We are happy to serve you.\n";
    if (shop.isEmpty())
    {
        cout << "Sorry, our inventory is empty right now. Please check again later.\n";
        return;
    }

    ShoppingCart cart;
    while (true)
    {
        int option = readInt("\n[Customer] 1.View Catalog 2.Add to Cart 3.Checkout/Pay 4.Back: ");

        try
        {
        if (option == 1)
        {
            shop.displayAll();
        }
        else if (option == 2)
        {
            int id = readInt("Enter Product ID to add: ");
            int quantity = readInt("Quantity: ");
            BaseProduct *p = shop.getProductById(id);
            if (p == nullptr) cout << "Product not found!\n";
            else if (quantity < 1 || quantity > 1000) cout << "Quantity must be between 1 and 1000.\n";
            else
            {
                cart.addProduct(p, quantity);
                cout << p->getName() << " added to cart!\n";
            }
        }
        else if (option == 3)
        {
            if (cart.empty())
            {
                cout << "Cart is empty!\n";
                continue;
            }

            string coupon = readLine("Apply coupon (SAVE10/SAVE20, or NONE): ");
            cart.applyDiscount(toUpper(coupon) == "NONE" ? "" : coupon);
            cart.display();

            int pay = readInt("Proceed to payment? (1:Yes, 2:No): ");
            if (pay != 1) continue;

            CustomerInfo customer;
            customer.name = readLine("Customer name: ");
            customer.phone = readLine("Phone number: ");
            customer.email = readLine("Email: ");
            if (customer.name.empty() || customer.phone.empty() || customer.email.empty())
                throw runtime_error("All customer details are required.");

            string card = readLine("Enter 16-digit Card Number: ");
            if (!validateCard(card)) throw runtime_error("Payment Denied: Invalid Card!");

            if (!BillGenerator::generate(cart, customer, card))
                throw runtime_error("Could not create receipt.txt");

            cout << "Payment Successful! Thank you, " << customer.name
                 << ". receipt.txt generated.\n";
            cart.clear();
            return; // role selection par wapas
        }
        else if (option == 4)
        {
            return;
        }
        else
        {
            cout << "Invalid option.\n";
        }
        }
        catch (const runtime_error &e)
        {
            cout << "Error: " << e.what() << endl;
        }
    }
}

int main()
{
    Manager shop;
    shop.loadData();

    try
    {
        while (true)
        {
            int role = readInt("\n--- ROLE SELECTION ---\n1. Admin (Manage Shop)\n2. Customer (Buy Items)\n3. Exit\nChoice: ");
            if (role == 3) break;

            try
            {
                if (role == 1) adminMenu(shop);
                else if (role == 2) customerMenu(shop);
                else cout << "Invalid choice.\n";
            }
            catch (const runtime_error &e)
            {
                cout << "Error: " << e.what() << endl;
            }
        }
    }
    catch (const InputEnded &)
    {
        cout << "\nInput ended. Exiting.\n";
    }

    shop.saveData();
    return 0;
}