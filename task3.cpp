#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <ctime>
#include <limits>
using namespace std;

const string INVENTORY_FILE = "inventory.txt";
const string RECEIPT_FILE   = "receipts.txt";
const int    LOW_STOCK_THRESHOLD = 5;

// ============================================================
// Base class: Product
// Demonstrates: Classes & Objects, base for Inheritance/Polymorphism
// ============================================================
class Product {
protected:
    int    id;
    string name;
    int    quantity;
    double price;

public:
    Product() : id(0), name(""), quantity(0), price(0.0) {}

    Product(int id, string name, int quantity, double price)
        : id(id), name(name), quantity(quantity), price(price) {}

    virtual ~Product() {}

    // ---- Getters ----
    int getId() const { return id; }
    string getName() const { return name; }
    int getQuantity() const { return quantity; }
    double getPrice() const { return price; }

    // ---- Setters ----
    void setName(const string &n) { name = n; }
    void setQuantity(int q) { quantity = q; }
    void setPrice(double p) { price = p; }

    void reduceStock(int qty) { quantity -= qty; }
    void increaseStock(int qty) { quantity += qty; }

    double getTotalValue() const { return quantity * price; }

    // Polymorphic behaviour: each derived category can override this
    virtual string getCategory() const { return "General"; }

    // Extra info that subclasses may override (e.g. warranty, expiry)
    virtual string getExtraInfo() const { return "-"; }

    // Used when writing to file: subclasses append their own extra field
    virtual string serializeExtra() const { return ""; }

    virtual void display() const {
        cout << left
             << setw(6)  << id
             << setw(20) << name
             << setw(14) << getCategory()
             << setw(10) << quantity
             << setw(10) << fixed << setprecision(2) << price
             << setw(12) << getTotalValue()
             << setw(15) << getExtraInfo()
             << (quantity <= LOW_STOCK_THRESHOLD ? "  << LOW STOCK" : "")
             << "\n";
    }
};

// ============================================================
// Derived class: Electronics
// Demonstrates: Inheritance + Polymorphism (overrides category/extra info)
// ============================================================
class Electronics : public Product {
private:
    int warrantyMonths;

public:
    Electronics() : Product(), warrantyMonths(0) {}

    Electronics(int id, string name, int quantity, double price, int warrantyMonths)
        : Product(id, name, quantity, price), warrantyMonths(warrantyMonths) {}

    string getCategory() const override { return "Electronics"; }

    string getExtraInfo() const override {
        return to_string(warrantyMonths) + " mo warranty";
    }

    string serializeExtra() const override {
        return to_string(warrantyMonths);
    }

    int getWarrantyMonths() const { return warrantyMonths; }
};

// ============================================================
// Derived class: Grocery
// Demonstrates: Inheritance + Polymorphism (overrides category/extra info)
// ============================================================
class Grocery : public Product {
private:
    string expiryDate; // format: DD-MM-YYYY

public:
    Grocery() : Product(), expiryDate("N/A") {}

    Grocery(int id, string name, int quantity, double price, string expiryDate)
        : Product(id, name, quantity, price), expiryDate(expiryDate) {}

    string getCategory() const override { return "Grocery"; }

    string getExtraInfo() const override {
        return "Exp: " + expiryDate;
    }

    string serializeExtra() const override {
        return expiryDate;
    }

    string getExpiryDate() const { return expiryDate; }
};

// ============================================================
// Inventory class
// Manages the collection of products (STL vector of base-class pointers)
// Handles Add/Update/Delete/Search, Reports, File Handling, Sales/Receipts
// ============================================================
class Inventory {
private:
    vector<Product*> products;
    int nextId;

    Product* findById(int id) {
        for (Product* p : products)
            if (p->getId() == id) return p;
        return nullptr;
    }

    static string currentTimestamp() {
        time_t now = time(nullptr);
        char buf[64];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
        return string(buf);
    }

public:
    Inventory() : nextId(1) {}

    ~Inventory() {
        for (Product* p : products) delete p;
    }

    // ---------------- Add Product ----------------
    void addElectronics(string name, int quantity, double price, int warranty) {
        products.push_back(new Electronics(nextId++, name, quantity, price, warranty));
        cout << "Electronics product added successfully.\n";
    }

    void addGrocery(string name, int quantity, double price, string expiry) {
        products.push_back(new Grocery(nextId++, name, quantity, price, expiry));
        cout << "Grocery product added successfully.\n";
    }

    void addGeneral(string name, int quantity, double price) {
        products.push_back(new Product(nextId++, name, quantity, price));
        cout << "Product added successfully.\n";
    }

    // ---------------- Update Product ----------------
    void updateProduct(int id) {
        Product* p = findById(id);
        if (!p) {
            cout << "Product with ID " << id << " not found.\n";
            return;
        }

        string name;
        int qty;
        double price;

        cout << "Updating: " << p->getName() << "\n";
        cout << "Enter new name (current: " << p->getName() << "): ";
        cin.ignore();
        getline(cin, name);
        cout << "Enter new quantity (current: " << p->getQuantity() << "): ";
        cin >> qty;
        cout << "Enter new price (current: " << p->getPrice() << "): ";
        cin >> price;

        if (!name.empty()) p->setName(name);
        p->setQuantity(qty);
        p->setPrice(price);

        cout << "Product updated successfully.\n";
    }

