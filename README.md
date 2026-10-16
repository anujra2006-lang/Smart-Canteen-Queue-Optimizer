# Smart Canteen Queue Optimizer

A C++ console application that replaces the traditional first-come-first-served physical canteen line with a structured digital FIFO queue. It applies core Data Structures and Algorithms (DSA) — queues, linked lists, hash maps, and search operations — to manage order flow, live inventory, billing, and wait-time estimation.

## Overview

Instead of standing in a long physical line, students place orders through a terminal. Every order is tracked from placement to pickup using a First-In-First-Out (FIFO) discipline, ensuring fairness: whoever orders first is served first. Each order receives a unique token number and a live status (Waiting → Preparing → Ready → Completed).

Core estimation formula:

```
Wait Time = Orders Ahead x Average Service Time
```

Example: if 3 orders are ahead and each takes an average of 2 minutes, the estimated wait is 6 minutes.

## System Modules

The system is organised into four core modules, plus optional quality-of-life functions.

### A. Student Module

| # | Function | Description |
|---|----------|-------------|
| 1 | Student Login / ID Verification | Enter student ID or smart-card number, validate it, and optionally display student details |
| 2 | View Digital Menu | Display food items, prices, and availability, grouped by category (snacks, meals, drinks) |
| 3 | Place an Order | Select one or more items, enter quantities, calculate the total bill, and check stock |
| 4 | Receive Token | Generate a unique token number and display the estimated waiting time |

### B. Order Queue Module

| # | Function | Description |
|---|----------|-------------|
| 5 | Add Order (Enqueue) | Add a new order to the rear of the queue using `std::queue<Order>` or a custom linked list |
| 6 | View Pending Orders | Display all waiting orders with token numbers, student ID, and ordered items |
| 7 | Process Next Order (Dequeue) | Retrieve the front order and remove it from the pending queue when processing begins |
| 8 | Complete Order | Mark an order as completed and display its token number for collection |

### C. Inventory Management Module

| # | Function | Description |
|---|----------|-------------|
| 9 | Add New Food Item | Add a new menu item with its price and quantity |
| 10 | Update Stock | Increase or decrease the available quantity |
| 11 | Check Availability | Verify that enough stock exists before accepting an order |
| 12 | Automatic Stock Reduction | Reduce stock immediately after an order is confirmed |
| 13 | Out-of-Stock Notification | Mark an item unavailable when its stock reaches zero |

### D. Billing and Waiting Time Module

| # | Function | Description |
|---|----------|-------------|
| 14 | Calculate Bill | `Total Bill = Quantity x Price` (e.g., 2 burgers at ₹60 each = ₹120) |
| 15 | Estimate Waiting Time | `Wait Time = Orders Ahead x Average Service Time` |
| 16 | Display Order Status | Waiting, Preparing, Ready, Completed |

### Optional Functions (Quality of Life)

| # | Function | Description | Suggested DSA |
|---|----------|-------------|---------------|
| 17 | Search Food Item | Search by food name or ID | Hash map for fast lookup |
| 18 | Order History | Store completed orders and let students view past purchases | Vector or linked list |
| 19 | Most Popular Food | Count how often each item is ordered and display the best seller | Hash map or array of counters |
| 20 | Low-Stock Alert | Warn the admin when an item has fewer than 5 units left | Simple conditional check |

## Features

- **Student ID verification** — orders are tied to a validated student ID or smart-card number
- **Digital menu** — backed by a singly linked list, allowing items to be added without resizing arrays; items are grouped by category
- **Order placement (enqueue)** — supports multiple items with quantities, validates stock, deducts inventory, calculates the bill, and issues a unique token
- **Order processing (dequeue)** — kitchen staff serve orders strictly in arrival order
- **Order status tracking** — Waiting, Preparing, Ready, Completed
- **Live inventory management** — stock is decremented instantly and marked unavailable at zero
- **Estimated wait time** — calculated dynamically from the number of orders ahead
- **Queue status view** — shows all pending orders and which order is next
- **Admin tools** — add items, update stock, and low-stock warnings

## DSA Concepts Demonstrated

