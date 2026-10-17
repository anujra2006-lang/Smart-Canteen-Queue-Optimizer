#include <iostream>
#include <iomanip>
#include <limits>
#include <queue>
#include <sstream>
#include <string>
#include <vector>


struct OrderItem {
    int menuId{0};
    std::string name;
    int quantity{1};
};

struct QueueOrder {
    int token{0};                 
    std::string studentId;
    std::vector<OrderItem> items;
    std::string status{"Pending"}; 
};

class OrderQueue {
private:
    std::queue<QueueOrder> pending;        
    std::vector<QueueOrder> processing;    
    std::vector<QueueOrder> completed;     
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

    
    bool processNextOrder(QueueOrder* out = nullptr) {
        if (pending.empty()) return false;
        QueueOrder order = pending.front();
        pending.pop();
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
                std::cout << "\nOrder completed!\n";
                printOrder(order);
                std::cout << ">>> Please collect order with TOKEN " << tokenText(token)
                          << " (Student ID: " << order.studentId << ") <<<\n";
                return true;
            }
        }
        return false;
    }

    bool hasPending() const { return !pending.empty(); }
    size_t pendingCount() const { return pending.size(); }
    const std::vector<QueueOrder>& processingOrders() const { return processing; }
    const std::vector<QueueOrder>& completedOrders() const { return completed; }

    
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

   
    void runMenu() {
        while (true) {
            std::cout << "\n========== ORDER QUEUE ==========\n"
                      << "1. Add order\n"
                      << "2. Process next order\n"
                      << "3. Complete order\n"
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
                case 2: processNextInteractive(); break;
                case 3: completeInteractive(); break;
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
