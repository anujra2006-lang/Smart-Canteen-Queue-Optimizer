#include <iostream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <cctype>

#define main moduleD_demo_main
#include "module_d_billing_wait.cpp"
#undef main
#define main orderqueue_demo_main
#include "orderqueue.cpp"
#undef main
#define main inventory_demo_main
#include "inventory.cpp"
#undef main

OrderQueue chefQueue;

int readInt(const string& prompt, int low, int high) {
    int v;
    while (true) {
        cout << prompt;
        if (cin >> v && v >= low && v <= high) { cin.ignore(numeric_limits<streamsize>::max(), '\n'); return v; }
        if (cin.eof()) exit(0);
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "  Enter a number from " << low << " to " << high << ".\n";
    }
}

string readItemId(const string& prompt) {
    string id;
    cout << prompt;
    if (!(cin >> id)) exit(0);
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    for (char& c : id) c = toupper(c);
    return id;
}

void loadMenu() {
    menu.clear();
    menuIndex.clear();
    ifstream f("inventory.txt");
    string text, id, name;
    double price;
    int qty;
    while (getline(f, text)) {
        istringstream row(text);
        if (row >> id >> name >> price >> qty) menu.push_back({id, name, (int)(price + 0.5)});
    }
    buildIndex();
}

void showFoodMenu() {
    cout << "\nFood Menu\n";
    for (auto& it : menu) cout << left << setw(6) << it.id << setw(26) << it.name << it.price << "\n";
}

void placeOrderScreen() {
    showFoodMenu();
    Order o;
    o.studentId = readInt("\nEnter Student ID (number): ", 1, 999999999);
    cout << "Item ID to add (e.g. M01), 0 = finish.\n";
    while (true) {
        string id = readItemId("Item ID: ");
        if (id == "0") break;
        Item* it = findItem(id);
        if (!it) { cout << "  No such item.\n"; continue; }
        int needed = readInt("  Quantity (1-20): ", 1, 20) + o.lines[it->id];
        if (!checkAvailability(it->id, needed)) {
            cout << "  Sorry, " << it->name << " is unavailable or not enough stock.\n";
            if (o.lines[it->id] == 0) o.lines.erase(it->id);
            continue;
        }
        o.lines[it->id] = needed;
    }
    if (o.lines.empty()) { cout << "No items selected. Order cancelled.\n"; return; }

    vector<OrderItem> items;
    for (auto& l : o.lines) items.push_back({menuIndex[l.first] + 1, findItem(l.first)->name, l.second});
    int token = chefQueue.addOrder(to_string(o.studentId), items);
    if (token < 0) { cout << "Order could not be created.\n"; return; }

    o.tokenId = token;
    o.total = calculateBill(o.lines);
    o.status = WAITING;
    allOrders[token] = o;
    printBill(o);
    for (auto& l : o.lines) updateStock(l.first, -l.second);
}

void chefScreen(int action) {
    if (action == 4) {
        QueueOrder q;
        if (!chefQueue.processNextOrder(&q)) { cout << "No pending orders.\n"; return; }
        allOrders[q.token].status = PREPARING;
        cout << "Token " << q.token << " is now Preparing.\n";
        return;
    }
    Status need = (action == 5) ? PREPARING : READY, done = (action == 5) ? READY : COMPLETED;
    int token = readInt("Enter token number: ", 1, 999999);
    auto it = allOrders.find(token);
    if (it == allOrders.end() || it->second.status != need) {
        cout << "Token not found, or it is not " << statusToString(need) << ".\n";
        return;
    }
    if (done == COMPLETED && !chefQueue.completeOrder(token)) return;
    it->second.status = done;
    cout << "Token " << token << " is now " << statusToString(done) << ".\n";
}

void statusScreen() {
    int token = readInt("Enter token number: ", 1, 999999);
    if (!allOrders.count(token)) { cout << "Token not found.\n"; return; }
    displayOrderStatus(token);
    printBill(allOrders[token]);
}

int main() {
    loadMenu();
    cout << "\nCanteen Quiz Optimizer\n";
    int choice;
    do {
        cout << "\n 1. View food menu\n 2. Place an order\n 3. View pending queue\n"
             << " 4. [Chef] Start next order\n 5. [Chef] Mark ready\n 6. [Chef] Mark completed\n"
             << " 7. Order status / bill\n 8. Add / remove stock\n 0. Exit\n";
        choice = readInt("Enter your choice: ", 0, 8);
        switch (choice) {
            case 1: showFoodMenu();                  break;
            case 2: placeOrderScreen();              break;
            case 3: chefQueue.viewPendingOrders();   break;
            case 4: case 5: case 6: chefScreen(choice); break;
            case 7: statusScreen();                  break;
            case 8: { string id = readItemId("Item ID: ");
                      updateStock(id, readInt("Change (+ add, - remove): ", -10000, 10000)); } break;
        }
    } while (choice != 0);
    cout << "Thank you! Goodbye.\n";
    return 0;
}