| Concept | Where It's Used | Purpose |
|---------|-----------------|---------|
| Linked List | `MenuItem` chain (`menuHead`) | Dynamic menu items without array resizing |
| Queue (STL `std::queue`) | `orderQueue` | Strict FIFO order processing |
| Linear Search | `findItem()` | Looks up a menu item by ID — O(n) |
| Hash Map (optional) | Item search, popularity counters | O(1) average lookup and counting |
| Vector / Linked List (optional) | Order history | Stores completed orders |
| Encapsulation | `CanteenSystem` class | Hides internal structures, exposing only safe operations |
| Struct Modeling | `MenuItem`, `Order` | Represents menu entries and orders as nodes |

> **Optimization note (for a project report):** `findItem()` currently performs a linear scan of the linked list. Replacing this with a hash map (`unordered_map<int, MenuItem*>`) would reduce lookup time from O(n) to O(1), which is significant during high-volume periods with many transactions.

## Possible Extensions

- **Circular Queue** — for fixed-size buffer memory efficiency
- **Priority Queue** — to support pre-orders or priority customers
- **Binary Search Tree (BST)** — alternative to linear search for O(log n) menu lookups
- **Status Flags (enum)** — `WAITING`, `PREPARING`, `READY`, `COMPLETED` for detailed order tracking

## Project Structure

```
CanteenQueueOptimizer/
├── canteen.cpp        Full source code (single file)
└── README.md          This file
```

## Key Components in Code

- **`MenuItem` struct** — a node in the menu linked list (id, name, category, price, stock, next)
- **`Order` struct** — represents a single order in the queue (token, studentId, itemIds, quantities, totalAmount, status)
- **`CanteenSystem` class** — encapsulates all logic:
  - `verifyStudent()` — validates a student ID
  - `addMenuItem()` — linked list insertion
  - `updateStock()` — increases or decreases an item's quantity
  - `displayMenu()` — linked list traversal
  - `placeOrder()` — enqueue operation with inventory validation and billing
  - `processNextOrder()` — dequeue operation
  - `completeOrder()` — marks an order Ready/Completed
  - `viewQueueStatus()` — lists pending orders and the front of the queue

## Requirements

- A C++ compiler supporting C++11 or later (e.g., g++, clang++, MSVC)
- No external libraries — only the C++ Standard Library (`<iostream>`, `<queue>`, `<vector>`, `<string>`, `<unordered_map>`, `<iomanip>`)

## Build and Run

**Linux / macOS (g++)**

```bash
g++ -std=c++11 -o canteen canteen.cpp
./canteen
```

**Windows (g++ / MinGW)**

```bash
g++ -std=c++11 -o canteen.exe canteen.cpp
canteen.exe
```

**Using an IDE**

Open `canteen.cpp` in any C++ IDE (Code::Blocks, CLion, Visual Studio) and build/run the project normally.

## Usage

On launch, the application displays a menu-driven interface:

```
=== SMART CANTEEN OPTIMIZER ===
1. Display Menu
2. Place New Order
3. Process Next Order (Kitchen)
4. View Queue Status
5. Exit
Enter Choice:
```

| Option | Action |
|--------|--------|
| 1 | View all menu items with ID, name, price, and stock |
| 2 | Place an order — enter your student ID, then item IDs and quantities |
| 3 | (Kitchen-side) Serve the next order in the queue and mark it ready |
| 4 | View pending orders, their tokens, and which one is next |
| 5 | Exit the program |

### Example Flow

1. Choose option 1 to view the menu and note item IDs (e.g., Burger = 1, Pizza = 2).
2. Choose option 2, enter your student ID, then the item IDs and quantities (e.g., 2 Burgers and 1 Coke).
3. The system checks stock, deducts it, calculates the bill, issues a token, and displays the estimated wait time.
4. Kitchen staff repeatedly choose option 3 to process orders in the exact order they were placed.
5. When an order is ready, its token number is displayed for collection.

## Future Enhancements

- Replace linear search with a hash map for O(1) menu and inventory lookups
- Add a priority queue for pre-orders or special-needs customers
- Persist orders, menu, and order history to a file or database between sessions
- Build a GUI or web front-end on top of the existing `CanteenSystem` logic
- Add order cancellation and refund handling
- Introduce multi-threading to simulate multiple kitchen counters processing the queue in parallel
