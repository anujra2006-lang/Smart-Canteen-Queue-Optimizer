# Smart Canteen Queue Optimizer

A C++ console application that replaces the traditional first-come-first-served physical canteen line with a structured digital FIFO queue. It applies core Data Structures and Algorithms (DSA) — queues, linked lists, and search operations — to manage order flow, live inventory, and wait-time estimation.

---

## Overview

Instead of standing in a long physical line, students place orders through a terminal. Every order is tracked from placement to pickup using a First-In-First-Out (FIFO) discipline, ensuring fairness: whoever orders first is served first.

Core estimation formula:

```
Wait Time = Queue Size x Average Service Time
```

---

## Features

- **Digital Menu** — backed by a singly linked list, allowing items to be added without resizing arrays
- **Order Placement (Enqueue)** — validates stock, deducts inventory, and assigns a unique order ID
- **Order Processing (Dequeue)** — kitchen staff serve orders strictly in arrival order
- **Live Inventory Management** — stock is decremented instantly and marked unavailable at zero
- **Estimated Wait Time** — calculated dynamically from the current queue length
- **Queue Status View** — displays the number of orders waiting and which order is next

---

## DSA Concepts Demonstrated

| Concept | Where It's Used | Purpose |
|---|---|---|
| Linked List | `MenuItem` chain (`menuHead`) | Dynamic menu items without array resizing |
| Queue (STL `std::queue`) | `orderQueue` | Strict FIFO order processing |
| Linear Search | `findItem()` | Looks up a menu item by ID — O(n) |
| Encapsulation | `CanteenSystem` class | Hides internal structures, exposing only safe operations |
| Struct Modeling | `MenuItem`, `Order` | Represents menu entries and orders as nodes |

**Optimization note (for a project report):** `findItem()` currently performs a linear scan of the linked list. Replacing this with a hash map (`unordered_map<int, MenuItem*>`) would reduce lookup time from O(n) to O(1), which is significant during high-volume periods with many transactions.

### Possible Extensions

- Circular Queue — for fixed-size buffer memory efficiency
- Priority Queue — to support pre-orders or priority customers
- Binary Search Tree (BST) — alternative to linear search for O(log n) menu lookups
- Status Flags (enum) — `PENDING`, `PREPARING`, `READY` for more detailed order tracking

---

## Project Structure

```
CanteenQueueOptimizer/
├── canteen.cpp        Full source code (single file)
└── README.md          This file
```

### Key Components in Code

- **`MenuItem` struct** — a node in the menu linked list (`id`, `name`, `price`, `stock`, `next`)
- **`Order` struct** — represents a single order in the queue (`orderId`, `customerName`, `itemIds`, `totalAmount`, `status`)
- **`CanteenSystem` class** — encapsulates all logic:
  - `addMenuItem()` — linked list insertion
  - `displayMenu()` — linked list traversal
  - `placeOrder()` — enqueue operation with inventory validation
  - `processNextOrder()` — dequeue operation
  - `viewQueueStatus()` — inspects the front of the queue

---

## Requirements

- A C++ compiler supporting C++11 or later (e.g., g++, clang++, MSVC)
- No external libraries — only the C++ Standard Library (`<iostream>`, `<queue>`, `<vector>`, `<string>`, `<iomanip>`)

---

## Build and Run

### Linux / macOS (g++)

```bash
g++ -std=c++11 -o canteen canteen.cpp
./canteen
```

### Windows (g++ / MinGW)

```bash
g++ -std=c++11 -o canteen.exe canteen.cpp
canteen.exe
```

### Using an IDE

Open `canteen.cpp` in any C++ IDE (Code::Blocks, CLion, Visual Studio) and build/run the project normally.

---

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
|---|---|
| 1 | View all menu items with ID, name, price, and stock |
| 2 | Place an order — enter your name, number of items, then each item ID |
| 3 | (Kitchen-side) Serve the next order in the queue and mark it ready |
| 4 | View how many orders are waiting and which one is next |
| 5 | Exit the program |

### Example Flow

1. Choose option 1 to view the menu and note item IDs (e.g., Burger = 1, Pizza = 2).
2. Choose option 2, enter a name, then enter the item IDs to order (e.g., `1 3` for a Burger and a Coke).
3. The system deducts stock, assigns an order ID, and displays an estimated wait time.
4. Kitchen staff repeatedly choose option 3 to process orders in the exact order they were placed.

---

## Future Enhancements

- Replace linear search with a hash map for O(1) menu and inventory lookups
- Add a priority queue for pre-orders or special-needs customers
- Persist orders and menu data to a file or database between sessions
- Build a GUI or web front-end on top of the existing `CanteenSystem` logic
- Add order cancellation and refund handling
- Introduce multi-threading to simulate multiple kitchen counters processing the queue in parallel

---
