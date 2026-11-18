#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdio>
#include <cctype>
#include <limits>
using namespace std;


string inventoryPath = "inventory.txt";

string invUpper(string s) {
    for (size_t i = 0; i < s.size(); i++)
        s[i] = (char)toupper((unsigned char)s[i]);
    return s;
}

bool invFileExists(const string& path) {
    ifstream f(path);
    return f.is_open();
}


string invDirOf(const string& path) {
    size_t pos = path.find_last_of("/\\");
    return pos == string::npos ? "" : path.substr(0, pos + 1);
}


string dataPath(const string& name) {
    return invDirOf(inventoryPath) + name;
}




const char* DEFAULT_INVENTORY =
    "M01 VegThali 90 25 8\n"
    "M02 RajmaChawal 70 25 6\n"
    "M03 CholeBhature 80 25 10\n"
    "M04 MasalaDosa 60 25 10\n"
    "M05 VegFriedRice 60 25 8\n"
    "M06 PaneerButterMasala 110 25 12\n"
    "M07 DalRice 50 25 5\n"
    "M08 VegBiryani 80 25 8\n"
    "M09 AlooParatha 45 25 8\n"
    "M10 PavBhaji 60 25 7\n"
    "M11 VegNoodles 60 25 8\n"
    "M12 CholeRice 65 25 6\n"
    "M13 PaneerWrap 80 25 8\n"
    "M14 IdliSambar 40 25 5\n"
    "M15 VegBurgerMeal 90 25 10\n"
    "B01 MasalaTea 15 40 3\n"
    "B02 BlackCoffee 20 40 2\n"
    "B03 ColdCoffee 40 40 4\n"
    "B04 Lemonade 25 40 2\n"
    "B05 MangoShake 45 40 4\n"
    "B06 BananaShake 40 40 4\n"
    "B07 OrangeJuice 35 40 3\n"
    "B08 SweetLassi 30 40 2\n"
    "B09 SaltedLassi 30 40 2\n"
    "B10 HotChocolate 40 40 4\n"
    "B11 IcedTea 30 40 2\n"
    "B12 Buttermilk 20 40 1\n"
    "B13 StrawberryShake 45 40 4\n"
    "B14 CoconutWater 35 40 1\n"
    "B15 MineralWater 20 40 1\n"
    "S01 Samosa 25 30 3\n"
    "S02 VegMomos 50 30 8\n"
    "S03 FriedMomos 60 30 8\n"
    "S04 FrenchFries 50 30 6\n"
    "S05 VegSandwich 40 30 4\n"
    "S06 PaneerPakora 60 30 7\n"
    "S07 OnionPakora 35 30 6\n"
    "S08 SpringRolls 50 30 7\n"
    "S09 AlooTikki 35 30 5\n"
    "S10 BreadPakora 30 30 5\n"
    "S11 VegCutlet 35 30 5\n"
    "S12 CheeseBalls 60 30 7\n"
    "S13 GarlicBread 45 30 5\n"
    "S14 ChilliPotato 60 30 7\n"
    "S15 Dhokla 35 30 2\n";

void ensureInventoryFile() {
    if (invFileExists(inventoryPath)) return;
    ofstream out(inventoryPath.c_str());
    if (out.is_open()) {
        out << DEFAULT_INVENTORY;
        out.close();
        cout << "NOTE: inventory.txt was not found, so a default one was created at: "
             << inventoryPath << "\n";
    }
}


bool parseInvLine(const string& line, string& id, string& name,
                  double& price, int& quantity, int& wtime) {
    istringstream in(line);
    vector<string> t;
    string w;
    while (in >> w) t.push_back(w);
    if (t.size() < 5) return false;
    size_t n = t.size();
    try {
        size_t pos = 0;
        price = stod(t[n - 3], &pos);
        if (pos != t[n - 3].size()) return false;
        quantity = stoi(t[n - 2], &pos);
        if (pos != t[n - 2].size()) return false;
        wtime = stoi(t[n - 1], &pos);
        if (pos != t[n - 1].size()) return false;
    } catch (...) {
        return false;
    }
    id = t[0];
    name = t[1];
    for (size_t i = 2; i + 3 < n; i++) name += " " + t[i];
    return true;
}


