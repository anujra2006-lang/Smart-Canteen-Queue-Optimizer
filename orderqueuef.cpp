#include <iostream>
#include <iomanip>
#include <limits>
#include <deque>
#include <sstream>
#include <string>
#include <vector>
#include <cstddef>
using namespace std;

struct OrderItem {
    int menuId{0};
    string name;
    int quantity{1};
    int wtime{0};
};

struct QueueOrder {
    int token{0};
    string studentId;
    vector<OrderItem> items;
    string status{"Pending"};
};

class OrderQueue {
private:
    deque<QueueOrder> pending;
    vector<QueueOrder> processing;
    vector<QueueOrder> completed;
    int nextToken = 1;

    static string tokenText(int token) {
        ostringstream out;
        out << "T" << setfill('0') << setw(3) << token;
        return out.str();
    }

    static void printOrder(const QueueOrder& o) {
        cout << "Token " << tokenText(o.token)
                  << " | Student ID: " << o.studentId
                  << " | Status: " << o.status << "\n";
        for (const auto& item : o.items)
            cout << "     - " << item.quantity << " x " << item.name << "\n";
    }

    static int orderTime(const QueueOrder& o) {
        int total = 0;
        for (const auto& item : o.items)
            total += item.wtime * item.quantity;
        return total;
    }

    static bool reduceIn(QueueOrder& o, const string& name, int amount) {
        for (size_t i = 0; i < o.items.size(); ++i) {
            if (o.items[i].name != name) continue;
            if (amount >= o.items[i].quantity)
                o.items.erase(o.items.begin() + i);
            else
                o.items[i].quantity -= amount;
            return true;
        }
        return false;
    }

    static void clearInput() {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

public:

    int addOrder(const string& studentId, const vector<OrderItem>& items) {
        if (studentId.empty() || items.empty()) return -1;
        for (const auto& item : items)
            if (item.name.empty() || item.quantity <= 0) return -1;

        QueueOrder order;
        order.token = nextToken++;
        order.studentId = studentId;
        order.items = items;
        order.status = "Pending";
        pending.push_back(order);
        return order.token;
    }

    bool processNextOrder(QueueOrder* out = nullptr) {
        if (pending.empty()) return false;
        QueueOrder order = pending.front();
        pending.pop_front();
        order.status = "Processing";
        processing.push_back(order);
        if (out) *out = order;
        return true;
    }

    bool completeOrder(int token) {
        for (size_t i = 0; i < processing.size(); ++i) {
            if (processing[i].token == token) {
                QueueOrder order = processing[i];
                processing.erase(processing.begin() + i);
                order.status = "Completed";
                completed.push_back(order);
                cout << "\nOrder completed!\n";
                printOrder(order);
                cout << ">>> Please collect order with TOKEN " << tokenText(token)
                          << " (Student ID: " << order.studentId << ") <<<\n";
                return true;
            }
        }
        return false;
    }

    void viewPendingOrders() const {
        if (pending.empty()) {
            cout << "No pending orders.\n";
            return;
        }
        cout << "\nPending orders (" << pending.size() << "):\n";
        int running = 0;
        for (const auto& o : pending) {
            running += orderTime(o);
            printOrder(o);
            cout << "     Estimated waiting time: " << running << " min\n";
        }
    }

    int waitingTime(int token) const {
        int total = 0;
        for (const auto& o : pending) {
            total += orderTime(o);
            if (o.token == token) return total;
        }
        return -1;
    }

    bool reduceItem(int token, const string& name, int amount) {
        for (auto& o : pending)
            if (o.token == token) return reduceIn(o, name, amount);
        for (auto& o : processing)
            if (o.token == token) return reduceIn(o, name, amount);
        return false;
    }

    // Remove a still-pending order (used for cancelling).
    bool cancelOrder(int token) {
        for (size_t i = 0; i < pending.size(); ++i) {
            if (pending[i].token == token) {
                pending.erase(pending.begin() + i);
                return true;
            }
        }
        return false;
    }

    // Put a saved order back into the queue (where: 0 pending, 1 processing, 2 completed).
    void restoreOrder(const QueueOrder& o, int where) {
        QueueOrder c = o;
        if (where == 0)      { c.status = "Pending";    pending.push_back(c); }
        else if (where == 1) { c.status = "Processing"; processing.push_back(c); }
        else                 { c.status = "Completed";  completed.push_back(c); }
        reserveToken(c.token);
    }

    // Make sure this token number is never handed out again.
    void reserveToken(int token) {
        if (token >= nextToken) nextToken = token + 1;
    }

    bool hasPending() const { return !pending.empty(); }
    size_t pendingCount() const { return pending.size(); }
    const vector<QueueOrder>& processingOrders() const { return processing; }
    const vector<QueueOrder>& completedOrders() const { return completed; }

    void addOrderInteractive() {
        string studentId;
        cout << "Student ID: ";
        cin >> studentId;
        clearInput();

        vector<OrderItem> items;
        while (true) {
            string name;
            cout << "Item name (blank to finish): ";
            getline(cin, name);
            if (name.empty()) break;

            int qty;
            cout << "Quantity: ";
            if (!(cin >> qty) || qty <= 0) {
                clearInput();
                cout << "Quantity must be a positive number. Item skipped.\n";
                continue;
            }
            clearInput();
            OrderItem item;
            item.name = name;
            item.quantity = qty;
            items.push_back(item);
        }

        int token = addOrder(studentId, items);
        if (token < 0) cout << "Order not created (needs a student ID and at least one item).\n";
        else cout << "Order added. Your token number is " << tokenText(token) << "\n";
    }

    void processNextInteractive() {
        QueueOrder order;
        if (!processNextOrder(&order)) {
            cout << "No pending orders to process.\n";
            return;
        }
        cout << "\nNow processing:\n";
        printOrder(order);
    }

    void completeInteractive() {
        if (processing.empty()) {
            cout << "No orders are currently being processed.\n";
            return;
        }
        cout << "\nOrders being processed:\n";
        for (const auto& o : processing) printOrder(o);

        cout << "Enter token number to complete (digits only, e.g. 1): ";
        int token;
        if (!(cin >> token)) {
            clearInput();
            cout << "Invalid token.\n";
            return;
        }
        clearInput();
        if (!completeOrder(token)) cout << "No processing order with that token.\n";
    }

    void runMenu() {
        while (true) {
            cout << "\nORDER QUEUE\n"
                      << "1. Add order\n"
                      << "2. Process next order\n"
                      << "3. Complete order\n"
                      << "4. View pending orders\n"
                      << "0. Back\n"
                      << "Choose an option: ";
            int choice;
            if (!(cin >> choice)) {
                if (cin.eof()) return;
                clearInput();
                cout << "Invalid option.\n";
                continue;
            }
            clearInput();
            switch (choice) {
                case 1: addOrderInteractive(); break;
                case 2: processNextInteractive(); break;
                case 3: completeInteractive(); break;
                case 4: viewPendingOrders(); break;
                case 0: return;
                default: cout << "Invalid option.\n";
            }
        }
    }
};

int main() {
    OrderQueue orders;
    orders.runMenu();
    return 0;
}
