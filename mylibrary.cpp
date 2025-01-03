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
#include <algorithm>
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
string generateSalt(int length) {
    vector<unsigned char> salt(length);
    if (RAND_bytes(salt.data(), length) != 1) {
        cerr << "Error generating random salt!" << endl;
        exit(1);
    }
    return bytesToHex(salt);
}

// Function to hash password using SHA-256
string hashPasswordWithSalt(const string& password, const string& salt) {
    vector<unsigned char> hash(SHA256_DIGEST_LENGTH);
    const string salted_password = password + salt;

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

        // Check if username is taken
        while (checkUsername(username)) {
            // Prompt for new username
            cout << "Username already exists. Please choose a different one." << endl;
            cout << "Here are some suggestions: " << suggestUsername(username) << " | "
                                        << suggestUsername(username) << " | "
                                << suggestUsername(username) << endl; // Display random generated username suggestions
            cout << "Enter username: ";
            getline(cin, username);
        }

        cout << "Enter password: ";
        getline(cin, password);
    }

    // Generate salt and hash the password
    string salt = generateSalt(16);
    string password_hash = hashPasswordWithSalt(password, salt);

    // Open credentials file to save new user info
    ofstream file(FILE_PATH_DIR, ios::app);
    // Check if the file is open
    if (!file.is_open()) {
        cerr << "Unable to open file!" << endl;  // Display error
        exit(1);
    }

    // Save the user data (username, hashed password, and salt)
    file << username << " " << password_hash << " " << salt << endl;
    cout << "User registered successfully!" << endl;
    file.close();

    cout << "Proceed to login" << endl;
    return;
}

// Function that check if username exist in credentials file
bool checkUsername(const string& username) {
    ifstream file(FILE_PATH_DIR);
    // Check if file exists
    if (!file.is_open()) {
        return false;  // Return false value
    }

    // If file exists check whether username exists
    string stored_username, stored_password_hash, stored_salt;
    while (file >> stored_username >> stored_password_hash >> stored_salt) {
        if (stored_username == username) {
            return true; // Username found
        }
    }
    return false; // Username not found
}

// Function that generates random available variation of taken username
string suggestUsername(const string username) {
    int valid = 0;  // Initialization of validation parameter
    string newUsername;  // Initialize new username string

    do {
        // Generate new username that uses old one as a base
        string usernameAddon = generateSalt(2);
        transform(usernameAddon.begin(), usernameAddon.end(), usernameAddon.begin(), ::toupper);  // Capitalize entire generated string
        newUsername = username + "_" + usernameAddon;  // Combining taken username and generated addon
        if (!checkUsername(newUsername))  // Check if new username exists
            valid = 1;
    } while (valid == 0);

    return newUsername;
}

// Function to check login credentials
void loginUser() {
    string username, password;

    cout << "Enter username: ";
    getline(cin, username);

    // Check if the username exists
    if (!checkUsername(username)) {
        string choice;
        // Ask user to create new account
        do {
            cout << "Username not found! Would you like to register a new account? (y/n): " << endl;;
            getline(cin, choice);
        } while (!(choice == "Y" || choice == "y" || choice == "N" || choice == "n"));

        // Ask user to use pre-filled username
        if (choice == "Y" || choice == "y") {
            string choice2;
            do {
                cout << "Would you like to use \"" << username << "\" as username? (y/n): ";
                getline(cin, choice2);
            } while (!(choice2 == "Y" || choice2 == "y" || choice2 == "N" || choice2 == "n"));

            // Create new account with pre-filled username
            if (choice2 == "Y" || choice2 == "y") {
                registerUser(username);
            }
            // Prompt for new username
            else {
                string newUsername;
                cout << "Enter new username that you would like to use: ";
                getline(cin, newUsername);

                // Check if username is taken
                while (checkUsername(newUsername)) {
                    // Prompt for new username
                    cout << "Username already exists. Please choose a different one." << endl;
                    cout << "Here are some suggestions: " << suggestUsername(newUsername) << " | "
                                                << suggestUsername(newUsername) << " | "
                                        << suggestUsername(newUsername) << endl;  // Display random generated username suggestions
                    cout << "Enter username: ";
                    getline(cin, newUsername);
                }

                registerUser(newUsername);
            }
        }
        else {
            cout << "Please try to log in again." << endl;
            cout << "Exiting to main menu..." << endl;
            return;
        }
    }

    // If username exists, proceed to prompt for password
    cout << "Enter password: ";
    getline(cin, password);

    ifstream file(FILE_PATH_DIR);
    // Display error message if the file is not opened
    if (!file.is_open()) {
        cerr << "Unable to open file!" << endl;
        exit(1);
    }

    // Loop through credentials.dat file and check if password matches stored hashed password
    string stored_username, stored_password_hash, stored_salt;
    while (file >> stored_username >> stored_password_hash >> stored_salt) {
        if (stored_username == username) {
            // Hash the entered password with the stored salt
            string entered_password_hash = hashPasswordWithSalt(password, stored_salt);
            if (entered_password_hash == stored_password_hash) {  // Validity check for password
                cout << "Logging in ..." << endl;
                cout << "Login successful!" << endl;
                // Check whether account used to log in is administrator or user
                if (username == "admin") {
                    file.close();  // Close credentials file
                    adminMode(); // Enter Admin mode
                    return;
                }
                else {
                    file.close();  // Close credentials file
                    userMode(); // Enter User mode
                    return;
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