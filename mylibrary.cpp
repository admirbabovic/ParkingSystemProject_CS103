#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include <vector>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <ctime>
#include <cstdlib>
#include <unordered_map>
#include <random>
#include "mylibrary.h"

using namespace std;

// Function to convert byte array to hex string
string bytesToHex(const vector<unsigned char>& bytes) {
    stringstream ss;
    for (unsigned char byte : bytes) {
        ss << setw(2) << setfill('0') << hex << static_cast<int>(byte);
    }
    return ss.str();
}

// Function to generate a random chars salt of specified length
string generateSalt(const int length = 13) {
    const string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!#$%&/()=?*,.-_@[]{}+";

    random_device rd; // Seed
    mt19937 generator(rd()); // Mersenne Twister RNG
    std::uniform_int_distribution<> dis(0, charset.size() - 1); // Distribution range

    string salt;
    for (int i = 0; i < length; ++i) {
        salt += charset[dis(generator)];
    }

    return salt;
}

// Function to hash password using SHA-256
string hashPasswordWithSalt(const string& password, const string& salt, const string& username) {
    vector<unsigned char> hash(SHA256_DIGEST_LENGTH);
    const string salted_password = salt + password + username;

    SHA256_CTX sha256_ctx;
    SHA256_Init(&sha256_ctx);
    SHA256_Update(&sha256_ctx, salted_password.c_str(), salted_password.size());
    SHA256_Final(hash.data(), &sha256_ctx);

    return bytesToHex(hash);
}

// Function to register a new user with optional pre-filled username
void registerUser(const string& prefilled_username) {
    string username, password;

    // Display the pre-filled username if provided
    if (!prefilled_username.empty()) {
        username = prefilled_username;

        cout << "Enter password: ";
        getline(cin, password);
    }

    else {
        cout << "Enter username: ";
        getline(cin, username);

        cout << "Enter password: ";
        getline(cin, password);
    }

    // Generate salt and hash the password
    string salt = generateSalt();
    string password_hash = hashPasswordWithSalt(password, salt, username);

    // Store the user info in credentials.dat
    ofstream file(FILE_PATH_DIR, ios::app);
    if (!file) {
        cerr << "Unable to open file!" << endl;
        exit(1);
    }

    // Save the user data (username, hashed password, and salt)
    file << username << " " << password_hash << " " << salt << endl;
    cout << "User registered successfully!" << endl;
    file.close();

    cout << "Proceed to login" << endl;
    loginUser();
}

// Function that check if username exist in credentials.dat file
bool checkUsername(const string& username) {
    ifstream file(FILE_PATH_DIR);
    if (!file) {
        return false; // Credentials.dat file doesn't exist
    }

    string stored_username, stored_password_hash, stored_salt;
    while (file >> stored_username >> stored_password_hash >> stored_salt) {
        if (stored_username == username) {
            return true; // Username found
        }
    }
    return false; // Username not found
}

// Function to check login credentials
void loginUser() {
    string username, password;

    cout << "Enter username: ";
    getline(cin, username);

    // Check if the username exists
    if (!checkUsername(username)) {
        string choice;
        do {
            cout << "Username not found! Would you like to register a new account? (y/n): " << endl;;
            getline(cin, choice);
        } while (!(choice == "Y" || choice == "y" || choice == "N" || choice == "n"));

        if (choice == "Y" || choice == "y") {
            string choice2;
            do {
                cout << "Would you like to use \"" << username << "\" as username? (y/n): ";
                getline(cin, choice2);
            } while (!(choice2 == "Y" || choice2 == "y" || choice2 == "N" || choice2 == "n"));

            if (choice2 == "Y" || choice2 == "y") {
                registerUser(username);
            }
            else {
                string newUsername;
                cout << "Enter new username that you would like to use: ";
                getline(cin, newUsername);
                registerUser(newUsername);
            }
        }
        else {
            cout << "Please try logging in again." << endl;
            loginUser();
        }
    }

    // If username exists, proceed to prompt for password
    cout << "Enter password: ";
    getline(cin, password);

    ifstream file(FILE_PATH_DIR);
    if (!file) {
        cerr << "Unable to open file!" << endl;
        exit(1);
    }

    string stored_username, stored_password_hash, stored_salt;
    while (file >> stored_username >> stored_password_hash >> stored_salt) {
        if (stored_username == username) {
            // Hash the entered password with the stored salt
            string entered_password_hash = hashPasswordWithSalt(password, stored_salt, username);
            if (entered_password_hash == stored_password_hash) {
                cout << "Logging in ..." << endl;
                cout << "Login successful!" << endl;
                if (username == "admin") {
                    file.close();  // Close credentials file
                    adminMode(); // Enter Admin mode
                }
                else {
                    file.close();  // Close credentials file
                    userMode(); // Enter User mode
                }
            }
            else {
                cout << "Incorrect password! Try Again!" << endl;
            }
            return;
        }
    }
}

