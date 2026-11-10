# E-Shop Web Version

## 1. Introduction

This project provides a web interface for the original C++ shopping application. The browser frontend communicates with a native C++ HTTP server, so the catalog, cart calculations, discount rules, Luhn card validation, and receipt generation remain implemented in C++.

The web version is a local full-stack application:

```text
Browser UI  ->  C++ HTTP API  ->  C++ business classes  ->  catalog.txt / receipt.txt
```

## 2. Web Architecture

### Frontend

The frontend is located in the `web/` folder:

- `index.html` defines the storefront layout.
- `styles.css` provides responsive styling and the user interface design.
- `app.js` loads products, manages the browser cart, applies visual totals, submits admin forms, and calls the C++ API.

### Backend

`WebServer.cpp` is a lightweight HTTP server built with Windows Winsock. It:

- Listens on `localhost:8080`.
- Serves the web files.
- Returns products from the C++ `Manager` catalog.
- Adds products through `Manager::addProduct()`.
- Builds a C++ `ShoppingCart` during checkout.
- Uses the existing discount and Luhn validation logic.
- Generates `receipt.txt` through `BillGenerator`.

No JavaScript-only checkout calculation is trusted by the backend. The server rebuilds the cart and calculates the final amount in C++ before creating the receipt.

## 3. Web API

### Get products

```http
GET /api/products
```

Example response:

```json
[
  {"id":101,"name":"Laptop","price":55000,"type":"Product"}
]
```

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

The backend rejects invalid product types, non-positive prices, empty names, and duplicate IDs.

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

The backend validates the customer data and card number, recreates the cart from catalog IDs, applies the discount, and generates `receipt.txt`.

## 4. OOP Concepts Used

### Class and object

Classes define the structure and behavior of application entities:

- `BaseProduct` represents common product data.
- `Electronics` and `Clothing` represent product objects.
- `Manager` represents the product catalog.
- `ShoppingCart` represents a customer cart.
- `BillGenerator` represents bill creation behavior.
- `CustomerInfo` represents receipt customer data.

Objects are created and used by both the console application and `WebServer.cpp`.

### Encapsulation

Encapsulation keeps data private and exposes controlled methods.

Examples:

- Product ID, name, and price are private in `BaseProduct`.
- The product list is private in `Manager`.
- Cart items and discount state are private in `ShoppingCart`.
- Code outside these classes uses methods such as `getProductById()`, `addProduct()`, `getTotal()`, and `applyDiscount()`.

This prevents the web server from directly changing internal catalog or cart data.

### Abstraction

`BaseProduct` provides a common abstraction for all products. The rest of the application can work with a `BaseProduct*` without needing to know every product-specific implementation.

`BillGenerator::generate()` also abstracts file formatting. The web server only supplies cart and customer data; it does not manually format the bill.

### Inheritance

The product hierarchy is:

```text
BaseProduct
├── Electronics
└── Clothing
```

`Electronics` inherits common product behavior and adds warranty information. `Clothing` inherits common product behavior and adds size information.

### Polymorphism

`BaseProduct` declares the virtual function `displayDetails()`.

The derived classes override it:

- `Electronics::displayDetails()` displays warranty information.
- `Clothing::displayDetails()` displays size information.

When `Manager` stores products as `BaseProduct*`, the correct derived display method is selected at runtime.

### Dynamic memory and ownership

The catalog stores product pointers in `Manager`. `Manager::~Manager()` deletes the products when the manager is destroyed. This demonstrates dynamic allocation and ownership management.

The shopping cart stores non-owning pointers to catalog products. The cart does not delete those pointers because `Manager` owns the catalog products.

### Composition

`ShoppingCart` contains multiple `CartItem` values. Each `CartItem` connects a product pointer with a quantity.

`BillGenerator` uses a `ShoppingCart` and `CustomerInfo` to create a receipt. These objects work together to provide checkout behavior.

### Static member function

`BillGenerator::generate()` is static because bill generation does not require a long-lived `BillGenerator` object. It can be called directly by both the console application and the web server.

### Exception handling

The console application uses exceptions for invalid payment, missing details, invalid product data, and receipt creation failures. The web server converts invalid requests into HTTP error responses such as `400 Bad Request`.

### File handling

- `Manager` uses `ifstream` to load `catalog.txt`.
- `Manager` uses `ofstream` to save catalog changes.
- `BillGenerator` uses `ofstream` to create `receipt.txt`.

The web layer reuses the same file-backed C++ classes instead of creating a separate storage system.

## 5. Checkout Flow

```text
1. Browser loads products
2. User adds product and quantity
3. Browser opens checkout form
4. User enters name, phone, email, and test card
5. Browser sends POST /api/checkout
6. C++ validates required fields
7. C++ validates the card using Luhn
8. C++ finds each product in Manager
9. C++ builds ShoppingCart
10. C++ applies coupon or automatic discount
11. C++ generates receipt.txt
12. Browser displays payment success
```

## 6. Discount Rules

- `SAVE10`: 10% discount.
- `SAVE20`: 20% discount when subtotal is at least ₹1000.
- Subtotal of at least ₹500: automatic 10% discount.
- Subtotal of at least ₹1000 without a qualifying coupon: automatic 15% discount.

The final discount is calculated by `ShoppingCart::applyDiscount()` and `ShoppingCart::getTotal()`.

## 7. Luhn Card Validation

The web server validates a 16-digit card number using the Luhn algorithm:

1. Start from the rightmost digit.
2. Double every second digit.
3. Subtract 9 when a doubled digit is greater than 9.
4. Add all digits.
5. Accept the number when the total is divisible by 10.

Use this test number:

```text
4242424242424242
```

This is only a mathematical test number. No real payment is processed.

## 8. Build and Run

Open PowerShell in the project directory and compile:

```powershell
g++ -std=c++17 -Wall -Wextra -finput-charset=UTF-8 -o EShopWeb.exe WebServer.cpp Manager.cpp -lws2_32
```

Start the server:

```powershell
.\EShopWeb.exe
```

Open the application at:

```text
http://localhost:8080
```

The `web/` folder must remain beside the executable. The server reads `catalog.txt`, `web/index.html`, `web/styles.css`, and `web/app.js` using relative paths.

## 9. Web File Responsibilities

| File | Responsibility |
|---|---|
| `WebServer.cpp` | HTTP server, API routing, request validation, C++ checkout orchestration. |
| `web/index.html` | Product catalog, cart, admin form, checkout form. |
| `web/styles.css` | Layout, colors, cards, responsive design, modal form. |
| `web/app.js` | API requests, cart display, quantity controls, coupon input, user messages. |
| `Manager.*` | Catalog storage and product lookup. |
| `ShoppingCart.h` | Cart quantities, subtotal, discounts, and total. |
| `BillGenerator.h` | Receipt formatting and customer information. |

## 10. Current Limitations

- The web server is designed for local development, not production deployment.
- It uses a simple manual JSON parser and should be replaced with a proper JSON library for production.
- The server is single-threaded.
- There is no login or admin authentication.
- There is no stock quantity tracking.
- Product-specific warranty and clothing size are not fully serialized in the current API response.
- `receipt.txt` is overwritten by each new checkout.
- The payment system validates card format only and does not connect to a payment gateway.

## 11. Future Improvements

- Add a proper JSON library such as nlohmann/json.
- Add HTTPS and authentication.
- Add admin login and protected product APIs.
- Use a database instead of plain text files.
- Add inventory quantity and stock validation.
- Add unique invoice numbers and separate receipt files.
- Add automated tests for API routes, discounts, cart quantities, and Luhn validation.
- Add a production HTTP framework such as Crow or Drogon.
