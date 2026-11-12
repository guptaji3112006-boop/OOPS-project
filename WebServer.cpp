#include <winsock2.h>
#include <ws2tcpip.h>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include "Manager.h"
#include "Electronics.h"
#include "Clothing.h"
#include "ShoppingCart.h"
#include "BillGenerator.h"

using namespace std;

static const string JSON_TYPE = "application/json; charset=utf-8";

string valueOf(const string &json, const string &key)
{
    string marker = "\"" + key + "\":";
    size_t start = json.find(marker);
    if (start == string::npos)
        return "";
    start += marker.size();
    while (start < json.size() && isspace(static_cast<unsigned char>(json[start])))
        ++start;
    if (start < json.size() && json[start] == '\"')
    {
        ++start;
        size_t end = json.find('\"', start);
        return end == string::npos ? "" : json.substr(start, end - start);
    }
    size_t end = json.find_first_of(",}", start);
    return json.substr(start, end == string::npos ? string::npos : end - start);
}

bool toInt(const string &s, int &out)
{
    try
    {
        size_t used = 0;
        out = stoi(s, &used);
        while (used < s.size() && isspace(static_cast<unsigned char>(s[used])))
            ++used;
        return used == s.size();
    }
    catch (...)
    {
        return false;
    }
}

bool toDouble(const string &s, double &out)
{
    try
    {
        size_t used = 0;
        out = stod(s, &used);
        while (used < s.size() && isspace(static_cast<unsigned char>(s[used])))
            ++used;
        return used == s.size();
    }
    catch (...)
    {
        return false;
    }
}

bool intAfter(const string &s, size_t pos, int &out)
{
    try
    {
        out = stoi(s.substr(pos));
        return true;
    }
    catch (...)
    {
        return false;
    }
}

string jsonEscape(const string &s)
{
    string out;
    for (char c : s)
    {
        switch (c)
        {
        case '"':
            out += "\\\"";
            break;
        case '\\':
            out += "\\\\";
            break;
        case '\n':
            out += "\\n";
            break;
        case '\r':
            out += "\\r";
            break;
        case '\t':
            out += "\\t";
            break;
        default:
            out += c;
        }
    }
    return out;
}

bool luhn(const string &card)
{
    if (card.size() != 16)
        return false;
    int sum = 0;
    bool alternate = false;
    for (int i = static_cast<int>(card.size()) - 1; i >= 0; --i)
    {
        if (!isdigit(static_cast<unsigned char>(card[i])))
            return false;
        int digit = card[i] - '0';
        if (alternate && (digit *= 2) > 9)
            digit -= 9;
        sum += digit;
        alternate = !alternate;
    }
    return sum % 10 == 0;
}

string productsJson(const Manager &shop)
{
    ostringstream out;
    out << fixed << setprecision(2);
    out << "[";
    bool first = true;
    for (auto product : shop.getProducts())
    {
        if (!first)
            out << ",";
        first = false;
        out << "{\"id\":" << product->getId()
            << ",\"name\":\"" << jsonEscape(product->getName())
            << "\",\"price\":" << product->getPrice()
            << ",\"type\":\"" << jsonEscape(product->getType())
            << "\",\"extra\":\"" << jsonEscape(product->getExtra()) << "\"}";
    }
    out << "]";
    return out.str();
}

void respond(SOCKET client, const string &status, const string &type, const string &body)
{
    string response = "HTTP/1.1 " + status + "\r\nContent-Type: " + type +
                      "\r\nContent-Length: " + to_string(body.size()) + "\r\nConnection: close\r\n\r\n" + body;
    send(client, response.c_str(), static_cast<int>(response.size()), 0);
}

void fail(SOCKET client, const string &status, const string &message)
{
    respond(client, status, JSON_TYPE, "{\"error\":\"" + jsonEscape(message) + "\"}");
}

string fileBody(const string &path)
{
    ifstream file(path, ios::binary);
    if (!file)
        return "";
    return string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
}

// Poori request padhta hai (headers + Content-Length jitni body)
string readRequest(SOCKET client)
{
    string request;
    char buf[4096];
    size_t headerEnd = string::npos;
    size_t expected = 0;
    while (request.size() < 1000000)
    {
        int n = recv(client, buf, sizeof(buf), 0);
        if (n <= 0)
            break;
        request.append(buf, n);
        if (headerEnd == string::npos)
        {
            size_t found = request.find("\r\n\r\n");
            if (found != string::npos)
            {
                headerEnd = found + 4;
                string headers = request.substr(0, headerEnd);
                for (auto &c : headers)
                    c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
                size_t cl = headers.find("content-length:");
                if (cl != string::npos)
                {
                    try
                    {
                        expected = stoul(headers.substr(cl + 15));
                    }
                    catch (...)
                    {
                        expected = 0;
                    }
                }
            }
        }
        if (headerEnd != string::npos && request.size() >= headerEnd + expected)
            break;
    }
    return request;
}

