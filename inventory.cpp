
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
using namespace std;

void addNewFoodItem() {
    ofstream file("inventory.txt", ios::app);

    if (!file.is_open()) {
        cout << "Error opening inventory.txt!\n";
        return;
    }

    string id, name;
    int quantity, wtime;
    double price; 

    cout << "Enter ID, Name, Price, Quantity, Wtime: ";
    cin >> id >> name >> price >> quantity >> wtime;

    file << id << " " << name << " "
         << price << " " << quantity << " "
         << wtime << '\n';
    file.close();
    cout << "Food item added successfully!\n";
}

void updateStock(string id, int change) {
    ifstream file("inventory.txt");

    if (!file.is_open()) {
        cout << "Error opening inventory.txt!\n";
        return;
    }
    ofstream temp("temp.txt");
    if (!temp.is_open()) {
        cout << "Error creating temp.txt!\n";
        file.close();
        return;
    }
    string fid, name;
    int quantity, wtime;
    double price;
    bool found = false;

    while (file >> fid >> name >> price >> quantity >> wtime) {
        if (fid == id) {
            quantity += change;
            if (quantity < 0)
                quantity = 0;

            found = true;
        }
        temp << fid << " " << name << " "
             << price << " " << quantity << " "
             << wtime << '\n';
    }

    file.close();
    temp.close();

    if (!temp) {
        cout << "Error writing temporary file!\n";
        remove("temp.txt");
        return;
    }
    if (!found) {
        remove("temp.txt");
        cout << "Food ID not found!\n";
        return;
    }
    if (rename("temp.txt", "inventory.txt") != 0) {
        cout << "Error replacing inventory.txt!\n";
        return;
    }
    cout << "Stock updated successfully!\n";
}

bool checkAvailability(string id, int required) {
    ifstream file("inventory.txt");
    if (!file.is_open()) {
        cout << "Error opening inventory.txt!\n";
        return false;
    }
    string fid, name;
    int quantity, wtime;
    double price;

    while (file >> fid >> name >> price >> quantity >> wtime) {
        if (fid == id) {
            file.close();
            return quantity >= required;
        }
    }

    file.close();
    return false;
}

int main() {
    int choice, quantity, change;
    string id;

    do {
        cout << "\n     INVENTORY MANAGEMENT\n";
        cout << "1. Add New Food Item\n";
        cout << "2. Update Stock\n";
        cout << "3. Check Availability\n";
        cout << "0. Exit\n";
        cout << "Enter choice: ";

        if (!(cin >> choice)) {
            cout << "Invalid input!\n";
            break;
        }
        switch (choice) {
            case 1:
                addNewFoodItem();
                break;

            case 2:
                cout << "Enter Food ID and Stock Change: ";
                cin >> id >> change;
                updateStock(id, change);
                break;

            case 3:
                cout << "Enter Food ID and Required Quantity: ";
                cin >> id >> quantity;

                if (quantity <= 0)
                    cout << "Quantity must be greater than zero!\n";
                else if (checkAvailability(id, quantity))
                    cout << "Food is available!\n";
                else
                    cout << "Food is unavailable or insufficient stock!\n";
                break;

            case 0:
                cout << "Exiting program...\n";
                break;

            default:
                cout << "Invalid choice!\n";
        }

    } while (choice != 0);

    return 0;
}
