#pragma once
#include <iostream>
#include <vector>

using namespace std;

struct ParkingSlot;
void configureSlotsAndRates();
void generateReports();
void adminMode();
void checkAvailability();
void parkVehicle();
void calculateFees(const string& vehicleID);
void viewChargesAndLeave(const string& vehicleID);
void userMode();
string bytesToHex(const vector<unsigned char>& bytes);
string generateSalt(int length);
string hashPasswordWithSalt(const string& password, const string& salt, const string& username);
void registerUser(const string& prefilled_username);
bool checkUsername(const string& username);
void loginUser();