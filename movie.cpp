#include <iostream>
#include <vector>
#include <iomanip>
#include <ctime> // for getting current time
#include <algorithm> // for find function
#include <sqlite3.h> 

// Class definitions
class Tour {
public:
    std::string type;
    std::string description;
    std::string date;
    std::string time;
    int length;

    Tour(std::string type, std::string description, std::string date, std::string time, int length)
        : type(type), description(description), date(date), time(time), length(length) {}
};

class TourGuide {
public:
    std::string name;
    std::string email; // New field for email
    std::string password; // New field for password
    std::vector<int> registeredTours;

    TourGuide(std::string name, std::string email, std::string password)
        : name(name), email(email), password(password) {}
};

// Global variables
bool isAdminLoggedIn = false;
int loggedInTourGuideIndex = -1;
std::vector<Tour> tours;
std::vector<TourGuide> tourGuides;

const int NUM_DAYS = 7; // Number of days in a week
const int NUM_HOURS = 11; // Number of hours from 8:00 am to 6:00 pm
const int TOTAL_HOURS = NUM_DAYS * NUM_HOURS; // Total number of hours in a week

// Function prototypes
void displayMainMenu();
void adminLogin();
int tourGuideLogin();
void displayAdminMenu();
void displayTourGuideMenu();
void addTour();
void viewTours();
void addTourGuide();
void viewTourGuides();
void viewProfile();
void viewTourScripts();
void viewTourRoutes();
void editAvailability();
void viewOtherTourGuides();
void registerForTour();
void viewRegisteredTours();
void deleteTourGuide(); // New function to delete a tour guide
void closeApplication();
void viewOtherTourGuides();
void createTourGuideTable();

// 2D vector representing availability
std::vector<std::vector<int> > availability(NUM_HOURS, std::vector<int>(NUM_DAYS, 0));

// Function to initialize the availability 2D vector
void initializeAvailability() {
    std::vector<std::vector<bool> > availability(NUM_HOURS, std::vector<bool>(NUM_DAYS, false));
}

sqlite3* db = nullptr;

int openDatabase() {
    int rc = sqlite3_open("your_database_name.db", &db);
    if (rc) {
        std::cerr << "Error opening database: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return -1;
    }
    return 0;
}

void closeDatabase() {
    sqlite3_close(db);
}

