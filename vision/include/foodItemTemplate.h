#ifndef FOOD_ITEM_TEMPLATE_H
#define FOOD_ITEM_TEMPLATE_H

#include <string>
#include <unordered_map>

struct FoodItemTemplate {
    int shelf_life_days; //estimated
    double weight_lbs; //estimated (in grams)
};


// Define the mapping with food name as the key
std::unordered_map<std::string, FoodItemTemplate> food_labels = {
    {"guacamole", {4, 227}}, {"consomme", {3, 298}},
    {"trifle", {2, 680}}, {"ice_cream", {90, 1360}}, {"ice_lolly", {210, 85}},
    {"French_loaf", {2, 454}}, {"bagel", {1, 85}}, {"pretzel", {1, 85}},
    {"cheeseburger", {0, 170}}, {"hotdog", {7, 45}}, {"mashed_potato", {4, 0}}, // Variable weight
    {"head_cabbage", {45, 1360}}, {"broccoli", {4, 454}}, {"cauliflower", {7, 680}},
    {"zucchini", {4, 198}}, {"spaghetti_squash", {45, 2268}}, {"acorn_squash", {45, 907}},
    {"butternut_squash", {75, 1360}}, {"cucumber", {7, 198}}, {"artichoke", {7, 198}},
    {"bell_pepper", {12, 170}}, {"cardoon", {17, 680}}, {"mushroom", {7, 227}},
    {"Granny_Smith", {37, 198}}, {"strawberry", {5, 454}}, {"orange", {24, 198}},
    {"lemon", {24, 142}}, {"fig", {2, 57}}, {"pineapple", {4, 1360}},
    {"banana", {4, 142}}, {"jackfruit", {17, 4536}}, {"custard_apple", {2, 227}},
    {"pomegranate", {45, 255}},
    {"chocolate_sauce", {180, 283}}, {"dough", {2, 0}}, // Variable weight
    {"meat_loaf", {3, 907}}, {"pizza", {3, 680}}, {"potpie", {4, 680}},
    {"burrito", {3, 283}}, {"red_wine", {4, 750}}, {"espresso", {0, 28}}, // Espresso consumed immediately
    {"eggnog", {4, 946}}
};
#endif