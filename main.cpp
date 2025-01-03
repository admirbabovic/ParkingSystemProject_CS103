#include <iostream>
#include <iomanip>
#include "mylibrary.h"

using namespace std;

/* Main function that allows users to choose between
 * logging in as an existing user or registering as a new user
 * and then redirects to User or Admin dashboard mode
 * Demo administrator account - admin:admin123!
 * Demo user account - user:user123!
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
