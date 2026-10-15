// ============================================================
// ORDER QUEUE MODULE  (Module B: items 5-8)
//   5. Add Order (Enqueue)
//   6. View Pending Orders
//   7. Process Next Order (Dequeue)
//   8. Complete Order
//
// Header-only, depends only on the C++ standard library.
// Uses std::queue<QueueOrder> for the pending queue.
//
// HOW TO CONNECT WITH OTHER MODULES
//   - Place order screen  : call  orders.addOrder(studentId, items)
//   - Staff screen        : call  orders.viewPendingOrders(),
//                                 orders.processNextOrder(),
//                                 orders.completeOrder(token)
//   - Or just call        : orders.runMenu()   (ready-made console menu)
//   Names are prefixed (QueueOrder, OrderItem) so they will not clash
//   with an "Order" struct written by someone else.
// ============================================================

#include <iostream>
#include <iomanip>
#include <limits>
#include <queue>
#include <sstream>
#include <string>
#include <vector>

// One line of an order, e.g. 2 x Samosa.
// menuId can be linked to the Menu module; use 0 if not known.
struct OrderItem {
    int menuId{0};
    std::string name;
    int quantity{1};
};

struct QueueOrder {
    int token{0};                 // token number shown to the student
    std::string studentId;
    std::vector<OrderItem> items;
    std::string status{"Pending"}; // Pending -> Processing -> Completed
};

class OrderQueue {
private:
    std::queue<QueueOrder> pending;        // waiting orders (FIFO)
    std::vector<QueueOrder> processing;    // dequeued, being prepared
    std::vector<QueueOrder> completed;     // finished orders
    int nextToken = 1;

    static std::string tokenText(int token) {
        std::ostringstream out;
        out << "T" << std::setfill('0') << std::setw(3) << token;
        return out.str();
    }

    static void printOrder(const QueueOrder& o) {
        std::cout << "Token " << tokenText(o.token)
                  << " | Student ID: " << o.studentId
                  << " | Status: " << o.status << "\n";
        for (const auto& item : o.items)
            std::cout << "     - " << item.quantity << " x " << item.name << "\n";
    }

    static void clearInput() {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

public:
    // ---------- 5. Add Order (Enqueue) ----------
    // Adds a new order to the rear of the queue.
    // Returns the token number, or -1 if the order is invalid.
    int addOrder(const std::string& studentId, const std::vector<OrderItem>& items) {
        if (studentId.empty() || items.empty()) return -1;
        for (const auto& item : items)
            if (item.name.empty() || item.quantity <= 0) return -1;

        QueueOrder order;
        order.token = nextToken++;
        order.studentId = studentId;
        order.items = items;
        order.status = "Pending";
        pending.push(order);
        return order.token;
    }

    // ---------- 6. View Pending Orders ----------
    // Shows every waiting order (front of the queue first).
    void viewPendingOrders() const {
        std::cout << "\n--- Pending Orders ---\n";
        if (pending.empty()) {
            std::cout << "No pending orders.\n";
            return;
        }
        std::queue<QueueOrder> copy = pending; // copy so the real queue is untouched
        int position = 1;
        while (!copy.empty()) {
            std::cout << "[" << position++ << "] ";
            printOrder(copy.front());
            copy.pop();
        }
    }

    // ---------- 7. Process Next Order (Dequeue) ----------
    // Takes the front order out of the pending queue and starts processing it.
    // Returns false if the queue is empty. 'out' (optional) receives the order.
    bool processNextOrder(QueueOrder* out = nullptr) {
        if (pending.empty()) return false;
        QueueOrder order = pending.front();
        pending.pop();
        order.status = "Processing";
        processing.push_back(order);
        if (out) *out = order;
        return true;
    }

    // ---------- 8. Complete Order ----------
    // Marks a processing order as completed and shows its token for collection.
    // Returns false if no processing order has that token.
    bool completeOrder(int token) {
        for (size_t i = 0; i < processing.size(); ++i) {
            if (processing[i].token == token) {
                QueueOrder order = processing[i];
                processing.erase(processing.begin() + i);
                order.status = "Completed";
                completed.push_back(order);
                std::cout << "\nOrder completed!\n";
                printOrder(order);
                std::cout << ">>> Please collect order with TOKEN " << tokenText(token)
                          << " (Student ID: " << order.studentId << ") <<<\n";
                return true;
            }
        }
        return false;
    }

    // ---------- Helpers for other modules ----------
    bool hasPending() const { return !pending.empty(); }
    size_t pendingCount() const { return pending.size(); }
    const std::vector<QueueOrder>& processingOrders() const { return processing; }
    const std::vector<QueueOrder>& completedOrders() const { return completed; }

    // ---------- Console UI ----------
    void addOrderInteractive() {
        std::string studentId;
        std::cout << "Student ID: ";
        std::cin >> studentId;
        clearInput();

        std::vector<OrderItem> items;
        while (true) {
            std::string name;
            std::cout << "Item name (blank to finish): ";
            std::getline(std::cin, name);
            if (name.empty()) break;

            int qty;
            std::cout << "Quantity: ";
            if (!(std::cin >> qty) || qty <= 0) {
                clearInput();
                std::cout << "Quantity must be a positive number. Item skipped.\n";
                continue;
            }
            clearInput();
            OrderItem item;
            item.name = name;
            item.quantity = qty;
            items.push_back(item);
        }

        int token = addOrder(studentId, items);
        if (token < 0) std::cout << "Order not created (needs a student ID and at least one item).\n";
        else std::cout << "Order added. Your token number is " << tokenText(token) << "\n";
    }

    void processNextInteractive() {
        QueueOrder order;
        if (!processNextOrder(&order)) {
            std::cout << "No pending orders to process.\n";
            return;
        }
        std::cout << "\nNow processing:\n";
        printOrder(order);
    }

    void completeInteractive() {
        if (processing.empty()) {
            std::cout << "No orders are currently being processed.\n";
            return;
        }
        std::cout << "\nOrders being processed:\n";
        for (const auto& o : processing) printOrder(o);

        std::cout << "Enter token number to complete (digits only, e.g. 1): ";
        int token;
        if (!(std::cin >> token)) {
            clearInput();
            std::cout << "Invalid token.\n";
            return;
        }
        clearInput();
        if (!completeOrder(token)) std::cout << "No processing order with that token.\n";
    }

    // Ready-made menu. Call from main() or from the main menu of the full system.
    void runMenu() {
        while (true) {
            std::cout << "\n========== ORDER QUEUE ==========\n"
                      << "1. Add order\n"
                      << "2. View pending orders\n"
                      << "3. Process next order\n"
                      << "4. Complete order\n"
                      << "0. Back\n"
                      << "Choose an option: ";
            int choice;
            if (!(std::cin >> choice)) {
                if (std::cin.eof()) return;
                clearInput();
                std::cout << "Invalid option.\n";
                continue;
            }
            clearInput();
            switch (choice) {
                case 1: addOrderInteractive(); break;
                case 2: viewPendingOrders(); break;
                case 3: processNextInteractive(); break;
                case 4: completeInteractive(); break;
                case 0: return;
                default: std::cout << "Invalid option.\n";
            }
        }
    }
};


int main() {
    OrderQueue orders;
    orders.runMenu();
    return 0;
}