void createTourGuideTable() {
    const char* sql = "CREATE TABLE IF NOT EXISTS tour_guides ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "name TEXT NOT NULL,"
                      "email TEXT NOT NULL,"
                      "password TEXT NOT NULL"
                      ");";
    char* errMsg;
    int rc = sqlite3_exec(db, sql, nullptr, 0, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}

// Function to display availability table
void displayAvailability() {
    std::cout << "\nEdit Availability\n";

    // Display table header
    std::cout << "+-------+";
    for (int day = 0; day < NUM_DAYS; ++day) {
        std::cout << "--------+";
    }
    std::cout << "\n| Time  |";
    for (int day = 0; day < NUM_DAYS; ++day) {
        std::cout << std::setw(8) << "Day " << day << "|";
    }
    std::cout << "\n+-------+";
    for (int day = 0; day < NUM_DAYS; ++day) {
        std::cout << "--------+";
    }

    // Display time slots with 'X' for selected slots
    const char selectedMarker = 'X';
    for (int hour = 0; hour < NUM_HOURS; ++hour) {
        std::cout << "\n| " << std::setw(5) << 8 + hour << ":00 |";
        for (int day = 0; day < NUM_DAYS; ++day) {
            if (availability[hour][day]) {
                std::cout << std::setw(8) << selectedMarker << "|";
            } else {
                std::cout << "        |";
            }
        }
        std::cout << "\n+-------+";
        for (int day = 0; day < NUM_DAYS; ++day) {
            std::cout << "--------+";
        }
    }

    std::cout << "\n\n";
}

void editAvailability() {
    if (loggedInTourGuideIndex != -1) {
        int dayOfWeek, startHour, endHour;
        char confirm;

        std::cout << "Edit Availability\n";
        std::cout << "Choose day of the week (0 - 6, where 0 = Sunday): ";
        std::cin >> dayOfWeek;
        if (dayOfWeek < 0 || dayOfWeek >= NUM_DAYS) {
            std::cout << "Invalid day of the week. Please try again.\n";
            return;
        }

        std::cout << "Choose starting hour (0 - 10): ";
        std::cin >> startHour;
        if (startHour < 0 || startHour > 10) {
            std::cout << "Invalid starting hour. Please try again.\n";
            return;
        }

        std::cout << "Choose ending hour (0 - 10): ";
        std::cin >> endHour;
        if (endHour < 0 || endHour > 10) {
            std::cout << "Invalid ending hour. Please try again.\n";
            return;
        }

        // Validate startHour and endHour range
        if (startHour > endHour) {
            std::cout << "Starting hour cannot be greater than ending hour. Please try again.\n";
            return;
        }

        // Update the availability based on the selection
        for (int hour = startHour; hour <= endHour; ++hour) {
            availability[hour][dayOfWeek] = 1;
        }

        // Show the updated availability table
        displayAvailability();

        // Ask for confirmation or further edit
        std::cout << "Confirm your selection (Y/N) or enter 'E' to edit further: ";
        std::cin >> confirm;

        if (confirm == 'Y' || confirm == 'y') {
            std::cout << "Availability updated successfully!\n\n";
        } else if (confirm == 'E' || confirm == 'e') {
            // If the user chooses to edit further, call the function recursively
            editAvailability();
        } else {
            // If the user enters any other input, discard the changes
            std::cout << "Availability update cancelled.\n\n";
            for (int hour = startHour; hour <= endHour; ++hour) {
                availability[hour][dayOfWeek] = 0;
            }
        }
    }
}

void addTour() {
    std::string type, description, date, time;
    int length;
    std::cout << "Enter Tour Type (PS - prospective students, LG - Large group, AS - Admitted students, HT - Housing tours): ";
    std::cin.ignore();
    std::getline(std::cin, type);
    std::cout << "Enter Description: ";
    std::getline(std::cin, description);
    std::cout << "Enter Date (YYYY-MM-DD): ";
    std::getline(std::cin, date);
    std::cout << "Enter Time (HH:MM): ";
    std::getline(std::cin, time);
    std::cout << "Enter Length (in hours): ";
    std::cin >> length;

    tours.push_back(Tour(type, description, date, time, length));
    std::cout << "Tour added successfully!\n";
}

void viewTours() {
    std::cout << "\nAll Tours\n";
    std::cout << "+------+--------------------------------+------------+----------+-------+\n";
    std::cout << "| Type | Description                    | Date       | Time     | Length|\n";
    std::cout << "+------+--------------------------------+------------+----------+-------+\n";

    int index = 0;
    for (const auto& tour : tours) {
        std::cout << "| " << std::left << std::setw(6) << tour.type << "| " << std::left << std::setw(30) << tour.description
                  << "| " << std::left << std::setw(10) << tour.date << "| " << std::left << std::setw(8) << tour.time
                  << "| " << std::left << std::setw(6) << tour.length << "|\n";
        index++;
    }

    std::cout << "+------+--------------------------------+------------+----------+-------+\n";
}

void addTourGuide() {
    std::string name, email, password;
    std::cout << "Enter Tour Guide Name: ";
    std::cin.ignore();
    std::getline(std::cin, name);
    std::cout << "Enter Tour Guide Email: ";
    std::getline(std::cin, email);
    std::cout << "Enter Tour Guide Password: ";
    std::getline(std::cin, password);

    const char* sql = "INSERT INTO tour_guides (name, email, password) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, email.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, password.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
    std::cout << "Tour guide added successfully!\n";
}

void deleteTourGuide() {
    if (isAdminLoggedIn) {
        std::string name;
        std::cout << "Enter the name of the tour guide you want to delete: ";
        std::cin.ignore();
        std::getline(std::cin, name);

        const char* sql = "DELETE FROM tour_guides WHERE name = ?;";
        sqlite3_stmt* stmt;
        int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
            return;
        }

        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        }

        sqlite3_finalize(stmt);
        std::cout << "Tour guide '" << name << "' deleted successfully!\n";
    } else {
        std::cout << "You don't have admin privileges to delete a tour guide.\n";
    }
}


void viewTourGuides() {
    std::cout << "\nAll Tour Guides\n";
    std::cout << "+----------------------+---------------------+---------------------+\n";
    std::cout << "| Name                 | Contact Info        |Password             |\n";
    std::cout << "+----------------------+---------------------+---------------------+\n";

    const char* sql = "SELECT * FROM tour_guides;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        std::string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        std::string password = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        tourGuides.push_back(TourGuide(name, email, password));
        std::cout << "| " << std::left << std::setw(20) << name << "| " << std::left << std::setw(20) << email << std::setw(20) << password << "|\n";
    }

    if (rc != SQLITE_DONE) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
    std::cout << "+----------------------+---------------------+---------------------+\n";
}

void viewOtherTourGuides() {
    std::cout << "\nAll Tour Guides\n";
    std::cout << "+----------------------+---------------------+\n";
    std::cout << "| Name                 | Contact Info        |\n";
    std::cout << "+----------------------+---------------------+\n";

    const char* sql = "SELECT * FROM tour_guides;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        std::string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        std::string password = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        tourGuides.push_back(TourGuide(name, email, password));
        std::cout << "| " << std::left << std::setw(20) << name << "| " << std::left << std::setw(20) << email << "|\n";
    }

    if (rc != SQLITE_DONE) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
    std::cout << "+----------------------+---------------------+\n";
}

