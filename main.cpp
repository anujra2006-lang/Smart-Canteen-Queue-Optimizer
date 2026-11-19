#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cctype>
#include <cstdlib>
using namespace std;





#include "billing.cpp"
#include "orderqueuef.cpp"
#include "inventory.cpp"

OrderQueue chefQueue;




string trim(const string& s) {
    size_t a = 0, b = s.size();
    while (a < b && isspace((unsigned char)s[a])) a++;
    while (b > a && isspace((unsigned char)s[b - 1])) b--;
    return s.substr(a, b - a);
}






string readLine() {
    string s;
    if (!getline(cin, s)) {
        cout << "\nInput ended. Exiting.\n";
        exit(0);
    }
    return trim(s);
}

bool parseInt(const string& s, int& out) {
    if (s.empty() || s.size() > 10) return false;
    size_t i = (s[0] == '-' || s[0] == '+') ? 1 : 0;
    if (i >= s.size()) return false;
    for (size_t k = i; k < s.size(); k++)
        if (!isdigit((unsigned char)s[k])) return false;
    long long v = atoll(s.c_str());
    if (v < -2000000000LL || v > 2000000000LL) return false;
    out = (int)v;
    return true;
}

int readNumber(string message, int low, int high) {
    cout << message;
    int n = 0;
    while (!parseInt(readLine(), n) || n < low || n > high)
        cout << "Invalid input, try again: ";
    return n;
}





string readItemId() {
    string id;
    while (true) {
        cout << "Item ID (0 to finish): ";
        id = readLine();
        if (!id.empty()) break;
    }
    for (size_t i = 0; i < id.size(); i++)
        id[i] = (char)toupper((unsigned char)id[i]);
    return id;
}

string tokenLabel(int token) {
    ostringstream o;
    o << "T" << setfill('0') << setw(3) << token;
    return o.str();
}

void loadMenu(bool verbose = false) {
    menu.clear();
    menuIndex.clear();
    ensureInventoryFile();
    ifstream file(inventoryPath.c_str());
    if (!file.is_open()) {
        cout << "WARNING: could not open " << inventoryPath << " (is the folder writable?).\n";
        return;
    }





    
    string line;
    int skipped = 0;
    while (getline(file, line)) {
        if (trim(line).empty()) continue;
        string id, name;
        double price;
        int quantity, wtime;
        if (!parseInvLine(line, id, name, price, quantity, wtime)) {
            skipped++;
            continue;
        }
        Item item;
        item.id = invUpper(id);
        item.name = name;
        item.price = (int)price;
        item.wtime = wtime;
        menu.push_back(item);
    }
    file.close();
    
    buildIndex();
    if (verbose && skipped > 0)
        cout << "WARNING: " << skipped << " line(s) in the inventory file could not be read and were skipped.\n";
}

void saveOrders() {
    vector<int> tokens;
    for (auto& p : allOrders) tokens.push_back(p.first);
    sort(tokens.begin(), tokens.end());

    ofstream out(dataPath("orders.txt").c_str());
    if (!out.is_open()) {
        cout << "WARNING: could not save orders to disk.\n";
        return;
    }
    for (size_t i = 0; i < tokens.size(); i++) {
        const Order& o = allOrders[tokens[i]];
        out << o.tokenId << " " << o.studentId << " " << (int)o.status << " "
            << o.total << " " << o.lines.size();
        for (auto& l : o.lines) out << " " << l.first << " " << l.second;
        out << "\n";
    }
}





void loadOrders() {
    ifstream in(dataPath("orders.txt").c_str());
    if (!in.is_open()) return;
    string line;
    int skipped = 0;
    while (getline(in, line)) {
        if (trim(line).empty()) continue;
        istringstream ls(line);
        Order o;
        o.tokenId = 0;
        o.studentId = 0;
        o.total = 0;
        int st = 0;
        size_t n = 0;
        if (!(ls >> o.tokenId >> o.studentId >> st >> o.total >> n) ||
            st < 0 || st > (int)CANCELLED || n > 1000) {
            skipped++;
            continue;
        }
        QueueOrder q;
        q.token = o.tokenId;
        q.studentId = to_string(o.studentId);
        bool ok = true;
        for (size_t i = 0; i < n; i++) {
            string id;
            int qty;
            if (!(ls >> id >> qty) || qty <= 0) { ok = false; break; }
            Item* it = findItem(id);
            if (it == nullptr) { ok = false; break; }
            o.lines[it->id] += qty;
            OrderItem oi;
            oi.name = it->name;
            oi.quantity = qty;
            oi.wtime = it->wtime;
            q.items.push_back(oi);
        }
        chefQueue.reserveToken(o.tokenId);      
        if (!ok) { skipped++; continue; }
        o.status = (Status)st;
        allOrders[o.tokenId] = o;
        if (o.status == WAITING) chefQueue.restoreOrder(q, 0);
        else if (o.status == PREPARING || o.status == READY) chefQueue.restoreOrder(q, 1);
        else if (o.status == COMPLETED) chefQueue.restoreOrder(q, 2);
    }
    if (skipped > 0)
        cout << "WARNING: " << skipped << " saved order(s) could not be restored.\n";
}

