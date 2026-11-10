# E-Shop Shopping Application

## Overview

This is a console-based C++ shopping application. It allows an administrator to manage products and allows customers to add products to a cart, calculate totals, apply discounts, validate a test card number, and generate a receipt.

The project demonstrates object-oriented programming concepts including inheritance, polymorphism, dynamic memory, classes, encapsulation, file handling, and exception handling.

## Main Features

- Admin product management
  - Add electronics and clothing products.
  - Delete products by ID.
  - Display all products.
  - Prevent duplicate product IDs.
  - Validate product type and price.
- Customer shopping
  - View the catalog.
  - Add products to a cart.
  - Add quantities for each product.
  - Automatically combine repeated products in the cart.
  - Calculate subtotal and final total.
- Discounts
  - `SAVE10` gives 10% off.
  - `SAVE20` gives 20% off for orders of ₹1000 or more.
  - Orders of ₹500 or more automatically receive 10% off.
  - Orders of ₹1000 or more automatically receive 15% off when no higher coupon applies.
- Checkout and billing
  - Ask for customer name, phone number, and email.
  - Validate required customer details.
  - Validate a 16-digit card number using the Luhn algorithm.
  - Store only the last four card digits in the receipt.
  - Generate a detailed `receipt.txt` file with date, customer data, items, quantities, discount, and total.
- Edge-case handling
  - Show a message when inventory is empty.
  - Reject empty customer details.
  - Reject invalid card numbers.
  - Reject invalid product types, duplicate IDs, invalid prices, and invalid quantities.

## Project Files

| File | Purpose |
|---|---|
| `main.cpp` | Main menu, admin flow, customer flow, checkout, and card validation. |
| `BaseProduct.h` | Abstract base product class containing common product data. |
| `Electronics.h` | Electronics product type with warranty information. |
| `Clothing.h` | Clothing product type with size information. |
| `Manager.h` | Declaration of catalog management operations. |
| `Manager.cpp` | Product catalog storage, loading, saving, adding, and deleting. |
| `ShoppingCart.h` | Cart items, quantities, subtotal, discounts, and final total. |
| `BillGenerator.h` | Receipt creation and customer information structure. |
| `WebServer.cpp` | Native C++ HTTP server and JSON API for the browser application. |
| `web/index.html` | Web storefront layout. |
| `web/styles.css` | Responsive storefront styling. |
| `web/app.js` | Browser cart, discounts, admin form, and API calls. |
| `User.h` | User, customer, and admin class definitions for future extension. |
| `catalog.txt` | Persistent product catalog data. |
| `receipt.txt` | Receipt generated after a successful checkout. |

## Requirements

- Windows, Linux, or macOS.
- A C++ compiler supporting C++17 or later.
- MinGW g++, GCC, or another compatible compiler.

## Build

From the project directory, run:

```bash
g++ -std=c++17 -Wall -Wextra -o EShopApp main.cpp Manager.cpp
```

On Windows with UTF-8 source files, this command can be used:

```powershell
g++ -std=c++17 -Wall -Wextra -finput-charset=UTF-8 -o EShopApp.exe main.cpp Manager.cpp
```

## Web Version

The project also includes a browser frontend connected to the C++ business logic through a small native HTTP server using Windows Winsock.

Build the web server on Windows with MinGW:

```powershell
g++ -std=c++17 -Wall -Wextra -finput-charset=UTF-8 -o EShopWeb.exe WebServer.cpp Manager.cpp -lws2_32
```

Start it from the project directory:

```powershell
.\EShopWeb.exe
```

Open [http://localhost:8080](http://localhost:8080) in a browser. Keep the `web` folder beside the executable because the server loads `web/index.html`, `web/styles.css`, and `web/app.js` at runtime.

Web API endpoints:

- `GET /api/products` returns catalog products.
- `POST /api/products` adds a product.
- `POST /api/checkout` validates the customer/card and generates `receipt.txt`.

The web interface uses the same C++ catalog, cart, discount rules, Luhn validation, and receipt generator as the console application.

## Run

Run the executable from the project directory:

```powershell
.\EShopApp.exe
```

Run it from the project directory because the application uses relative paths for `catalog.txt` and `receipt.txt`.

## Admin Workflow

1. Select `1` at the role-selection menu.
2. Select `1` to add a product.
3. Choose product type:
   - `1` for Electronics
   - `2` for Clothing
4. Enter the product ID, name, and price.
5. Enter the warranty for electronics or size for clothing.
6. Select `2` to delete a product by ID.
7. Select `3` to view the catalog.

Products are saved to `catalog.txt` immediately after adding or deleting.

## Customer Workflow

1. Select `2` at the role-selection menu.
2. View the catalog with option `1`.
3. Add a product with option `2` and enter its ID and quantity.
4. Select checkout with option `3`.
5. Enter `SAVE10`, `SAVE20`, or `NONE` as the coupon.
6. Review the cart total.
7. Confirm payment.
8. Enter the customer name, phone, email, and a valid 16-digit test card number.
9. The application generates `receipt.txt` after successful checkout.

## Test Card

Use this test number with the built-in Luhn validation:

```text
4242424242424242
```

This is not a real payment card. The application only validates the number mathematically; it does not connect to a payment gateway or process real payments.

## Catalog Format

The current catalog file uses comma-separated values:

```text
101,Laptop,55000
102,Shirt,1200
```

The current loader recreates loaded products as electronics with a default 12-month warranty. If preserving product type, clothing size, or warranty across restarts is required, the catalog format should be extended with a product-type field.

## Receipt Contents

The generated receipt includes:

- Date and time.
- Customer name, phone, and email.
- Product names and quantities.
- Subtotal.
- Discount name and amount.
- Final total in rupees.
- Masked card number showing only the last four digits.
- Thank-you greeting.

## OOP Concepts Used

- Abstraction through `BaseProduct`.
- Inheritance through `Electronics` and `Clothing`.
- Runtime polymorphism through `displayDetails()`.
- Encapsulation through private product and cart data.
- Dynamic memory using product pointers managed by `Manager`.
- File handling using `ifstream` and `ofstream`.
- Exception handling for invalid checkout and product input.

## Current Limitations

- Product names cannot contain spaces because input uses `cin >> name`.
- Inventory does not track stock quantities.
- Product type information is not preserved in the current catalog file format.
- The checkout validates card format only and does not process real payments.
- `receipt.txt` is overwritten after each successful checkout.

## Future Improvements

- Add stock quantity and prevent overselling.
- Store product type, warranty, and clothing size in `catalog.txt`.
- Support product names with spaces.
- Add separate receipt files with unique invoice numbers.
- Add real database storage.
- Integrate a secure payment provider for production use.
- Add automated unit tests for cart totals, discounts, and Luhn validation.
