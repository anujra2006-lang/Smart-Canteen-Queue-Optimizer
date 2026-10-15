// ============================================================
//  MODULE D: Billing and Waiting Time
//    14. Calculate Bill
//    15. Estimate Waiting Time
//    16. Display Order Status
//
//  Other modules (menu, inventory, order placement, chef queue)
//  are written by teammates. The "DEMO ONLY" part at the bottom
//  is just there so this module can be run and shown on its own.
// ============================================================
#include <iostream>
#include <iomanip>
#include <deque>
#include <map>
#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>
using namespace std;

// ---------------- Shared data (agree on these with the team) ----------------
enum Status { WAITING, PREPARING, READY, COMPLETED };

struct Item {
    string id, name;
    int price;       // Rs.
    int prepTime;    // minutes
};

struct Order {
    int tokenId;
    int studentId;
    map<string, int> lines;   // item ID -> quantity
    int total;                // Rs.
    int prepTime;             // minutes for the whole order
    Status status;
};

vector<Item> menu = {
    // Meals
    {"M01", "Veg Thali", 90, 8},
    {"M02", "Rajma Chawal", 70, 6},
    {"M03", "Chole Bhature", 80, 10},
    {"M04", "Masala Dosa", 60, 10},
    {"M05", "Veg Fried Rice", 60, 8},
    {"M06", "Paneer Butter Masala", 110, 12},
    {"M07", "Dal Rice", 50, 5},
    {"M08", "Veg Biryani", 80, 8},
    {"M09", "Aloo Paratha", 45, 8},
    {"M10", "Pav Bhaji", 60, 7},
    {"M11", "Veg Noodles", 60, 8},
    {"M12", "Chole Rice", 65, 6},
    {"M13", "Paneer Wrap", 80, 8},
    {"M14", "Idli Sambar", 40, 5},
    {"M15", "Veg Burger Meal", 90, 10},
    // Beverages
    {"B01", "Masala Tea", 15, 3},
    {"B02", "Black Coffee", 20, 2},
    {"B03", "Cold Coffee", 40, 4},
    {"B04", "Lemonade", 25, 2},
    {"B05", "Mango Shake", 45, 4},
    {"B06", "Banana Shake", 40, 4},
    {"B07", "Orange Juice", 35, 3},
    {"B08", "Sweet Lassi", 30, 2},
    {"B09", "Salted Lassi", 30, 2},
    {"B10", "Hot Chocolate", 40, 4},
    {"B11", "Iced Tea", 30, 2},
    {"B12", "Buttermilk", 20, 1},
    {"B13", "Strawberry Shake", 45, 4},
    {"B14", "Coconut Water", 35, 1},
    {"B15", "Mineral Water (500 ml)", 20, 1},
    // Starters and snacks
    {"S01", "Samosa (2 pcs)", 25, 3},
    {"S02", "Veg Momos (6 pcs)", 50, 8},
    {"S03", "Fried Momos (6 pcs)", 60, 8},
    {"S04", "French Fries", 50, 6},
    {"S05", "Veg Sandwich", 40, 4},
    {"S06", "Paneer Pakora", 60, 7},
    {"S07", "Onion Pakora", 35, 6},
    {"S08", "Spring Rolls (4 pcs)", 50, 7},
    {"S09", "Aloo Tikki (2 pcs)", 35, 5},
    {"S10", "Bread Pakora", 30, 5},
    {"S11", "Veg Cutlet (2 pcs)", 35, 5},
    {"S12", "Cheese Balls (6 pcs)", 60, 7},
    {"S13", "Garlic Bread", 45, 5},
    {"S14", "Chilli Potato", 60, 7},
    {"S15", "Dhokla", 35, 2},
};

unordered_map<string, int> menuIndex;   // item ID -> position in menu (O(1) lookup)
unordered_map<int, Order> allOrders;    // token ID -> order details + status
deque<int> orderQueue;                  // token IDs of WAITING orders (FIFO)
int nextToken = 1;

// ---------------- Helpers ----------------
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

// ============================================================
//  14. CALCULATE BILL        Total = Qty x Price (for each item)
// ============================================================
int calculateBill(const map<string, int>& lines) {
    int total = 0;
    for (auto& l : lines)
        total += findItem(l.first)->price * l.second;
    return total;
}

// Prep time of a whole order = sum of (Qty x prep time of each item)
int calculatePrepTime(const map<string, int>& lines) {
    int minutes = 0;
    for (auto& l : lines)
        minutes += findItem(l.first)->prepTime * l.second;
    return minutes;
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
    cout << "ESTIMATED PREP TIME FOR ORDER: " << o.prepTime << " min\n";
}