    // ---------------- Delete Product ----------------
    void deleteProduct(int id) {
        for (size_t i = 0; i < products.size(); i++) {
            if (products[i]->getId() == id) {
                delete products[i];
                products.erase(products.begin() + i);
                cout << "Product deleted successfully.\n";
                return;
            }
        }
        cout << "Product with ID " << id << " not found.\n";
    }

    // ---------------- Search Product ----------------
    void searchById(int id) {
        Product* p = findById(id);
        if (!p) {
            cout << "Product with ID " << id << " not found.\n";
            return;
        }
        printHeader();
        p->display();
    }

    void searchByName(const string &name) {
        bool found = false;
        printHeader();
        for (Product* p : products) {
            if (p->getName().find(name) != string::npos) {
                p->display();
                found = true;
            }
        }
        if (!found) cout << "No product found matching \"" << name << "\".\n";
    }

    // ---------------- Display ----------------
    void printHeader() const {
        cout << left
             << setw(6)  << "ID"
             << setw(20) << "Name"
             << setw(14) << "Category"
             << setw(10) << "Qty"
             << setw(10) << "Price"
             << setw(12) << "Value"
             << setw(15) << "Extra Info"
             << "\n";
        cout << string(87, '-') << "\n";
    }

    void listAll() const {
        if (products.empty()) {
            cout << "Inventory is empty.\n";
            return;
        }
        printHeader();
        for (Product* p : products) p->display();
    }

    // ---------------- Reports ----------------
    void generateReport() const {
        int totalProducts = products.size();
        int availableStock = 0;
        double inventoryValue = 0.0;

        for (Product* p : products) {
            availableStock += p->getQuantity();
            inventoryValue += p->getTotalValue();
        }

        cout << "\n================ INVENTORY REPORT ================\n";
        cout << "Total Products     : " << totalProducts << "\n";
        cout << "Available Stock     : " << availableStock << " units\n";
        cout << "Total Inventory Value: Rs. " << fixed << setprecision(2) << inventoryValue << "\n";
        cout << "----------------------------------------------------\n";
        cout << "Low Stock Alerts (<= " << LOW_STOCK_THRESHOLD << " units):\n";

        bool anyLow = false;
        for (Product* p : products) {
            if (p->getQuantity() <= LOW_STOCK_THRESHOLD) {
                cout << "  - " << p->getName() << " (ID: " << p->getId()
                     << ") | Qty left: " << p->getQuantity() << "\n";
                anyLow = true;
            }
        }
        if (!anyLow) cout << "  None. All stock levels are healthy.\n";
        cout << "====================================================\n\n";
    }

    // ---------------- Bonus: Sell product + generate receipt ----------------
    void sellProduct(int id, int qtySold) {
        Product* p = findById(id);
        if (!p) {
            cout << "Product with ID " << id << " not found.\n";
            return;
        }
        if (qtySold <= 0 || qtySold > p->getQuantity()) {
            cout << "Invalid quantity. Available stock: " << p->getQuantity() << "\n";
            return;
        }

        double total = qtySold * p->getPrice();
        p->reduceStock(qtySold);

        // Print receipt to console
        cout << "\n================ SALES RECEIPT ================\n";
        cout << "Date/Time : " << currentTimestamp() << "\n";
        cout << "Product   : " << p->getName() << " (" << p->getCategory() << ")\n";
        cout << "Qty Sold  : " << qtySold << "\n";
        cout << "Unit Price: Rs. " << fixed << setprecision(2) << p->getPrice() << "\n";
        cout << "Total     : Rs. " << total << "\n";
        cout << "Remaining Stock: " << p->getQuantity() << "\n";
        cout << "=================================================\n\n";

        // Append receipt to file
        ofstream out(RECEIPT_FILE, ios::app);
        if (out.is_open()) {
            out << "----- RECEIPT -----\n";
            out << "Date/Time : " << currentTimestamp() << "\n";
            out << "Product   : " << p->getName() << " (" << p->getCategory() << ")\n";
            out << "Qty Sold  : " << qtySold << "\n";
            out << "Unit Price: " << fixed << setprecision(2) << p->getPrice() << "\n";
            out << "Total     : " << total << "\n";
            out << "Remaining Stock: " << p->getQuantity() << "\n\n";
            out.close();
        }

        // Persist updated stock immediately
        saveToFile();
        cout << "Sale recorded, receipt saved, and inventory updated.\n";
    }

    // ---------------- File Handling ----------------
    // File format per line: type|id|name|quantity|price|extra
    void saveToFile() const {
        ofstream out(INVENTORY_FILE);
        if (!out.is_open()) {
            cout << "Error: could not open file for saving.\n";
            return;
        }
        for (Product* p : products) {
            out << p->getCategory() << "|"
                << p->getId() << "|"
                << p->getName() << "|"
                << p->getQuantity() << "|"
                << p->getPrice() << "|"
                << p->serializeExtra() << "\n";
        }
        out.close();
    }

