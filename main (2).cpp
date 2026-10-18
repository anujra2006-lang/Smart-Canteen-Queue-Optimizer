#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <cctype>
#include <cstdlib>
using namespace std;

#include "billing.h"
#include "orderqueue.h"
#include "inventory.h"

OrderQueue chefQueue;

int readNumber(string message, int low, int high) {
    int n;
    cout << message;
    while (!(cin >> n) || n < low || n > high) {
        if (cin.eof()) {
            cout << "\nInput ended. Exiting.\n";
            exit(0);
        }
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "Invalid input, try again: ";
    }
    return n;
}

string readItemId() {
    string id;
    cout << "Item ID (0 to finish): ";
    if (!(cin >> id)) {
        cout << "\nInput ended. Exiting.\n";
        exit(0);
    }
    for (size_t i = 0; i < id.size(); i++)
        id[i] = (char)toupper((unsigned char)id[i]);
    return id;
}

void loadMenu() {
    menu.clear();
    menuIndex.clear();
    ensureInventoryFile();
    ifstream file("inventory.txt");
    if (!file.is_open()) {
        cout << "WARNING: could not open inventory.txt (is the folder writable?).\n";
        return;
    }
    string id, name;
    double price;
    int quantity, wtime;
    while (file >> id >> name >> price >> quantity >> wtime) {
        Item item;
        item.id = id;
        item.name = name;
        item.price = (int)price;
        item.wtime = wtime;
        menu.push_back(item);
    }
    file.close();
    buildIndex();
}

void showFoodMenu() {
    cout << "\nMENU\n";
    cout << left << setw(6) << "ID" << setw(22) << "Item" << "Price\n";
    for (size_t i = 0; i < menu.size(); i++)
        cout << left << setw(6) << menu[i].id << setw(22) << menu[i].name
             << "Rs. " << menu[i].price << "\n";
}

void placeOrder() {
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

    vector<OrderItem> items;
    for (map<string, int>::iterator it = order.lines.begin(); it != order.lines.end(); it++) {
        OrderItem oi;
        oi.name = findItem(it->first)->name;
        oi.quantity = it->second;
        oi.wtime = findItem(it->first)->wtime;
        items.push_back(oi);
    }
    int token = chefQueue.addOrder(to_string(order.studentId), items);
    if (token < 0) {
        cout << "Order could not be created.\n";
        return;
    }

    order.tokenId = token;
    order.total = calculateBill(order.lines);
    order.status = WAITING;
    allOrders[token] = order;

    printBill(order);
    cout << "Your token number: " << token << "\n";
    cout << "Estimated waiting time: " << chefQueue.waitingTime(token) << " minutes\n";

    for (map<string, int>::iterator it = order.lines.begin(); it != order.lines.end(); it++)
        updateStock(it->first, -it->second, false);
}

void markReady() {
    QueueOrder next;
    if (!chefQueue.processNextOrder(&next)) {
        cout << "No pending orders\n";
        return;
    }
    allOrders[next.token].status = READY;
    cout << "Token " << next.token << " is ready\n";
}

void markCompleted() {
    int token = readNumber("Token number: ", 1, 999999);
    if (allOrders.count(token) == 0 || allOrders[token].status != READY) {
        cout << "Token not found or not ready\n";
        return;
    }
    if (chefQueue.completeOrder(token))
        allOrders[token].status = COMPLETED;
    else
        cout << "Queue has no processing order with this token\n";
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
    if (order.status == COMPLETED) {
        cout << "This order is already completed\n";
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
        cout << "Cannot remove the only item of an order\n";
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
    cout << "Order updated\n";
    printBill(order);
}

int main() {
    loadMenu();
    int choice;
    do {
        cout << "\nSMART CANTEEN OPTIMIZER\n"
             << "1. Show menu\n"
             << "2. Place order\n"
             << "3. View pending orders (chef)\n"
             << "4. Mark next order ready (chef)\n"
             << "5. Mark completed (chef)\n"
             << "6. Order status and bill\n"
             << "7. Update stock\n"
             << "8. Reduce order quantity (chef)\n"
             << "0. Exit\n";
        choice = readNumber("Choice: ", 0, 8);

        if (choice == 1) showFoodMenu();
        else if (choice == 2) placeOrder();
        else if (choice == 3) chefQueue.viewPendingOrders();
        else if (choice == 4) markReady();
        else if (choice == 5) markCompleted();
        else if (choice == 6) checkStatus();
        else if (choice == 7) changeStock();
        else if (choice == 8) adjustQuantity();
    } while (choice != 0);

    cout << "Goodbye!\n";
    return 0;
}
