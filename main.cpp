#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
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

int readNumber(string message, int low, int high) {
    int n;
    cout << message;
    cin >> n;
    while (cin.fail() || n < low || n > high) {
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "Invalid input, try again: ";
        cin >> n;
    }
    return n;
}

string readItemId() {
    string id;
    cout << "Item ID: ";
    cin >> id;
    for (int i = 0; i < id.size(); i++)
        id[i] = toupper(id[i]);
    return id;
}

void loadMenu() {
    menu.clear();
    menuIndex.clear();
    ifstream file("inventory.txt");
    string id, name;
    double price;
    int quantity, wtime;
    while (file >> id >> name >> price >> quantity >> wtime) {
        Item item;
        item.id = id;
        item.name = name;
        item.price = (int)price;
        menu.push_back(item);
    }
    file.close();
    buildIndex();
}

void showFoodMenu() {
    cout << "\nMenu\n";
    for (int i = 0; i < menu.size(); i++)
        cout << setw(6) << menu[i].id << setw(15) << menu[i].name << "Rs. " << menu[i].price << "\n";
}

void placeOrder() {
    showFoodMenu();
    Order order;
    order.studentId = readNumber("\nStudent ID: ", 1, 999999999);

    cout << "Enter item IDs, 0 to finish\n";
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
        if (order.lines.count(id) > 0)
            total = total + order.lines[id];
        if (checkAvailability(id, total))
            order.lines[id] = total;
        else
            cout << item->name << " is out of stock\n";
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
        items.push_back(oi);
    }
    int token = chefQueue.addOrder(to_string(order.studentId), items);

    order.tokenId = token;
    order.total = calculateBill(order.lines);
    order.status = WAITING;
    allOrders[token] = order;
    printBill(order);

    for (map<string, int>::iterator it = order.lines.begin(); it != order.lines.end(); it++)
        updateStock(it->first, -it->second);
}

void startNextOrder() {
    QueueOrder next;
    if (chefQueue.processNextOrder(&next)) {
        allOrders[next.token].status = PREPARING;
        cout << "Token " << next.token << " is preparing\n";
    } else {
        cout << "No pending orders\n";
    }
}

void markReady() {
    int token = readNumber("Token number: ", 1, 999999);
    if (allOrders.count(token) == 0 || allOrders[token].status != PREPARING) {
        cout << "Token not found or not preparing\n";
        return;
    }
    allOrders[token].status = READY;
    cout << "Token " << token << " is ready\n";
}

void markCompleted() {
    int token = readNumber("Token number: ", 1, 999999);
    if (allOrders.count(token) == 0 || allOrders[token].status != READY) {
        cout << "Token not found or not ready\n";
        return;
    }
    chefQueue.completeOrder(token);
    allOrders[token].status = COMPLETED;
}

void checkStatus() {
    int token = readNumber("Token number: ", 1, 999999);
    if (allOrders.count(token) == 0) {
        cout << "Token not found\n";
        return;
    }
    displayOrderStatus(token);
    printBill(allOrders[token]);
}

void changeStock() {
    string id = readItemId();
    int change = readNumber("Change in stock: ", -10000, 10000);
    updateStock(id, change);
}

int main() {
    loadMenu();
    int choice;
    do {
        cout << "\n1. Menu\n2. Place order\n3. Pending orders\n"
             << "4. Start next order\n5. Mark ready\n6. Mark completed\n"
             << "7. Order status\n8. Update stock\n0. Exit\n";
        choice = readNumber("Choice: ", 0, 8);

        if (choice == 1) showFoodMenu();
        else if (choice == 2) placeOrder();
        else if (choice == 3) chefQueue.viewPendingOrders();
        else if (choice == 4) startNextOrder();
        else if (choice == 5) markReady();
        else if (choice == 6) markCompleted();
        else if (choice == 7) checkStatus();
        else if (choice == 8) changeStock();
    } while (choice != 0);

    return 0;
}
