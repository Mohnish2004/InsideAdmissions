#ifndef TOURGUIDE_H
#define TOURGUIDE_H
#include "Globals.h"
#include <string>
#include <vector>

class TourGuide {
public:
    std::string name;
    std::string email;
    std::string password;
    std::vector<int> registeredTours;

    TourGuide(std::string name, std::string email, std::string password);
};

#endif // TOURGUIDE_H
