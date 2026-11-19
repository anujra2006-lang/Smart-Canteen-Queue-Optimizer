#include <iostream>
#include <iomanip>
#include <map>
#include <vector>
#include <unordered_map>
#include <string>
#include <cctype>
using namespace std;
enum Status { WAITING, PREPARING, READY, COMPLETED, CANCELLED };
struct Item {
    string id, name;
    int price;
    int wtime;
};
struct Order {
    int tokenId;
    int studentId;
    map<string, int> lines;
    int total;
    Status status;
};
vector<Item> menu = {
    {"M01", "Veg Thali",      90, 8},
    {"M02", "Rajma Chawal",   70, 6},
    {"M04", "Masala Dosa",    60, 10},
    {"B01", "Masala Tea",     15, 3},
    {"B03", "Cold Coffee",    40, 4},
    {"S01", "Samosa (2 pcs)", 25, 3},
};
unordered_map<string, int> menuIndex;
unordered_map<int, Order> allOrders;
int nextToken = 1;
void buildIndex() {
    menuIndex.clear();
    for (int i = 0; i < (int)menu.size(); i++) menuIndex[menu[i].id] = i;
}
Item* findItem(string id) {
    for (size_t i = 0; i < id.size(); i++)
        id[i] = (char)toupper((unsigned char)id[i]);
    auto it = menuIndex.find(id);
    return it == menuIndex.end() ? nullptr : &menu[it->second];
}

string statusToString(Status s) {
    switch (s) {
        case WAITING:   return "Waiting";
        case PREPARING: return "Preparing";
        case READY:     return "Ready";
        case COMPLETED: return "Completed";
        case CANCELLED: return "Cancelled";
    }
    return "Unknown";
}
int calculateBill(const map<string, int>& lines) {
    int total = 0;
    for (auto& l : lines) {
        Item* it = findItem(l.first);
        if (it != nullptr)
            total += it->price * l.second;
    }
    return total;
}
void printBill(const Order& o) {
    cout << "\nBILL (Token " << o.tokenId << ")\n";
    cout << left << setw(6) << "ID" << setw(22) << "Item" << setw(6) << "Qty"
         << setw(8) << "Price" << "Total\n";
    for (auto& l : o.lines) {
        Item* it = findItem(l.first);
        if (it == nullptr) continue;
        cout << left << setw(6) << it->id << setw(22) << it->name << setw(6) << l.second
             << setw(8) << it->price << it->price * l.second << "\n";
    }
    cout << string(48, '-') << "\n";
    cout << "GRAND TOTAL (Rs.): " << o.total << "\n";
}
void displayOrderStatus(int tokenId) {
    auto it = allOrders.find(tokenId);
    if (it == allOrders.end()) { cout << "Token not found.\n"; return; }

    cout << "Token " << tokenId << " status: "
         << statusToString(it->second.status) << "\n";
}
void demoPlaceOrder() {
    Order o;
    cout << "Student ID: ";
    if (!(cin >> o.studentId)) { cin.clear(); cin.ignore(1000, '\n'); cout << "Invalid ID.\n"; return; }
    cout << "Menu:\n";
    for (auto& m : menu)
        cout << "  " << m.id << "  " << m.name << "  Rs." << m.price << "\n";
    cout << "Enter item ID then quantity (e.g. M01 2). Type 0 to finish.\n";

    while (true) {
        string id; int qty;
        cout << "Item ID: ";
        cin >> id;
        if (id == "0") break;
        Item* it = findItem(id);
        if (!it) { cout << "Invalid item ID.\n"; continue; }
        cout << it->name << " - Quantity: ";
        if (!(cin >> qty)) { cin.clear(); cin.ignore(1000, '\n'); cout << "Invalid quantity.\n"; continue; }
        if (qty > 0) o.lines[it->id] += qty;
    }
    if (o.lines.empty()) { cout << "No items selected.\n"; return; }
    o.tokenId = nextToken++;
    o.total = calculateBill(o.lines);
    o.status = WAITING;
    allOrders[o.tokenId] = o;

    printBill(o);
    cout << "Your token: " << o.tokenId << "\n";
}
void demoSetStatus(int tokenId, Status s) {
    auto it = allOrders.find(tokenId);
    if (it == allOrders.end()) { cout << "Token not found.\n"; return; }
    it->second.status = s;
    cout << "Token " << tokenId << " is now " << statusToString(s) << ".\n";
}
int main() {
    buildIndex();
    int choice, token;
    do {
        cout << "\nMODULE D DEMO\n"
             << "1. Place order (bill)\n"
             << "2. Check order status\n"
             << "3. [Chef] Start order (Preparing)\n"
             << "4. [Chef] Mark ready\n"
             << "5. [Chef] Mark completed\n"
             << "0. Exit\nChoice: ";
        if (!(cin >> choice)) break;
        switch (choice) {
            case 1: demoPlaceOrder(); break;
            case 2: cout << "Token: "; cin >> token; displayOrderStatus(token); break;
            case 3: cout << "Token: "; cin >> token; demoSetStatus(token, PREPARING); break;
            case 4: cout << "Token: "; cin >> token; demoSetStatus(token, READY); break;
            case 5: cout << "Token: "; cin >> token; demoSetStatus(token, COMPLETED); break;
        }
    } while (choice != 0);
    return 0;
}