void showFoodMenu() {
    loadMenu();                        
    cout << "\nMENU\n";
    cout << left << setw(6) << "ID" << setw(22) << "Item" << setw(10) << "Price" << "Stock\n";
    for (size_t i = 0; i < menu.size(); i++) {
        int stock = getStock(menu[i].id);
        ostringstream price;
        price << "Rs. " << menu[i].price;
        cout << left << setw(6) << menu[i].id << setw(22) << menu[i].name << setw(10) << price.str();
        if (stock <= 0) cout << "Out of stock\n";
        else cout << stock << "\n";
    }
}






void placeOrder() {
    loadMenu();
    if (menu.empty()) {
        cout << "Menu is empty. Cannot place an order.\n";
        return;
    }
    Order order;
    order.studentId = readNumber("\nStudent ID: ", 1, 999999999);

    cout << "Enter item IDs. Type 0 to finish.\n";
    while (true) {
        string id = readItemId();
        if (id == "0") break;

        Item* item = findItem(id);
        if (item == nullptr) {
            cout << "Item not found\n";
            continue;
        }
        int qty = readNumber("Quantity: ", 1, 20);
        int total = qty;
        if (order.lines.count(item->id) > 0)
            total = total + order.lines[item->id];
        if (checkAvailability(item->id, total))
            order.lines[item->id] = total;
        else
            cout << item->name << " is out of stock (or not enough left)\n";
    }

    if (order.lines.empty()) {
        cout << "Order cancelled\n";
        return;
    }

    vector<pair<string, int> > taken;
    bool ok = true;
    for (map<string, int>::iterator it = order.lines.begin(); it != order.lines.end(); it++) {
        if (!updateStock(it->first, -it->second, false)) { ok = false; break; }
        taken.push_back(make_pair(it->first, it->second));
    }
    if (!ok) {
        for (size_t i = 0; i < taken.size(); i++) updateStock(taken[i].first, taken[i].second, false);
        cout << "Could not update stock. Order cancelled.\n";
        return;
    }




    
   
    vector<OrderItem> items;
    for (map<string, int>::iterator it = order.lines.begin(); it != order.lines.end(); it++) {
        Item* mi = findItem(it->first);
        OrderItem oi;
        oi.name = mi->name;
        oi.quantity = it->second;
        oi.wtime = mi->wtime;
        items.push_back(oi);
    }
    int token = chefQueue.addOrder(to_string(order.studentId), items);
    if (token < 0) {
        for (size_t i = 0; i < taken.size(); i++) updateStock(taken[i].first, taken[i].second, false);
        cout << "Order could not be created.\n";
        return;
    }

    order.tokenId = token;
    order.total = calculateBill(order.lines);
    order.status = WAITING;
    allOrders[token] = order;
    saveOrders();

    printBill(order);
    cout << "Your token number: " << token << "\n";
    cout << "Estimated waiting time: " << chefQueue.waitingTime(token) << " minutes\n";
}





void startNext() {
    QueueOrder next;
    if (!chefQueue.processNextOrder(&next)) {
        cout << "No pending orders\n";
        return;
    }
    allOrders[next.token].status = PREPARING;
    saveOrders();
    cout << "Token " << next.token << " is now being prepared\n";
}

void markReady() {
    int token = readNumber("Token number: ", 1, 999999);
    auto found = allOrders.find(token);
    if (found == allOrders.end()) {
        cout << "Token not found\n";
        return;
    }
    if (found->second.status != PREPARING) {
        cout << "Token " << token << " is " << statusToString(found->second.status)
             << " - only orders being prepared can be marked ready\n";
        return;
    }
    found->second.status = READY;
    saveOrders();
    cout << "Token " << token << " is ready\n";
}

void markCompleted() {
    int token = readNumber("Token number: ", 1, 999999);
    auto found = allOrders.find(token);
    if (found == allOrders.end() || found->second.status != READY) {
        cout << "Token not found or not ready\n";
        return;
    }
    if (chefQueue.completeOrder(token)) {
        found->second.status = COMPLETED;
        saveOrders();
    } else {
        cout << "Queue has no processing order with this token\n";
    }
}





void viewKitchenOrders() {
    const vector<QueueOrder>& list = chefQueue.processingOrders();
    if (list.empty()) {
        cout << "No orders are being prepared or waiting for pickup.\n";
        return;
    }
    cout << "\nBeing prepared / ready for pickup:\n";
    for (size_t i = 0; i < list.size(); i++) {
        cout << "Token " << tokenLabel(list[i].token)
             << " | Student ID: " << list[i].studentId
             << " | Status: " << statusToString(allOrders[list[i].token].status) << "\n";
        for (size_t k = 0; k < list[i].items.size(); k++)
            cout << "     - " << list[i].items[k].quantity << " x " << list[i].items[k].name << "\n";
    }
}