// ============================================================
//  15. ESTIMATE WAITING TIME
//      Wait = total prep time of the orders ahead in the queue
// ============================================================
int ordersAhead(int tokenId) {
    int pos = 0;
    for (int t : orderQueue) {
        if (t == tokenId) return pos;
        pos++;
    }
    return -1;   // not in queue
}

int estimateWaitTime(int tokenId) {
    int wait = 0;
    for (int t : orderQueue) {
        if (t == tokenId) break;
        wait += allOrders[t].prepTime;
    }
    return wait;
}

// For a new order that has not been added to the queue yet:
// everyone already in the queue is ahead of it.
int estimateWaitForNewOrder() {
    int wait = 0;
    for (int t : orderQueue) wait += allOrders[t].prepTime;
    return wait;
}

// ============================================================
//  16. DISPLAY ORDER STATUS   (Waiting / Preparing / Ready / Completed)
// ============================================================
void displayOrderStatus(int tokenId) {
    auto it = allOrders.find(tokenId);
    if (it == allOrders.end()) { cout << "Token not found.\n"; return; }

    const Order& o = it->second;
    cout << "Token " << tokenId << " status: " << statusToString(o.status) << "\n";

    if (o.status == WAITING) {
        int wait = estimateWaitTime(tokenId);
        cout << "Orders ahead: " << ordersAhead(tokenId)
             << " | Estimated wait: " << wait << " min"
             << " | Ready in about: " << wait + o.prepTime << " min\n";
    }
}
// ============================================================
//  DEMO ONLY: stands in for the teammates' modules
//  (order placement and the chef's queue). Remove when merging.
// ============================================================
void demoPlaceOrder() {
    Order o;
    cout << "Student ID: ";
    cin >> o.studentId;
    cout << "Enter item ID then quantity (e.g. M01 2). Type 0 to finish.\n";

    while (true) {
        string id; int qty;
        cout << "Item ID: ";
        cin >> id;
        if (id == "0") break;
        Item* it = findItem(id);
        if (!it) { cout << "Invalid item ID.\n"; continue; }
        cout << it->name << " - Quantity: ";
        cin >> qty;
        if (qty > 0) o.lines[it->id] += qty;
    }
    if (o.lines.empty()) { cout << "No items selected.\n"; return; }

    o.tokenId = nextToken++;
    o.total = calculateBill(o.lines);          // module D: item 14
    o.prepTime = calculatePrepTime(o.lines);
    o.status = WAITING;

    int ahead = orderQueue.size();
    int wait = estimateWaitForNewOrder();      // module D: item 15

    allOrders[o.tokenId] = o;
    orderQueue.push_back(o.tokenId);           // enqueue (teammate's module)

    printBill(o);
    cout << "Your token: " << o.tokenId << "\n";
    cout << "Orders ahead: " << ahead << " | Estimated wait: " << wait
         << " min | Ready in about: " << wait + o.prepTime << " min\n";
}

void demoChefAdvance() {
    // Chef takes the next order (dequeue), then moves it along
    if (orderQueue.empty()) { cout << "No orders in queue.\n"; return; }
    int t = orderQueue.front();
    orderQueue.pop_front();
    allOrders[t].status = PREPARING;
    cout << "Token " << t << " is now Preparing.\n";
}

void demoSetStatus(int tokenId, Status s) {
    auto it = allOrders.find(tokenId);
    if (it == allOrders.end()) { cout << "Token not found.\n"; return; }
    if (it->second.status == WAITING) { cout << "Still in queue. Start it first.\n"; return; }
    it->second.status = s;
    cout << "Token " << tokenId << " is now " << statusToString(s) << ".\n";
}

int main() {
    buildIndex();
    int choice, token;
    do {
        cout << "\n===== MODULE D DEMO =====\n"
             << "1. Place order (bill + wait time)\n"
             << "2. Check order status\n"
             << "3. [Chef] Start next order\n"
             << "4. [Chef] Mark ready\n"
             << "5. [Chef] Mark completed\n"
             << "0. Exit\nChoice: ";
        cin >> choice;
        switch (choice) {
            case 1: demoPlaceOrder(); break;
            case 2: cout << "Token: "; cin >> token; displayOrderStatus(token); break;
            case 3: demoChefAdvance(); break;
            case 4: cout << "Token: "; cin >> token; demoSetStatus(token, READY); break;
            case 5: cout << "Token: "; cin >> token; demoSetStatus(token, COMPLETED); break;
        }
    } while (choice != 0);
    return 0;
}