void handleAddProduct(SOCKET client, Manager &shop, const string &request)
{
    int id = 0, type = 0;
    double price = 0;
    string name = valueOf(request, "name");
    string extra = valueOf(request, "extra");

    if (!toInt(valueOf(request, "id"), id) || id <= 0)
        return fail(client, "400 Bad Request", "Product ID must be a positive number");
    if (!toDouble(valueOf(request, "price"), price) || !(price > 0 && price < 1e9))
        return fail(client, "400 Bad Request", "Price must be a positive number");
    if (!toInt(valueOf(request, "type"), type) || (type != 1 && type != 2))
        return fail(client, "400 Bad Request", "Type must be 1 (Electronics) or 2 (Clothing)");
    if (name.empty())
        return fail(client, "400 Bad Request", "Product name is required");
    if (name.find('|') != string::npos || extra.find('|') != string::npos)
        return fail(client, "400 Bad Request", "The character | is not allowed");
    if (shop.getProductById(id))
        return fail(client, "400 Bad Request", "Product ID already exists");

    BaseProduct *product = nullptr;
    if (type == 1)
    {
        int warranty = 12;
        if (!extra.empty() && (!toInt(extra, warranty) || warranty < 0 || warranty > 600))
            return fail(client, "400 Bad Request", "Warranty must be a number of months");
        product = new Electronics(id, name, price, warranty);
    }
    else
    {
        product = new Clothing(id, name, price, extra.empty() ? "M" : extra);
    }

    // addProduct() product ka malik ban jata hai; fail hone par khud delete karta hai.
    if (shop.addProduct(product))
        respond(client, "201 Created", JSON_TYPE, "{\"ok\":true}");
    else
        fail(client, "400 Bad Request", "Could not add product");
}

void handleCheckout(SOCKET client, Manager &shop, const string &request)
{
    string card = valueOf(request, "card");
    string name = valueOf(request, "name");
    string phone = valueOf(request, "phone");
    string email = valueOf(request, "email");
    if (name.empty() || phone.empty() || email.empty() || !luhn(card))
        return fail(client, "400 Bad Request", "Customer details or card number are invalid");

    size_t itemsPos = request.find("\"items\"");
    if (itemsPos == string::npos)
        return fail(client, "400 Bad Request", "Cart is empty");

    string items = request.substr(itemsPos);
    ShoppingCart cart;
    int added = 0;
    size_t pos = 0;
    while ((pos = items.find("\"id\":", pos)) != string::npos)
    {
        int id = 0, quantity = 0;
        size_t q = items.find("\"quantity\":", pos);
        if (q == string::npos || !intAfter(items, pos + 5, id) || !intAfter(items, q + 11, quantity) ||
            quantity < 1 || quantity > 1000)
            return fail(client, "400 Bad Request", "Invalid cart data");
        BaseProduct *p = shop.getProductById(id);
        if (!p)
            return fail(client, "400 Bad Request", "A product in the cart no longer exists");
        cart.addProduct(p, quantity);
        ++added;
        pos += 5;
    }
    if (added == 0)
        return fail(client, "400 Bad Request", "Cart is empty");

    cart.applyDiscount(valueOf(request, "coupon"));
    CustomerInfo customer{name, phone, email};
    if (BillGenerator::generate(cart, customer, card))
        respond(client, "200 OK", JSON_TYPE, "{\"receipt\":\"receipt.txt\"}");
    else
        fail(client, "500 Internal Server Error", "Could not create receipt");
}

int main()
{
    Manager shop;
    shop.loadData();

    WSADATA data;
    if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
    {
        cerr << "WSAStartup failed\n";
        return 1;
    }
    SOCKET server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server == INVALID_SOCKET)
    {
        cerr << "Could not create socket\n";
        WSACleanup();
        return 1;
    }
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(8080);
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // sirf isi computer se
    if (bind(server, reinterpret_cast<sockaddr *>(&address), sizeof(address)) == SOCKET_ERROR ||
        listen(server, 10) == SOCKET_ERROR)
    {
        cerr << "Could not start server (port 8080 already in use?)\n";
        closesocket(server);
        WSACleanup();
        return 1;
    }
    cout << "E-Shop web server running at http://localhost:8080\n";

    while (true)
    {
        SOCKET client = accept(server, nullptr, nullptr);
        if (client == INVALID_SOCKET)
            continue;

        string request = readRequest(client);
        if (request.empty())
        {
            closesocket(client);
            continue;
        }

        try
        {
            size_t lineEnd = request.find("\r\n");
            istringstream line(request.substr(0, lineEnd));
            string method, path;
            line >> method >> path;

            if (method == "GET" && path == "/api/products")
                respond(client, "200 OK", JSON_TYPE, productsJson(shop));
            else if (method == "GET" && (path == "/" || path == "/index.html"))
                respond(client, "200 OK", "text/html; charset=utf-8", fileBody("web/index.html"));
            else if (method == "GET" && path == "/styles.css")
                respond(client, "200 OK", "text/css; charset=utf-8", fileBody("web/styles.css"));
            else if (method == "GET" && path == "/app.js")
                respond(client, "200 OK", "application/javascript; charset=utf-8", fileBody("web/app.js"));
            else if (method == "POST" && path == "/api/products")
                handleAddProduct(client, shop, request);
            else if (method == "POST" && path == "/api/checkout")
                handleCheckout(client, shop, request);
            else
                respond(client, "404 Not Found", "text/plain; charset=utf-8", "Not found");
        }
        catch (...)
        {
            fail(client, "500 Internal Server Error", "Something went wrong");
        }
        closesocket(client);
    }
}