struct ParkingSlots {
    bool isOccupied{};   // Indicates whether the slot is occupied
    string vehicleID;  // The ID of the vehicle parked in this slot
    time_t entryTime{};  // The time the vehicle entered the parking slot
};

vector<ParkingSlots> slots; // Vector to store parking slots
unordered_map<string, int> vehicleMap; // Maps vehicle ID to slot number
int totalSlots;            // Total number of parking slots
double hourlyRate;         // Hourly parking rate
double dailyRate = hourlyRate * 24 * 0.7 ;  // Daily parking rate
double dailyRevenue = 0.0; // Revenue generated for the current day
double weeklyRevenue = 0.0; // Revenue generated for the current week
int dailyOccupancy = 0;    // Number of parking slots occupied today
int weeklyOccupancy = 0;   // Number of parking slots occupied this week

void configureSlotsAndRates() {
    cout << "Enter the number of slots: ";
    cin >> totalSlots;
    cout << "Enter the hourly rate: ";
    cin >> hourlyRate;
    cout << "Enter the daily rate: ";
    cin >> dailyRate;

    // Resize the slots vector and initialize slot data
    slots.resize(totalSlots);
    for (int i = 0; i < totalSlots; ++i) {
        slots[i].isOccupied = false;  // Initially, all slots are unoccupied
        slots[i].vehicleID = "";      // No vehicle is parked initially
    }
    cout << "Configuration updated.\n";
}

// Function to generate daily and weekly revenue reports
void generateReports() {
    cout << "Daily Revenue: $" << fixed << setprecision(2) << dailyRevenue << endl;
    cout << "Daily Occupancy: " << dailyOccupancy << " / " << totalSlots << endl;
    cout << "Weekly Revenue: $" << fixed << setprecision(2) << weeklyRevenue << endl;
    cout << "Weekly Occupancy: " << weeklyOccupancy << endl;
}

// Admin mode function that provides options for configuration and reports
void adminMode() {
    int choice;
    while (true) {
        // Display admin menu
        cout << "Welcome to admin dashboard!\nYou can choose to:\n";
        cout << "1. Configure parking slots and rates\n";
        cout << "2. Generate daily/weekly reports\n";
        cout << "3. Exit admin dashboard\n"; // Option to exit Admin mode
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
        case 1:
            configureSlotsAndRates(); // Call function to configure slots and rates
            break;
        case 2:
            generateReports(); // Call function to generate reports
            break;
        case 3:
            cout << "Exiting to the main menu...\n";
            return; // Exit Admin mode and return to main menu
        default:
            cout << "Invalid choice.\n";
        }
    }
}

// Function to check availability of parking slots
void checkAvailability() {
    int availableSlots = 0;
    for (const auto& slot : slots) {
        if (!slot.isOccupied) {
            ++availableSlots;  // Count the number of available slots
        }
    }
    cout << "Available slots: " << availableSlots << " / " << totalSlots << endl;

    // Check if only one slot is left
    if (availableSlots == 1) {
        cout << "Hurry up, parking is almost full!!!\n"; // Display warning message if only one slot is left
    }
}