void viewProfile() {
    if (loggedInTourGuideIndex != -1) {
        const TourGuide& loggedInTourGuide = tourGuides[loggedInTourGuideIndex];
        std::cout << "\nTour Guide Profile\n";
        std::cout << "Welcome, " << loggedInTourGuide.name << "!\n";
        std::time_t now = std::time(0);
        std::cout << "Current Time: " << std::asctime(std::localtime(&now)) << "\n";
        std::cout << "Contact Info: " << loggedInTourGuide.email << "\n\n";
    }
}

void viewTourScripts() {
    if (loggedInTourGuideIndex != -1) {
        const TourGuide& loggedInTourGuide = tourGuides[loggedInTourGuideIndex];
        std::cout << "\nTour Scripts for " << loggedInTourGuide.name << "\n";
        std::cout << "No tour scripts available.\n\n";
    }
}

void viewTourRoutes() {
    if (loggedInTourGuideIndex != -1) {
        std::cout << "\nView Tour Routes\n";
        std::cout << "Please open the tour route PDF.\n\n";
    }
}



void registerForTour() {
    if (loggedInTourGuideIndex != -1) {
        std::cout << "\nRegister for a Tour\n";
        // Display available tours
        std::cout << "Available Tours:\n";
        std::cout << "+------+--------------------------------+------------+----------+-------+\n";
        std::cout << "| Index| Type                           | Description| Date     | Time  |\n";
        std::cout << "+------+--------------------------------+------------+----------+-------+\n";

        int index = 0;
        for (const auto& tour : tours) {
            bool isRegistered = std::find(tourGuides[loggedInTourGuideIndex].registeredTours.begin(),
                                          tourGuides[loggedInTourGuideIndex].registeredTours.end(),
                                          index) != tourGuides[loggedInTourGuideIndex].registeredTours.end();

            if (!isRegistered) {
                std::cout << "| " << std::left << std::setw(6) << index << "| " << std::left << std::setw(30) << tour.type
                          << "| " << std::left << std::setw(11) << tour.description << "| " << std::left << std::setw(8) << tour.date
                          << "| " << std::left << std::setw(5) << tour.time << "|\n";
            }
            index++;
        }

        std::cout << "+------+--------------------------------+------------+----------+-------+\n";

        int tourIndex;
        std::cout << "Enter the index of the tour you want to register for: ";
        std::cin >> tourIndex;

        if (tourIndex >= 0 && tourIndex < static_cast<int>(tours.size())) {
            // Check if the tour is not already registered for
            if (std::find(tourGuides[loggedInTourGuideIndex].registeredTours.begin(),
                          tourGuides[loggedInTourGuideIndex].registeredTours.end(),
                          tourIndex) == tourGuides[loggedInTourGuideIndex].registeredTours.end()) {
                tourGuides[loggedInTourGuideIndex].registeredTours.push_back(tourIndex);
                std::cout << "You are now registered for the selected tour.\n";
            } else {
                std::cout << "You are already registered for this tour.\n";
            }
        } else {
            std::cout << "Invalid tour index. Please try again.\n";
        }
    }
}

void viewRegisteredTours() {
    if (loggedInTourGuideIndex != -1) {
        const TourGuide& loggedInTourGuide = tourGuides[loggedInTourGuideIndex];
        std::cout << "\nRegistered Tours for " << loggedInTourGuide.name << "\n";
        std::cout << "+------+--------------------------------+------------+----------+-------+\n";
        std::cout << "| Index| Type                           | Description| Date     | Time  |\n";
        std::cout << "+------+--------------------------------+------------+----------+-------+\n";

        for (int tourIndex : loggedInTourGuide.registeredTours) {
            const Tour& tour = tours[tourIndex];
            std::cout << "| " << std::left << std::setw(6) << tourIndex << "| " << std::left << std::setw(30) << tour.type
                      << "| " << std::left << std::setw(11) << tour.description << "| " << std::left << std::setw(8) << tour.date
                      << "| " << std::left << std::setw(5) << tour.time << "|\n";
        }

        std::cout << "+------+--------------------------------+------------+----------+-------+\n";
    }
}

void closeApplication() {
    std::cout << "Closing application... Goodbye!\n";
    closeDatabase();
    std::exit(0);
}

void displayMainMenu() {
    std::cout << "\nMain Menu\n";
    std::cout << "1. Admin Login\n";
    std::cout << "2. Tour Guide Login\n";
    std::cout << "3. Close Application\n";
    std::cout << "Enter your choice (1-3): ";
}

