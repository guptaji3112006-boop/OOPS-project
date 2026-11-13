# E-Shop (C++ OOP Project)

## 1. Introduction

E-Shop is a shopping application written in C++ using Object-Oriented Programming. It has two interfaces that share the same C++ classes:

- **Console application** (`main.cpp`): Admin and Customer menus in the terminal.
- **Web application** (`WebServer.cpp` + `web/`): a browser frontend that talks to a native C++ HTTP server.

The catalog, cart calculations, discount rules, Luhn card validation, and receipt generation are implemented in C++ in both versions.

The web version is a local full-stack application:

```text
Browser UI  ->  C++ HTTP API  ->  C++ business classes  ->  catalog.txt / receipt.txt
```

## 2. Project Structure

```text
OOPS-project/
├── BaseProduct.h      abstract base class for products
├── Electronics.h      derived class (warranty)
├── Clothing.h         derived class (size)
├── Manager.h/.cpp     product catalog, loading and saving catalog.txt
├── ShoppingCart.h     cart items, subtotal, discounts, total
├── BillGenerator.h    receipt formatting and customer information
├── main.cpp           console application
├── WebServer.cpp      HTTP server and API for the web version
├── catalog.txt        saved products
└── web/
    ├── index.html
    ├── styles.css
    └── app.js
```

`.gitignore` excludes compiled files (`*.exe`, `*.o`), `receipt.txt`, and `.vscode/`, so they are not stored in the repository.

## 3. Web Architecture

### Frontend

The frontend is located in the `web/` folder:

- `index.html` defines the storefront layout, cart, admin form, and checkout form.
- `styles.css` provides responsive styling and the user interface design.
- `app.js` loads products, manages the browser cart, shows visual totals, submits the admin and checkout forms, and calls the C++ API. Product names and details are HTML-escaped before they are shown, and network or server errors are shown as messages.

### Backend

`WebServer.cpp` is a lightweight HTTP server built with Windows Winsock. It:

- Listens on `localhost:8080` (loopback only, not reachable from other computers).
- Serves the web files.
- Returns products from the C++ `Manager` catalog, including the product type and its warranty or size.
- Validates input and adds products through `Manager::addProduct()`.
- Builds a C++ `ShoppingCart` during checkout.
- Uses the discount and Luhn validation logic.
- Generates `receipt.txt` through `BillGenerator`.

No JavaScript-only checkout calculation is trusted by the backend. The server rebuilds the cart from catalog IDs and calculates the final amount in C++ before creating the receipt.

## 4. Web API

All error responses use this format:

```json
{"error":"Message describing the problem"}
```

### Get products

```http
GET /api/products
```

Example response:

```json
[
  {"id":101,"name":"press","price":850.00,"type":"Electronics","extra":"12"},
  {"id":201,"name":"jeans","price":600.00,"type":"Clothing","extra":"M"}
]
```

`extra` is the warranty in months for Electronics and the size for Clothing.

### Add product

```http
POST /api/products
Content-Type: application/json
```

Request data:

```json
{
  "type": "1",
  "id": "101",
  "name": "Laptop",
  "price": "55000",
  "extra": "12"
}
```

- `type`: `1` = Electronics, `2` = Clothing.
- `extra`: warranty in months (0 to 600, default 12) for Electronics; size for Clothing (default `M`).

The backend returns `201 Created` on success and `400 Bad Request` for:

- a missing or non-numeric ID or price, or an ID that is not positive,
- a price that is not greater than zero,
- an invalid product type,
- an empty product name,
- a duplicate ID,
- a name or size containing the `|` character (used by the catalog file format),
- an invalid warranty value.

### Checkout

```http
POST /api/checkout
Content-Type: application/json
```

Request data:

```json
{
  "customer": {
    "name": "Test Customer",
    "phone": "9876543210",
    "email": "test@example.com"
  },
  "card": "4242424242424242",
  "coupon": "SAVE10",
  "items": [
    {"id": 101, "quantity": 2}
  ]
}
```

The backend validates the customer data and card number, rejects an empty cart, unknown product IDs and invalid quantities (1 to 1000), recreates the cart from catalog IDs, applies the discount, and generates `receipt.txt`.

## 5. Console Application

`main.cpp` provides the same shop in the terminal:

- **Admin:** add a product (Electronics or Clothing), delete a product, or view all products.
- **Customer:** view the catalog, add items to the cart, apply a coupon, and pay with a card.

