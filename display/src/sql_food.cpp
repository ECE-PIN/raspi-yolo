#include <glog/logging.h>
#include <iostream>
#include <sqlite3.h>

#include "../../food_item.h"
#include "sql_food.h"

/**
 * Open the database and make sure the table for the food items is present.
 *
 * @param database Pointer to database pointer so that it can be modified (opened)
 * @return None
 */
void openDatabase(sqlite3** database) {
  int sqlReturn = sqlite3_open("test.db", database);
  if (sqlReturn != SQLITE_OK) {
    LOG(FATAL) << "Error opening database: " << sqlite3_errmsg(*database);
  }

  const char* createSqlTable = "CREATE TABLE IF NOT EXISTS foodItems("
                               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                               "name TEXT,"
                               "catagory TEXT,"
                               "scanDateYear INTEGER,"
                               "scanDateMonth INTEGER,"
                               "scanDateDay INTEGER,"
                               "expirationDateYear INTEGER,"
                               "expirationDateMonth INTEGER,"
                               "expirationDateDay INTEGER,"
                               "weight FLOAT,"
                               "quantity INTEGER);";

  char* errorMessage = nullptr;
  sqlReturn          = sqlite3_exec(*database, createSqlTable, NULL, NULL, &errorMessage);
  if (sqlReturn != SQLITE_OK) {
    LOG(FATAL) << "SQL Exec Error: " << errorMessage;
    sqlite3_free(errorMessage);
  }
}

/**
 * Store a food item into a database. Stores all food item fields besides the photo path.
 * Not storing photo path due to the photo only being used before the food item is stored
 * into the database.
 *
 * @param database The database to store the food item into
 * @param foodItem The food item to store
 * @return None
 */
void storeFoodItem(sqlite3* database, struct FoodItem foodItem) {
  const char* insertSql =
      "INSERT INTO foodItems (name, catagory, scanDateYear, "
      "scanDateMonth, scanDateDay, expirationDateYear, expirationDateMonth, "
      "expirationDateDay, weight, quantity) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

  sqlite3_stmt* statement;
  int sqlReturn = sqlite3_prepare_v2(database, insertSql, -1, &statement, nullptr);
  if (sqlReturn != SQLITE_OK) {
    LOG(FATAL) << "Prepare error: " << sqlite3_errmsg(database);
  }

  int scanDateYear           = static_cast<int>(foodItem.scanDate.year());
  unsigned int scanDateMonth = static_cast<unsigned>(foodItem.scanDate.month());
  unsigned int scanDateDay   = static_cast<unsigned>(foodItem.scanDate.day());
  int expirationDateYear     = static_cast<int>(foodItem.expirationDate.year());
  unsigned int expirationDateMonth =
      static_cast<unsigned>(foodItem.expirationDate.month());
  unsigned int expirationDateDay = static_cast<unsigned>(foodItem.expirationDate.day());

  sqlite3_bind_text(statement, 1, foodItem.name.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(statement, 2, foodItem.catagory.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_int(statement, 3, scanDateYear);
  sqlite3_bind_int(statement, 4, scanDateMonth);
  sqlite3_bind_int(statement, 5, scanDateDay);
  sqlite3_bind_int(statement, 6, expirationDateYear);
  sqlite3_bind_int(statement, 7, expirationDateMonth);
  sqlite3_bind_int(statement, 8, expirationDateDay);
  sqlite3_bind_double(statement, 9, foodItem.weight);
  sqlite3_bind_int(statement, 10, foodItem.quantity);

  sqlReturn = sqlite3_step(statement);
  if (sqlReturn != SQLITE_DONE) {
    LOG(FATAL) << "Execution Error: " << sqlite3_errmsg(database);
  }

  sqlite3_finalize(statement);
}

int readFoodItemCallback(void* foodItemVector,
                         int numColumns,
                         char** columns,
                         char** columnNames) {
  std::vector<FoodItem>* allFoodItems =
      static_cast<std::vector<FoodItem>*>(foodItemVector);
  struct FoodItem foodItem;

  for (int i = 0; i < numColumns; i++) {
    if (std::string(columnNames[i]) == "name") {
      foodItem.name = columns[i];
    }
  }
  allFoodItems->push_back(foodItem);
  return 0;
}
