#include <iostream>
#include <iomanip>
#include "mylibrary.h"
using namespace std;

/* Project name: ParkingSystem
 * Group: 43
 * Group members: Ibrahim Omerasevic, Admir Babovic, Ammar Obralic, Aldin Sokol, Imaad Omerovic
 * Short description: Parking system which allows users to login and/or register new user account.
 * After loggin in, users are displayed dashboard which consists of features such as:
 * checking available parking slots, parking their vehicle, checking current charges etc. 
 */

/* For testing purposes use these demo credentials or create new ones
 * 
 * [DEMO ADMINISTRATOR ACCOUNT]
 * username: admin
 * password: admin123!
 * 
 * [DEMO USER ACCOUNT]
 * username: user
 * password: user123!
 */

/* Main function that allows users to choose log in to
 * an existing user/admininistrator account or register a new account
 * and then redirects user accounts to User Dashboard Mode
 * and administrator account to Admin Dashboard Mode
 */
int main() {
    int mode;
    while (true) {
        cout << "Would you like to: \n"<< "1. Login\n"<< "2. Register\n" << "0. Exit \nChoose: ";
        cin >> mode;

        if (mode == 1) {
            cin.ignore();
            loginUser(); // Proceed to log-in screen
        }
        else if (mode == 2) {
            cin.ignore();
            registerUser(""); // Proceed to registration screen
        }
        else if (mode == 0) {
            cout << "Exiting the program...\n";
            cout << "Goodbye!\n";
            break; // Exit the program
        }
        else {
            cout << "Invalid choice.\n";
        }
    }

    return 0;
}