// Function to park a vehicle in a specified slot
void parkVehicle() {
    int slotNumber;
    string vehicleID;
    cout << "Enter the slot number where you want to park (1 - " << totalSlots << "): "; // Display slots from 1 to totalSlots
    cin >> slotNumber;

    // Adjust slot number to 0-based index
    slotNumber -= 1;

    cout << "Enter vehicle ID: ";
    cin >> vehicleID;

    // Check if the slot is valid and not already occupied
    if (slotNumber < 0 || slotNumber >= totalSlots) {
        cout << "Invalid slot number!\n";
        return;
    }
    if (slots[slotNumber].isOccupied) {
        cout << "Slot is already occupied!\n";
        return;
    }

    // Park the vehicle
    slots[slotNumber].isOccupied = true;
    slots[slotNumber].vehicleID = vehicleID;
    slots[slotNumber].entryTime = time(0); // Set the entry time to current time
    vehicleMap[vehicleID] = slotNumber;   // Map the vehicle ID to the slot number
    ++dailyOccupancy;  // Increment daily occupancy
    ++weeklyOccupancy; // Increment weekly occupancy

    cout << "Vehicle " << vehicleID << " parked in slot " << (slotNumber + 1) << ".\n";
}

// Function to calculate fees for a vehicle based on the time spent in the parking lot
void calculateFees(const string& vehicleID) {
    if (vehicleMap.find(vehicleID) == vehicleMap.end()) {
        cout << "Vehicle ID not found.\n";
        return;
    }

    int slotNumber = vehicleMap[vehicleID];
    time_t currentTime = time(0);
    double hoursSpent = difftime(currentTime, slots[slotNumber].entryTime) / 3600; // Calculate time spent in hours
    double fee = hoursSpent * hourlyRate; // Calculate the fee

    cout << "Time spent: " << fixed << setprecision(2) << hoursSpent << " hours\n";
    cout << "Total fee: $" << fixed << setprecision(2) << fee << endl;
}

// Function to view charges and let the vehicle leave the parking lot
void viewChargesAndLeave(const string& vehicleID) {
    if (vehicleMap.find(vehicleID) == vehicleMap.end()) {
        cout << "Vehicle ID not found.\n";
        return;
    }

    int slotNumber = vehicleMap[vehicleID];
    time_t currentTime = time(0);
    double hoursSpent = difftime(currentTime, slots[slotNumber].entryTime) / 3600; // Calculate time spent
    double fee = hoursSpent * hourlyRate; // Calculate fee based on time spent
    dailyRevenue += fee;  // Add fee to daily revenue
    weeklyRevenue += fee; // Add fee to weekly revenue
    slots[slotNumber].isOccupied = false;  // Mark slot as unoccupied
    slots[slotNumber].vehicleID = "";      // Clear vehicle ID
    vehicleMap.erase(vehicleID);           // Remove vehicle ID from map

    cout << "Total fee for vehicle " << vehicleID << ": $" << fixed << setprecision(2) << fee << endl;
    cout << "Vehicle " << vehicleID << " has left the parking lot.\n";
}

// User mode function with options for parking and fee calculation
void userMode() {
    int choice;
    string vehicleID;
    while (true) {
        // Display user menu
        cout << "Welcome to customer dashboard!\nYou have the ability to:\n";
        cout << "1. Check real-time parking availability\n";
        cout << "2. Park a vehicle\n";
        cout << "3. Calculate fees based on time spent\n";
        cout << "4. View charges and leave the parking lot\n";
        cout << "5. Exit customer dashboard\n"; // Option to exit User mode
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
        case 1:
            checkAvailability(); // Call function to check parking availability
            break;
        case 2:
            parkVehicle(); // Call function to park a vehicle
            break;
        case 3:
            cout << "Enter vehicle ID: ";
            cin >> vehicleID;
            calculateFees(vehicleID); // Call function to calculate fees
            break;
        case 4:
            cout << "Enter vehicle ID: ";
            cin >> vehicleID;
            viewChargesAndLeave(vehicleID); // Call function to view charges and leave
            break;
        case 5:
            cout << "Exiting to the main menu...\n";
            return; // Exit User mode and return to main menu
        default:
            cout << "Invalid choice.\n";
        }
    }
}