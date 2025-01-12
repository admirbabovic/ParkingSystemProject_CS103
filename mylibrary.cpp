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
    if (!file) {
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
    bool isOccupied{};   // Indicates whether the slot is taken
    string vehicleID;  // The ID of the vehicle which is parked parked in this slot
    time_t entryTime{};  // The time at which the vehicle entered the parking slot
};

vector<ParkingSlots> slots; // Vector for stroing parking slots
unordered_map<string, int> vehicleMap; // It maps vehicle ID to a slot number
int totalSlots;            // Total amount of parking slots
double hourlyRate;         // Hourly parking rate
double dailyRate = hourlyRate * 24 * 0.7;  // Daily parking rate at a discount of 30%
double dailyRevenue = 0.0; // The amount of revenue generated for the current day
double weeklyRevenue = 0.0; // The amount of revenue generated this week
int dailyOccupancy = 0;    // The amount of parking spaces taken today
int weeklyOccupancy = 0;   // The amount of parking spaces taken this week

void configureSlotsAndRates() {
    cout << "Enter the number of slots: ";
    cin >> totalSlots;
    cout << "Enter the hourly rate: ";
    cin >> hourlyRate;
    cout << "Enter the daily rate: ";
    cin >> dailyRate;

    // Resizes the slots vector and initializes the data
    slots.resize(totalSlots);
    for (int i = 0; i < totalSlots; ++i) {
        slots[i].isOccupied = false;  // At the start, not parking spaces are taken
        slots[i].vehicleID = "";      // No vehicle parked at the start
    }
    cout << "Configuration updated." << endl;
}

// Function for generating daily and weekly revenue report
void generateReports() {
    cout << "Daily Revenue: $" << fixed << setprecision(2) << dailyRevenue << endl;
    cout << "Daily Occupancy: " << dailyOccupancy << " / " << totalSlots << endl;
    cout << "Weekly Revenue: $" << fixed << setprecision(2) << weeklyRevenue << endl;
    cout << "Weekly Occupancy: " << weeklyOccupancy << endl;
}

// Function which allows the admin to configure the slots and generate a report
void adminMode() {
    int choice;
    while (true) {
        // Displays the admin menu
        cout << "Welcome to admin dashboard!\nYou can choose to:" << endl;
        cout << "1. Configure parking slots and rates" << endl;
        cout << "2. Generate daily/weekly reports" << endl;
        cout << "3. Exit admin dashboard" << endl; // Leaves the admin mode
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
        case 1:
            configureSlotsAndRates(); // Calls a function for configuring the amount of parking slots and fee rates 
            break;
        case 2:
            generateReports(); // Calls a function to generate a report 
            break;
        case 3:
            cout << "Exiting to the main menu..." << endl;
            return; // Exists from admin menu and returns back to main menu
        default:
            cout << "Invalid choice." << endl;
        }
    }
}

// Function for checking all of the available parking slots
void checkAvailability() {
    int availableSlots = 0;
    for (const auto& slot : slots) {
        if (!slot.isOccupied) {
            ++availableSlots;  // It checks and counts the amount of available parking slot
        }
    }
    cout << "Available slots: " << availableSlots << " / " << totalSlots << endl;

    // It checks if the is one slot left
    if (availableSlots == 1) {
        cout << "Hurry up, parking is almost full!!!" << endl; // Displays that the is only one parking space left
    }
}