void checkStatus() {
    int token = readNumber("Token number: ", 1, 999999);
    if (allOrders.count(token) == 0) {
        cout << "Token not found\n";
        return;
    }
    displayOrderStatus(token);
    int wait = chefQueue.waitingTime(token);
    if (wait >= 0)
        cout << "Estimated waiting time: " << wait << " minutes\n";
    printBill(allOrders[token]);
}





void estimatedWait() {
    int token = readNumber("Token number: ", 1, 999999);
    auto found = allOrders.find(token);
    if (found == allOrders.end()) {
        cout << "Token not found\n";
        return;
    }
    Status s = found->second.status;
    if (s == WAITING)
        cout << "Estimated waiting time: " << chefQueue.waitingTime(token) << " minutes\n";
    else if (s == PREPARING)
        cout << "Your order is being prepared right now.\n";
    else if (s == READY)
        cout << "Your order is ready - please collect it.\n";
    else
        cout << "This order is " << statusToString(s) << ".\n";
}

void changeStock() {
    string id = readItemId();
    if (id == "0") return;
    int change = readNumber("Change in stock (+ add, - remove): ", -10000, 10000);
    updateStock(id, change);
}

void adjustQuantity() {
    int token = readNumber("Token number: ", 1, 999999);
    auto found = allOrders.find(token);
    if (found == allOrders.end()) {
        cout << "Token not found\n";
        return;
    }
    Order& order = found->second;
    if (order.status != WAITING) {
        cout << "Only orders that have not been started can be changed (this one is "
             << statusToString(order.status) << ")\n";
        return;
    }




    
    printBill(order);

    string id = readItemId();
    if (id == "0") return;
    Item* item = findItem(id);
    if (item == nullptr || order.lines.count(item->id) == 0) {
        cout << "That item is not in this order\n";
        return;
    }
    int current = order.lines[item->id];
    int amount = readNumber("Quantity to remove: ", 1, current);
    if (amount == current && order.lines.size() == 1) {
        cout << "Cannot remove the only item of an order (use Cancel order instead)\n";
        return;
    }
    if (!chefQueue.reduceItem(token, item->name, amount)) {
        cout << "Could not update the queue\n";
        return;
    }
    if (amount == current)
        order.lines.erase(item->id);
    else
        order.lines[item->id] = current - amount;
    order.total = calculateBill(order.lines);
    updateStock(item->id, amount, false);
    saveOrders();
    cout << "Order updated\n";
    printBill(order);
}






void cancelWholeOrder() {
    int token = readNumber("Token number: ", 1, 999999);
    auto found = allOrders.find(token);
    if (found == allOrders.end()) {
        cout << "Token not found\n";
        return;
    }
    Order& order = found->second;
    if (order.status != WAITING) {
        cout << "Only orders that have not been started can be cancelled (this one is "
             << statusToString(order.status) << ")\n";
        return;
    }
    if (!chefQueue.cancelOrder(token)) {
        cout << "Could not remove the order from the queue\n";
        return;
    }
    for (map<string, int>::iterator it = order.lines.begin(); it != order.lines.end(); it++)
        updateStock(it->first, it->second, false);
    order.status = CANCELLED;
    saveOrders();
    cout << "Token " << token << " cancelled and stock returned\n";
}

int main(int argc, char* argv[]) {
    locateInventory(argc > 0 ? argv[0] : "");
    loadMenu(true);
    loadOrders();
    cout << "Using inventory file: " << inventoryPath << "\n";

    int choice;
    do {
        cout << "\nSMART CANTEEN OPTIMIZER\n"
             << " 1. Show menu\n"
             << " 2. Place order\n"
             << " 3. View pending orders (chef)\n"
             << " 4. Start next order (chef)\n"
             << " 5. Mark ready (chef)\n"
             << " 6. Mark completed (chef)\n"
             << " 7. Order status and bill\n"
             << " 8. Update stock\n"
             << " 9. Estimated waiting time\n"
             << "10. Reduce order quantity (chef)\n"
             << "11. Cancel order (chef)\n"
             << "12. View orders being prepared / ready (chef)\n"
             << " 0. Exit\n";
        choice = readNumber("Choice: ", 0, 12);



        

        if (choice == 1) showFoodMenu();
        else if (choice == 2) placeOrder();
        else if (choice == 3) chefQueue.viewPendingOrders();
        else if (choice == 4) startNext();
        else if (choice == 5) markReady();
        else if (choice == 6) markCompleted();
        else if (choice == 7) checkStatus();
        else if (choice == 8) changeStock();
        else if (choice == 9) estimatedWait();
        else if (choice == 10) adjustQuantity();
        else if (choice == 11) cancelWholeOrder();
        else if (choice == 12) viewKitchenOrders();
    } while (choice != 0);

    cout << "Goodbye!\n";
    return 0;
}