bool saveInventoryLines(const vector<string>& lines) {
    string tmp = inventoryPath + ".tmp";
    {
        ofstream t(tmp.c_str());
        if (!t.is_open()) {
            cout << "Error creating temporary file!\n";
            return false;
        }
        for (size_t i = 0; i < lines.size(); i++) t << lines[i] << '\n';
        t.close();
        if (!t) {
            cout << "Error writing temporary file!\n";
            remove(tmp.c_str());
            return false;
        }
    }
    remove(inventoryPath.c_str());
    if (rename(tmp.c_str(), inventoryPath.c_str()) != 0) {
        ofstream o(inventoryPath.c_str());     
        if (!o.is_open()) {
            cout << "Error replacing inventory file! Your data is saved in " << tmp << "\n";
            return false;
        }
        for (size_t i = 0; i < lines.size(); i++) o << lines[i] << '\n';
        o.close();
        remove(tmp.c_str());
    }
    return true;
}


bool updateStock(string id, int change, bool verbose = true) {
    id = invUpper(id);
    ifstream file(inventoryPath.c_str());
    if (!file.is_open()) {
        cout << "Error opening inventory file!\n";
        return false;
    }
    vector<string> lines;
    string line;
    bool found = false, enough = true;

    while (getline(file, line)) {
        if (!line.empty() && line[line.size() - 1] == '\r') line.erase(line.size() - 1);
        string fid, name;
        double price;
        int quantity, wtime;
        if (!found && parseInvLine(line, fid, name, price, quantity, wtime)
            && invUpper(fid) == id) {
            found = true;
            if (quantity + change < 0) {
                enough = false;
            } else {
                quantity += change;
                ostringstream o;
                o << fid << " " << name << " " << price << " " << quantity << " " << wtime;
                line = o.str();
            }
        }
        lines.push_back(line);
    }
    file.close();

    if (!found) {
        cout << "Food ID not found!\n";
        return false;
    }
    if (!enough) {
        cout << "Not enough stock to remove that quantity!\n";
        return false;
    }
    if (!saveInventoryLines(lines)) return false;
    if (verbose)
        cout << "Stock updated successfully!\n";
    return true;
}


int getStock(string id) {
    id = invUpper(id);
    ifstream file(inventoryPath.c_str());
    if (!file.is_open()) return -1;
    string line;
    while (getline(file, line)) {
        string fid, name;
        double price;
        int quantity, wtime;
        if (parseInvLine(line, fid, name, price, quantity, wtime) && invUpper(fid) == id)
            return quantity;
    }
    return -1;
}

bool checkAvailability(string id, int required) {
    int stock = getStock(id);
    return stock >= 0 && stock >= required;
}

int main(int argc, char* argv[]) {
    locateInventory(argc > 0 ? argv[0] : "");
    ensureInventoryFile();
    int choice, quantity, change;
    string id;

    do {
        cout << "\n     INVENTORY MANAGEMENT\n";
        cout << "1. Update Stock\n";
        cout << "2. Check Availability\n";
        cout << "0. Exit\n";
        cout << "Enter choice: ";

        if (!(cin >> choice)) {
            cout << "Invalid input!\n";
            break;
        }
        switch (choice) {
            case 1:
                cout << "Enter Food ID and Stock Change: ";
                if (!(cin >> id >> change)) {
                    cin.clear();
                    cin.ignore(1000, '\n');
                    cout << "Invalid input!\n";
                    break;
                }
                updateStock(id, change);
                break;
            case 2:
                cout << "Enter Food ID and Required Quantity: ";
                if (!(cin >> id >> quantity)) {
                    cin.clear();
                    cin.ignore(1000, '\n');
                    cout << "Invalid input!\n";
                    break;
                }
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