    void loadFromFile() {
        ifstream in(INVENTORY_FILE);
        if (!in.is_open()) return; // no existing file yet, that's fine

        for (Product* p : products) delete p;
        products.clear();

        string line;
        int maxId = 0;
        while (getline(in, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string category, idStr, name, qtyStr, priceStr, extra;

            getline(ss, category, '|');
            getline(ss, idStr, '|');
            getline(ss, name, '|');
            getline(ss, qtyStr, '|');
            getline(ss, priceStr, '|');
            getline(ss, extra, '|');

            int id = stoi(idStr);
            int qty = stoi(qtyStr);
            double price = stod(priceStr);
            maxId = max(maxId, id);

            if (category == "Electronics") {
                int warranty = extra.empty() ? 0 : stoi(extra);
                products.push_back(new Electronics(id, name, qty, price, warranty));
            } else if (category == "Grocery") {
                products.push_back(new Grocery(id, name, qty, price, extra.empty() ? "N/A" : extra));
            } else {
                products.push_back(new Product(id, name, qty, price));
            }
        }
        in.close();
        nextId = maxId + 1;
        cout << "Inventory loaded from file (" << products.size() << " records).\n";
    }
};

// ============================================================
// Helper: clear bad cin state
// ============================================================
void clearInput() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

// ============================================================
// Main Menu
// ============================================================
void showMenu() {
    cout << "\n================ INVENTORY MANAGEMENT SYSTEM ================\n";
    cout << "1.  Add Product (General)\n";
    cout << "2.  Add Product (Electronics)\n";
    cout << "3.  Add Product (Grocery)\n";
    cout << "4.  Update Product\n";
    cout << "5.  Delete Product\n";
    cout << "6.  Search Product by ID\n";
    cout << "7.  Search Product by Name\n";
    cout << "8.  Display All Products\n";
    cout << "9.  Generate Inventory Report\n";
    cout << "10. Sell Product (Generate Receipt)\n";
    cout << "11. Save Inventory to File\n";
    cout << "0.  Exit\n";
    cout << "===============================================================\n";
    cout << "Enter your choice: ";
}

int main() {
    Inventory inventory;
    inventory.loadFromFile();

    int choice;
    do {
        showMenu();
        if (!(cin >> choice)) {
            clearInput();
            cout << "Invalid input. Please enter a number.\n";
            continue;
        }

        switch (choice) {
            case 1: {
                string name; int qty; double price;
                cin.ignore();
                cout << "Enter product name: ";
                getline(cin, name);
                cout << "Enter quantity: ";
                cin >> qty;
                cout << "Enter price: ";
                cin >> price;
                inventory.addGeneral(name, qty, price);
                break;
            }
            case 2: {
                string name; int qty, warranty; double price;
                cin.ignore();
                cout << "Enter product name: ";
                getline(cin, name);
                cout << "Enter quantity: ";
                cin >> qty;
                cout << "Enter price: ";
                cin >> price;
                cout << "Enter warranty period (months): ";
                cin >> warranty;
                inventory.addElectronics(name, qty, price, warranty);
                break;
            }
            case 3: {
                string name, expiry; int qty; double price;
                cin.ignore();
                cout << "Enter product name: ";
                getline(cin, name);
                cout << "Enter quantity: ";
                cin >> qty;
                cout << "Enter price: ";
                cin >> price;
                cin.ignore();
                cout << "Enter expiry date (DD-MM-YYYY): ";
                getline(cin, expiry);
                inventory.addGrocery(name, qty, price, expiry);
                break;
            }
            case 4: {
                int id;
                cout << "Enter product ID to update: ";
                cin >> id;
                inventory.updateProduct(id);
                break;
            }
            case 5: {
                int id;
                cout << "Enter product ID to delete: ";
                cin >> id;
                inventory.deleteProduct(id);
                break;
            }
            case 6: {
                int id;
                cout << "Enter product ID to search: ";
                cin >> id;
                inventory.searchById(id);
                break;
            }
            case 7: {
                string name;
                cin.ignore();
                cout << "Enter product name (or part of it): ";
                getline(cin, name);
                inventory.searchByName(name);
                break;
            }
            case 8:
                inventory.listAll();
                break;
            case 9:
                inventory.generateReport();
                break;
            case 10: {
                int id, qty;
                cout << "Enter product ID to sell: ";
                cin >> id;
                cout << "Enter quantity sold: ";
                cin >> qty;
                inventory.sellProduct(id, qty);
                break;
            }
            case 11:
                inventory.saveToFile();
                cout << "Inventory saved to file successfully.\n";
                break;
            case 0:
                inventory.saveToFile();
                cout << "Inventory saved. Exiting program. Goodbye!\n";
                break;
            default:
                cout << "Invalid choice. Please try again.\n";
        }

    } while (choice != 0);

    return 0;
}