// Function which parks a vehicle into a specific chosen parking slot
void parkVehicle() {
    int slotNumber;
    string vehicleID;
    cout << "Enter the slot number where you want to park (1 - " << totalSlots << "): "; // Display slots from 1 to totalSlots
    cin >> slotNumber;

    // Adjust the slot number to counting from zero
    slotNumber -= 1;

    cout << "Enter vehicle ID: ";
    cin >> vehicleID;

    // Checks whether or not if a parking space is available or not
    if (slotNumber < 0 || slotNumber >= totalSlots) {
        cout << "Invalid slot number!" << endl;
        return;
    }
    if (slots[slotNumber].isOccupied) {
        cout << "Slot is already occupied!" << endl;
        return;
    }

    // Park the vehicle
    slots[slotNumber].isOccupied = true;
    slots[slotNumber].vehicleID = vehicleID;
    slots[slotNumber].entryTime = time(0); // Set the entry time to the current time
    vehicleMap[vehicleID] = slotNumber;   // Map the vehicle ID to the slot number
    ++dailyOccupancy;  // Increases daily occupancy by 1
    ++weeklyOccupancy; // Increases weekly occupancy by 1

    cout << "Vehicle " << vehicleID << " parked in slot " << (slotNumber + 1) << "." << endl;
}

// Function which calculates the fee for a vehicle being parked in a space depending on the time spent parking
void calculateFees(const string& vehicleID) {
    if (vehicleMap.find(vehicleID) == vehicleMap.end()) {
        cout << "Vehicle ID not found." << endl;
        return;
    }

    int slotNumber = vehicleMap[vehicleID];
    time_t currentTime = time(0);
    double hoursSpent = difftime(currentTime, slots[slotNumber].entryTime) / 3600; // Calculates the time spent in hours
    double fee = hoursSpent * hourlyRate; // Calculates the fee

    cout << "Time spent: " << fixed << setprecision(2) << hoursSpent << " hours" << endl;
    cout << "Total fee: $" << fixed << setprecision(2) << fee << endl;
}

// Function to the view charges and let the vehicle to leave the parking lot
void viewChargesAndLeave(const string& vehicleID) {
    if (vehicleMap.find(vehicleID) == vehicleMap.end()) {
        cout << "Vehicle ID not found." << endl;
        return;
    }

    int slotNumber = vehicleMap[vehicleID];
    time_t currentTime = time(0);
    double hoursSpent = difftime(currentTime, slots[slotNumber].entryTime) / 3600; // Calculate the amount of time spent
    double fee = hoursSpent * hourlyRate; // Calculates fee based on time spent
    dailyRevenue += fee;  // Adds fee to daily revenue
    weeklyRevenue += fee; // Adds fee to weekly revenue
    slots[slotNumber].isOccupied = false;  // Mark slot as unoccupied
    slots[slotNumber].vehicleID = "";      // Cleans vehicle ID
    vehicleMap.erase(vehicleID);           // Deletes vehicle ID from map

    cout << "Total fee for vehicle " << vehicleID << ": $" << fixed << setprecision(2) << fee << endl;
    cout << "Vehicle " << vehicleID << " has left the parking lot." << endl;
}

// User mode function with options for parking and fee calculation
void userMode() {
    int choice;
    string vehicleID;
    while (true) {
        // Display user menu
        cout << "Welcome to customer dashboard!" << endl << "You have the ability to : " << endl;
        cout << "1. Check real-time parking availability" << endl;
        cout << "2. Park a vehicle" << endl;
        cout << "3. Calculate fees based on time spent" << endl;
        cout << "4. View charges and leave the parking lot" << endl;
        cout << "5. Exit customer dashboard" << endl; // Option for leaving the user mode
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
        case 1:
            checkAvailability(); // Calls a function which check if there are available parking spaces 
            break;
        case 2:
            parkVehicle(); // Calls a funtion to park a vehicle
            break;
        case 3:
            cout << "Enter vehicle ID: ";
            cin >> vehicleID;
            calculateFees(vehicleID); // Calls a function for calculating the fees of parking
            break;
        case 4:
            cout << "Enter vehicle ID: ";
            cin >> vehicleID;
            viewChargesAndLeave(vehicleID); // Calls a function to view the charges and leave the parking lot
            break;
        case 5:
            cout << "Exiting to the main menu..." << endl;
            return; // Exist user mode and goes back to the main menu
        default:
            cout << "Invalid choice." << endl;
        }
    }
}