void adminLogin() {
    std::string adminPassword;
    std::cout << "Enter the admin password: ";
    std::cin.ignore();
    std::getline(std::cin, adminPassword);

    // Replace "your_admin_password" with the actual admin password
    if (adminPassword == "password") {
        isAdminLoggedIn = true;
        std::cout << "Admin login successful!\n";
        displayAdminMenu();
    } else {
        std::cout << "Invalid password. Admin login failed.\n";
    }
}

int tourGuideLogin() {
    std::string name, email;

    std::cout << "Enter Tour Guide Name: ";
    std::cin.ignore();
    std::getline(std::cin, name);

    std::cout << "Enter Contact Info: ";
    std::getline(std::cin, email);

    const char* sql = "SELECT id, name, email FROM tour_guides WHERE name = ? AND email = ?;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        return -1;
    }

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, email.c_str(), -1, SQLITE_STATIC);

    int tourGuideIndex = -1;
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        tourGuideIndex = sqlite3_column_int(stmt, 0);
    } else if (rc != SQLITE_DONE) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
    return tourGuideIndex;
}

#include <iostream>

void displayAdminMenu() {
    std::cout << "\n*********** Admin Menu *************\n";
    std::cout << "+------------------------------------+\n";
    std::cout << "|       [1] Add Tour                 |\n";
    std::cout << "|       [2] View Tours               |\n";
    std::cout << "|       [3] Add Tour Guide           |\n";
    std::cout << "|       [4] View Tour Guides         |\n";
    std::cout << "|       [5] Delete Tour Guide        |\n";
    std::cout << "|       [6] Log out                  |\n";
    std::cout << "+------------------------------------+\n";
    std::cout << "| Enter your choice (1-6):           |\n";
    std::cout << "+------------------------------------+\n";

}


void displayTourGuideMenu() {
    std::cout << "\n********* Tour Guide Menu **********\n";
    std::cout << "+------------------------------------+\n";
    std::cout << "|       [1] View Profile             |\n";
    std::cout << "|       [2] View Tour Scripts        |\n";
    std::cout << "|       [3] View Tour Routes         |\n";
    std::cout << "|       [4] Edit Availability        |\n";
    std::cout << "|       [5] View Other Tour Guides   |\n";
    std::cout << "|       [6] Register for Tour        |\n";
    std::cout << "|       [7] View Registered Tours    |\n";
    std::cout << "|       [8] Logout                   |\n";
    std::cout << "+------------------------------------+\n";
    std::cout << "| Enter your choice (1-8):           |\n";
    std::cout << "+------------------------------------+\n";

}

int main() {
    int choice;
    openDatabase();
    createTourGuideTable();
    std::cout <<" ";
    while (true) {
        displayMainMenu();
        std::cin >> choice;

        switch (choice) {
            case 1: // Admin Login
                adminLogin();
                break;

            case 2: // Tour Guide Login
                loggedInTourGuideIndex = tourGuideLogin();
                if (loggedInTourGuideIndex != -1) {
                    displayTourGuideMenu();
                } else {
                    std::cout << "Tour guide login failed. Please check your name and contact info.\n";
                }
                break;

            case 3: // Close Application
                closeApplication();
                break;

            default:
                std::cout << "Invalid choice. Please try again.\n";
                break;
        }

        // If the admin is logged in, display the admin menu
        while (isAdminLoggedIn) {
            std::cin >> choice;

            switch (choice) {
                case 1: // Add Tour
                    addTour();
                    break;

                case 2: // View Tours
                    viewTours();
                    break;

                case 3: // Add Tour Guide
                    addTourGuide();
                    break;

                case 4: // View Tour Guides
                    viewTourGuides();
                    break;

                case 5:
                    deleteTourGuide();
                    break;

                case 6: // Back to Main Menu
                    isAdminLoggedIn = false;
                    break;

                default:
                    std::cout << "Invalid choice. Please try again.\n";
                    break;
            }

            displayAdminMenu();
        }

        // If a tour guide is logged in, display the tour guide menu
        while (loggedInTourGuideIndex != -1) {
            std::cin >> choice;
            switch (choice) {
                case 1: // View Profile
                    viewProfile();
                    break;

                case 2: // View Tour Scripts
                    viewTourScripts();
                    break;

                case 3: // View Tour Routes
                    viewTourRoutes();
                    break;

                case 4: // Edit Availability
                    editAvailability();
                    break;

                case 5: // View Other Tour Guides
                    viewOtherTourGuides();
                    break;

                case 6: // Register for Tour
                    registerForTour();
                    break;

                case 7: // View Registered Tours
                    viewRegisteredTours();
                    break;

                case 8: // Back to Main Menu
                    loggedInTourGuideIndex = -1;
                    break;

                default:
                    std::cout << "Invalid choice. Please try again.\n";
                    break;
            }

            displayTourGuideMenu();
        }
    }

    return 0;
}
