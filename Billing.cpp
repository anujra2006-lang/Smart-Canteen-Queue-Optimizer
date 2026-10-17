#include <iostream>
#include <iomanip>
#include <map>
#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>
using namespace std;

enum Status { WAITING, PREPARING, READY, COMPLETED };

struct Item {
    string id, name;
    int price;             
};

struct Order {
    int tokenId;
    int studentId;
    map<string, int> lines;  
    int total;               
    Status status;
};

vector<Item> menu;

unordered_map<string, int> menuIndex;  
unordered_map<int, Order> allOrders;  
int nextToken = 1;

void buildIndex() {
    for (int i = 0; i < (int)menu.size(); i++) menuIndex[menu[i].id] = i;
}

Item* findItem(string id) {
    transform(id.begin(), id.end(), id.begin(), ::toupper);
    auto it = menuIndex.find(id);
    return it == menuIndex.end() ? nullptr : &menu[it->second];
}

string statusToString(Status s) {
    switch (s) {
        case WAITING:   return "Waiting";
        case PREPARING: return "Preparing";
        case READY:     return "Ready";
        case COMPLETED: return "Completed";
    }
    return "Unknown";
}

int calculateBill(const map<string, int>& lines) {
    int total = 0;
    for (auto& l : lines)
        total += findItem(l.first)->price * l.second;
    return total;
}

void printBill(const Order& o) {
    cout << "\n========== BILL (Token " << o.tokenId << ") ==========\n";
    cout << left << setw(5) << "ID" << setw(24) << "Item" << setw(5) << "Qty"
         << setw(8) << "Price" << "Total\n";
    for (auto& l : o.lines) {
        Item* it = findItem(l.first);
        cout << left << setw(5) << it->id << setw(24) << it->name << setw(5) << l.second
             << setw(8) << it->price << it->price * l.second << "\n";
    }
    cout << string(46, '-') << "\n";
    cout << "GRAND TOTAL (Rs.): " << o.total << "\n";
}

void displayOrderStatus(int tokenId) {
    auto it = allOrders.find(tokenId);
    if (it == allOrders.end()) { cout << "Token not found.\n"; return; }

    cout << "Token " << tokenId << " status: "
         << statusToString(it->second.status) << "\n";
}
