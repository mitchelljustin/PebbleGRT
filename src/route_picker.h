#include "defines.h"

static route_id_t AVAILABLE_ROUTES[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 27, 29, 31, 32, 33, 35, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 71, 72, 73, 75, 91, 92, 110, 111, 116, 200, 201, 202};
static char *AVAILABLE_ROUTE_ID_STRINGS[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "20", "21", "22", "23", "24", "25", "27", "29", "31", "32", "33", "35", "51", "52", "53", "54", "55", "56", "57", "58", "59", "60", "61", "62", "63", "64", "65", "66", "67", "71", "72", "73", "75", "91", "92", "110", "111", "116", "200", "201", "202"};
static char *AVAILABLE_ROUTE_NAME_STRINGS[] = {
    "Stanley Park",
    "Forest Hill",
    "Ottawa South",
    "Glasgow",
    "Erb West",
    "Bridgeport",
    "Mainline",
    "University/Fairview Park",
    "Lakeshore",
    "Conestoga College",
    "Country Hills",
    "Fairview Park/Conestoga Mall",
    "Laurelwood",
    "Bathurst",
    "Frederick",
    "Forest Glen/Conestoga College",
    "Heritage Park",
    "Guelph Street",
    "Victoria South",
    "Victoria Hills",
    "Elmira",
    "Laurentian West",
    "Idlewood",
    "Highland",
    "Queen South",
    "Chicopee",
    "Keats Way",
    "Lexington",
    "Kumpf",
    "Huron",
    "Eastbridge",
    "Hespeler Road",
    "Fairview Park/Ainslie Street",
    "Franklin Boulevard",
    "Lisbon Pines",
    "St. Andrews",
    "Dunbar",
    "Blair Road",
    "Elmwood",
    "Christopher",
    "Northview Acres",
    "Conestoga College/Cambridge Centre",
    "Woodside",
    "Champlain",
    "Langs",
    "Fisher Mills",
    "Winston",
    "Lovell Industrial",
    "Melran",
    "Cherry Blossom BusPlus",
    "Northlake BusPlus",
    "Saginaw BusPlus",
    "Late Night Loop",
    "Yolobus",
    "Fairview Park/Conestoga College Express",
    "Ainslie Street/Conestoga College Express",
    "Forest Glen/Conestoga College Express",
    "iXpress",
    "iXpress",
    "iXpress"
};

void push_route_picker_window(void (*callback)(route_id_t));