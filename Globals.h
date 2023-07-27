#ifndef GLOBALS_H
#define GLOBALS_H
#include "Tour.h"
#include "TourGuide.h"
#include "TourGuideApp.h"
#include <vector>
class Tour;

extern bool isAdminLoggedIn;
extern int loggedInTourGuideIndex;
extern std::vector<Tour> tours;
extern std::vector<TourGuide> tourGuides;
extern const int NUM_DAYS;
extern const int NUM_HOURS;
extern const int TOTAL_HOURS;
extern std::vector<std::vector<int> > availability;
extern sqlite3* db;

#endif // GLOBALS_H
