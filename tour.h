#ifndef TOUR_H
#define TOUR_H
#include "Globals.h"
#include <string>
#include <vector>


class Tour {
public:
    std::string type;
    std::string description;
    std::string date;
    std::string time;
    int length;

    Tour(std::string type, std::string description, std::string date, std::string time, int length);
};

#endif // TOUR_H
