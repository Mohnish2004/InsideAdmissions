#ifndef TOURGUIDEAPP_H
#define TOURGUIDEAPP_H
#include "Globals.h"
void displayMainMenu();
void adminLogin();
int tourGuideLogin();
void displayAdminMenu();
void displayTourGuideMenu();
void addTour();
void viewTours();
void addTourGuide();
void deleteTourGuide();
void viewTourGuides();
void viewProfile();
void viewTourScripts();
void viewTourRoutes();
void editAvailability();
void viewOtherTourGuides();
void registerForTour();
void viewRegisteredTours();
void closeApplication();
void createTourGuideTable();
void createMessagesTable();
void displayAvailability();
int getHourIndex(const std::string& hourStr);
void sendRequestWithName(const std::string& name, const std::string& message);
void viewLatestMessages();

#endif // TOURGUIDEAPP_H
