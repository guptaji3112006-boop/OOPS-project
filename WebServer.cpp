#include <winsock2.h>
#include <ws2tcpip.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <cctype>
#include "Manager.h"
#include "Electronics.h"
#include "Clothing.h"
#include "ShoppingCart.h"
#include "BillGenerator.h"

using namespace std;

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
    out << "[";
    bool first = true;
    for (auto product : shop.getProducts())
    {
        if (!first)
            out << ",";
        first = false;
        out << "{\"id\":" << product->getId() << ",\"name\":\"" << product->getName()
            << "\",\"price\":" << product->getPrice() << ",\"type\":\"Product\"}";
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

string fileBody(const string &path)
{
    ifstream file(path, ios::binary);
    if (!file)
        return "";
    return string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
}

int main()
{
    Manager shop;
    shop.loadData();
    WSADATA data;
    WSAStartup(MAKEWORD(2, 2), &data);
    SOCKET server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(8080);
    address.sin_addr.s_addr = INADDR_ANY;
    bind(server, reinterpret_cast<sockaddr *>(&address), sizeof(address));
    listen(server, 10);
    cout << "E-Shop web server running at http://localhost:8080\n";
    while (true)
    {
        SOCKET client = accept(server, nullptr, nullptr);
        if (client == INVALID_SOCKET)
            continue;
        char buffer[65536]{};
        int received = recv(client, buffer, sizeof(buffer) - 1, 0);
        if (received <= 0)
        {
            closesocket(client);
            continue;
        }
        string request(buffer, received);
        size_t lineEnd = request.find("\r\n");
        string requestLine = request.substr(0, lineEnd);
        istringstream line(requestLine);
        string method, path;
        line >> method >> path;
        if (method == "GET" && path == "/api/products")
            respond(client, "200 OK", "application/json", productsJson(shop));
        else if (method == "GET" && (path == "/" || path == "/index.html"))
            respond(client, "200 OK", "text/html", fileBody("web/index.html"));
        else if (method == "GET" && path == "/styles.css")
            respond(client, "200 OK", "text/css", fileBody("web/styles.css"));
        else if (method == "GET" && path == "/app.js")
            respond(client, "200 OK", "application/javascript", fileBody("web/app.js"));
        else if (method == "POST" && path == "/api/products")
        {
            int id = stoi(valueOf(request, "id"));
            double price = stod(valueOf(request, "price"));
            string name = valueOf(request, "name");
            int type = stoi(valueOf(request, "type"));
            if (shop.getProductById(id) || name.empty() || price <= 0 || (type != 1 && type != 2))
                respond(client, "400 Bad Request", "application/json", "{\"error\":\"Invalid or duplicate product data\"}");
            else
            {
                shop.addProduct(type == 1 ? static_cast<BaseProduct *>(new Electronics(id, name, price, 12)) : static_cast<BaseProduct *>(new Clothing(id, name, price, "M")));
                respond(client, "201 Created", "application/json", "{\"ok\":true}");
            }
        }
        else if (method == "POST" && path == "/api/checkout")
        {
            string card = valueOf(request, "card");
            string name = valueOf(request, "name");
            string phone = valueOf(request, "phone");
            string email = valueOf(request, "email");
            if (name.empty() || phone.empty() || email.empty() || !luhn(card))
                respond(client, "400 Bad Request", "application/json", "{\"error\":\"Customer details or card number are invalid\"}");
            else
            {
                ShoppingCart cart;
                string items = request.substr(request.find("\"items\""));
                size_t pos = 0;
                while ((pos = items.find("\"id\":", pos)) != string::npos)
                {
                    int id = stoi(items.substr(pos + 5));
                    size_t q = items.find("\"quantity\":", pos);
                    int quantity = stoi(items.substr(q + 11));
                    if (BaseProduct *p = shop.getProductById(id))
                        cart.addProduct(p, quantity);
                    pos += 5;
                }
                cart.applyDiscount(valueOf(request, "coupon"));
                CustomerInfo customer{name, phone, email};
                if (BillGenerator::generate(cart, customer, card))
                    respond(client, "200 OK", "application/json", "{\"receipt\":\"receipt.txt\"}");
                else
                    respond(client, "500 Internal Server Error", "application/json", "{\"error\":\"Could not create receipt\"}");
            }
        }
        else
            respond(client, "404 Not Found", "text/plain", "Not found");
        closesocket(client);
    }
}
