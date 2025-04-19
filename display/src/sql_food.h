#ifndef SQL_FOOD_H
#define SQL_FOOD_H

#include <sqlite3.h>
#include <vector>

#include "../src/elements/sort_method.h"

void openDatabase(sqlite3**);
int storeFoodItem(sqlite3*, struct FoodItem);

void setFoodItem(struct FoodItem& foodItem,
                 const std::string& columnValue,
                 const std::string& columnName);

std::vector<FoodItem> readAllFoodItems();
int readAllFoodItemsCallback(void* foodItemVector,
                             int numColumns,
                             char** columns,
                             char** columnNames);

std::vector<FoodItem> readAllFoodItemsSorted(const SortMethod& sortMethod);
int readAllFoodItemsSortedCallback(void* foodItemVector,
                                   int numColumns,
                                   char** columns,
                                   char** columnNames);

FoodItem readFoodItemById(const int id);
int readFoodItemByIdCallback(void* passedFoodItem,
                             int numColumns,
                             char** columns,
                             char** columnNames);

void updateFoodItemQuantity(const int id, const int newQuantity);

void deleteById(const int id);

#endif