All input is validated. Invalid numbers are asked again, names with spaces are accepted, duplicate IDs and invalid values are rejected with a message, and the menus stay open after an error.

## 6. Catalog File Format

`catalog.txt` stores one product per line:

```text
Type|id|name|price|extra
```

Example:

```text
Electronics|101|press|850.00|12
Clothing|201|jeans|600.00|M
```

- Electronics: `extra` is the warranty in months.
- Clothing: `extra` is the size.
- Lines in the older `id,name,price` format are still read and loaded as Electronics with a 12 month warranty.
- Damaged lines are skipped instead of stopping the program.

## 7. OOP Concepts Used

### Class and object

- `BaseProduct` represents common product data.
- `Electronics` and `Clothing` represent product objects.
- `Manager` represents the product catalog.
- `ShoppingCart` represents a customer cart.
- `BillGenerator` represents bill creation behavior.
- `CustomerInfo` represents receipt customer data.

Objects are created and used by both the console application and `WebServer.cpp`.

### Encapsulation

Data is private and exposed through controlled methods.

- Product ID, name, and price are private in `BaseProduct`. The setters reject invalid values (a price must be greater than zero, a name cannot be empty).
- The product list is private in `Manager`.
- Cart items and the coupon are private in `ShoppingCart`.
- Code outside these classes uses methods such as `getProductById()`, `addProduct()`, `getTotal()`, and `applyDiscount()`.

### Abstraction

`BaseProduct` is an abstract class. It declares pure virtual functions (`displayDetails()`, `getType()`, `getExtra()`), so it cannot be instantiated and every product type must provide them. The rest of the application works with a `BaseProduct*` without knowing which product type it is.

`BillGenerator::generate()` also abstracts file formatting. The caller only supplies cart and customer data.

### Inheritance

```text
BaseProduct
├── Electronics
└── Clothing
```

`Electronics` inherits the common product behavior and adds warranty information. `Clothing` inherits it and adds size information.

### Polymorphism

The pure virtual functions are overridden in the derived classes:

- `displayDetails()` prints warranty for Electronics and size for Clothing.
- `getType()` returns `"Electronics"` or `"Clothing"`.
- `getExtra()` returns the warranty or size as text.

`Manager` stores products as `BaseProduct*`, and the correct derived version is selected at runtime. `Manager::saveData()` uses `getType()` and `getExtra()` to save each product correctly without checking its type.

### Dynamic memory and ownership

- `Manager` owns the products: it deletes them in its destructor, and `BaseProduct` has a virtual destructor so derived objects are deleted correctly.
- `Manager::addProduct()` takes ownership of the pointer. If the product is invalid (for example a duplicate ID), it deletes the product itself and returns `false`.
- The copy constructor and copy assignment of `Manager` are deleted, so the catalog can never be deleted twice.
- `ShoppingCart` stores non-owning pointers to catalog products and never deletes them.

### Composition

`ShoppingCart` contains multiple `CartItem` values, each connecting a product pointer with a quantity. `BillGenerator` uses a `ShoppingCart` and a `CustomerInfo` to create a receipt.

### Static member function

`BillGenerator::generate()` is static because bill generation does not need a long-lived object. The console application and the web server call it directly.

### Exception handling

The console application uses exceptions for invalid payment, missing details, invalid product data, and receipt creation failures. The web server turns invalid requests into HTTP error responses such as `400 Bad Request`, and unexpected errors into `500` instead of stopping.

### File handling

- `Manager` uses `ifstream` to load `catalog.txt` and `ofstream` to save it. Saving reports failure to the caller.
- `BillGenerator` uses `ofstream` to create `receipt.txt` and reports write failures.

## 8. Checkout Flow

```text
1. Browser loads products
2. User adds a product and quantity
3. Browser opens the checkout form
4. User enters name, phone, email, and test card
5. Browser sends POST /api/checkout
6. C++ validates the required fields
7. C++ validates the card using Luhn
8. C++ finds each product in Manager
9. C++ builds ShoppingCart
10. C++ applies the coupon or automatic discount
11. C++ generates receipt.txt
12. Browser displays payment success and clears the cart and coupon
```

## 9. Discount Rules

The first rule that matches is used:

| Condition | Discount |
|---|---|
| Coupon `SAVE10` (any subtotal) | 10% |
| Coupon `SAVE20` and subtotal of at least ₹1000 | 20% |
| Subtotal of at least ₹1000 | automatic 15% |
| Subtotal of at least ₹500 | automatic 10% |
| Otherwise | none |

Notes:

- Coupons are not case-sensitive (`save10` works).
- The discount amount is rounded to 2 decimal places.
- `SAVE10` always gives 10%, even above ₹1000 where the automatic discount would be 15%.
- The rules exist in `ShoppingCart` (C++) and in `app.js` (for the cart total shown on screen). If one is changed, change the other too.

The receipt always uses the C++ calculation.

## 10. Luhn Card Validation

The system includes a payment validation simulation based on the Luhn algorithm (Mod 10), the checksum formula used by financial institutions to validate card numbers. A card must have exactly 16 digits and pass the check:

1. Start from the rightmost digit.
2. Double every second digit.
3. Subtract 9 when a doubled digit is greater than 9.
4. Add all digits.
5. Accept the number when the total is divisible by 10.

Use this test number:

```text
4242424242424242
```

This is only a mathematical test number. No real payment is processed. The receipt shows only the last 4 digits of the card.

Example receipt:

```text
=============== E-SHOP BILL ===============
Date: 2026-11-12 18:05:10
Customer: Test User
Phone:    9999999999
Email:    test@example.com
--------------------------------------------
Product                      Qty       Price
tv                             1    20000.00
--------------------------------------------
Subtotal:                          ₹20000.00
Discount (Automatic 15% discount): -₹3000.00
TOTAL:                             ₹17000.00
Card:                               ****4242
============================================
Thank you, Test User! Visit us again.
```

## 11. Build and Run

The project is built for Windows (MinGW g++). Open PowerShell in the project directory.

### Web version

```powershell
g++ -std=c++17 -Wall -Wextra -finput-charset=UTF-8 -o EShopWeb.exe WebServer.cpp Manager.cpp -lws2_32
.\EShopWeb.exe
```

Then open:

```text
http://localhost:8080
```

The `web/` folder must stay beside the executable. The server reads `catalog.txt`, `web/index.html`, `web/styles.css`, and `web/app.js` using relative paths.

### Console version

```powershell
g++ -std=c++17 -Wall -Wextra -finput-charset=UTF-8 -o EShopApp.exe main.cpp Manager.cpp
.\EShopApp.exe
```

Compiled `.exe` files are not stored in the repository, so build them after cloning. If a build fails with "Permission denied", the program is still running; close it and compile again.

## 12. File Responsibilities

| File | Responsibility |
|---|---|
| `WebServer.cpp` | HTTP server, API routing, request validation, C++ checkout orchestration. |
| `main.cpp` | Console application with Admin and Customer menus. |
| `web/index.html` | Product catalog, cart, admin form, checkout form. |
| `web/styles.css` | Layout, colors, cards, responsive design, modal form. |
| `web/app.js` | API requests, cart display, quantity controls, coupon input, HTML escaping, user messages. |
| `Manager.*` | Catalog storage, loading and saving, product lookup. |
| `BaseProduct.h`, `Electronics.h`, `Clothing.h` | Product class hierarchy. |
| `ShoppingCart.h` | Cart quantities, subtotal, discounts, and total. |
| `BillGenerator.h` | Receipt formatting and customer information. |

## 13. Current Limitations

- The web server is designed for local development, not production deployment, and works on Windows only (Winsock).
- It uses a simple manual JSON parser and should be replaced with a proper JSON library for production. For this reason product names cannot contain `"`, `\`, or `|`.
- The server is single-threaded.
- There is no login or admin authentication.
- There is no stock quantity tracking.
- `receipt.txt` is overwritten by each new checkout.
- The payment system validates the card format only and does not connect to a payment gateway.
- The Luhn check is written separately in `main.cpp` and `WebServer.cpp`.
- The shopping cart in the web page is kept in the browser and is lost when the page is reloaded.

## 14. Future Improvements

- Add a proper JSON library such as nlohmann/json.
- Add HTTPS and authentication.
- Add admin login and protected product APIs.
- Use a database instead of plain text files.
- Add inventory quantity and stock validation.
- Add unique invoice numbers and separate receipt files.
- Add automated tests for API routes, discounts, cart quantities, and Luhn validation.
- Move the Luhn check into one shared header.
- Add a production HTTP framework such as Crow or Drogon.