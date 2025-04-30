#include <glog/logging.h>
#include <iostream>
#include <sqlite3.h>
#include <sstream>

#include "../food_item.h"
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
                               "category TEXT,"
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
int storeFoodItem(sqlite3* database, struct FoodItem foodItem) {
  const char* insertSql =
      "INSERT INTO foodItems (name, category, scanDateYear, "
      "scanDateMonth, scanDateDay, expirationDateYear, expirationDateMonth, "
      "expirationDateDay, weight, quantity) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

  sqlite3_stmt* statement = nullptr;
  int sqlReturn = sqlite3_prepare_v2(database, insertSql, -1, &statement, nullptr);
  if (sqlReturn != SQLITE_OK) {
    LOG(FATAL) << "Prepare error: " << sqlite3_errmsg(database);
  }

  // Pull date elements out of chrono
  std::chrono::year_month_day scanDate       = foodItem.getScanDate();
  std::chrono::year_month_day expirationDate = foodItem.getExpirationDate();
  int scanDateYear                           = static_cast<int>(scanDate.year());
  unsigned int scanDateMonth                 = static_cast<unsigned>(scanDate.month());
  unsigned int scanDateDay                   = static_cast<unsigned>(scanDate.day());
  int expirationDateYear                     = static_cast<int>(expirationDate.year());
  unsigned int expirationDateMonth = static_cast<unsigned>(expirationDate.month());
  unsigned int expirationDateDay   = static_cast<unsigned>(expirationDate.day());

  // Bind to query
  sqlite3_bind_text(statement, 1, foodItem.getName().c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(statement, 2, foodItem.categoryToString().c_str(), -1,
                    SQLITE_TRANSIENT);
  sqlite3_bind_int(statement, 3, scanDateYear);
  sqlite3_bind_int(statement, 4, scanDateMonth);
  sqlite3_bind_int(statement, 5, scanDateDay);
  sqlite3_bind_int(statement, 6, expirationDateYear);
  sqlite3_bind_int(statement, 7, expirationDateMonth);
  sqlite3_bind_int(statement, 8, expirationDateDay);
  sqlite3_bind_double(statement, 9, foodItem.getWeight());
  sqlite3_bind_int(statement, 10, foodItem.getQuantity());

  sqlReturn = sqlite3_step(statement);
  if (sqlReturn != SQLITE_DONE) {
    LOG(FATAL) << "Execution Error: " << sqlite3_errmsg(database);
  }

  int lastRowId = sqlite3_last_insert_rowid(database);

  sqlite3_finalize(statement);

  return lastRowId;
}

/**
 * When reading a food item from a database, this function provides an interface between
 * the columns in the database and the FoodItem type. It gives the ability to assin a food
 * item field from a column in the database.
 *
 * @param foodItem The food item to assign the field of
 * @param columnValue The value read out of the column
 * @param columnName The name of the column that columnValue was read from
 * @return None
 */
void setFoodItem(FoodItem& foodItem,
                 const std::string& columnValue,
                 const std::string& columnName) {
  std::chrono::year_month_day scanDate       = foodItem.getScanDate();
  std::chrono::year_month_day expirationDate = foodItem.getExpirationDate();
  if (columnName == "id") {
    foodItem.setId(stoi(columnValue));
  }
  if (columnName == "name") {
    foodItem.setName(columnValue);
  }
  else if (columnName == "category") {
    foodItem.setCategory(foodCategoryFromString(columnValue));
  }
  else if (columnName == "scanDateYear") {
    foodItem.setScanDate(std::chrono::year_month_day(std::chrono::year{stoi(columnValue)},
                                                     scanDate.month(), scanDate.day()));
  }
  else if (columnName == "scanDateMonth") {
    foodItem.setScanDate(std::chrono::year_month_day(
        scanDate.year(),
        std::chrono::month{static_cast<unsigned>(std::stoi(columnValue))},
        scanDate.day()));
  }
  else if (columnName == "scanDateDay") {
    foodItem.setScanDate(std::chrono::year_month_day(
        scanDate.year(), scanDate.month(),
        std::chrono::day{static_cast<unsigned>(stoi(columnValue))}));
  }
  else if (columnName == "expirationDateYear") {
    foodItem.setExpirationDate(
        std::chrono::year_month_day(std::chrono::year{stoi(columnValue)},
                                    expirationDate.month(), expirationDate.day()));
  }
  else if (columnName == "expirationDateMonth") {
    foodItem.setExpirationDate(std::chrono::year_month_day(
        expirationDate.year(),
        std::chrono::month{static_cast<unsigned>(stoi(columnValue))},
        expirationDate.day()));
  }
  else if (columnName == "expirationDateDay") {
    foodItem.setExpirationDate(std::chrono::year_month_day(
        expirationDate.year(), expirationDate.month(),
        std::chrono::day{static_cast<unsigned>(stoi(columnValue))}));
  }
  else if (columnName == "weight") {
    foodItem.setWeight(stof(columnValue));
  }
  else if (columnName == "quantity") {
    foodItem.setQuantity(stoi(columnValue));
  }
}

/**
 * Read all food items currently stored in the database.
 *
 * @param None
 * @return Vector of food item objects, each corresponding to a food item read from the
 * database.
 */
std::vector<FoodItem> readAllFoodItems() {
  sqlite3* database = nullptr;
  openDatabase(&database);

  char* errorMessage    = nullptr;
  const char* selectAll = "SELECT * FROM foodItems;";

  std::vector<FoodItem> allFoodItems;
  sqlite3_exec(database, selectAll, readAllFoodItemsCallback, &allFoodItems,
               &errorMessage);

  sqlite3_close(database);
  return allFoodItems;
}

/**
 * Callback function for reading a food item by its id.
 *
 * @param foodItemVector Vector to store read food items into
 * @param numColumns How many columns in the table
 * @param columns The values stored in the columns
 * @param columnNames The names of the columns
 * @return Always 0
 */
int readAllFoodItemsCallback(void* foodItemVector,
                             int numColumns,
                             char** columns,
                             char** columnNames) {
  std::vector<FoodItem>* allFoodItems =
      static_cast<std::vector<FoodItem>*>(foodItemVector);
  struct FoodItem foodItem;

  for (int i = 0; i < numColumns; i++) {
    std::string columnValue(columns[i], strlen(columns[i]));
    std::string columnName(columnNames[i], strlen(columnNames[i]));
    setFoodItem(foodItem, columnValue, columnName);
  }
  allFoodItems->push_back(foodItem);
  return 0;
}

/**
 * Read all food items currently stored in the database, sorted by date.
 *
 * @param sortMethod The method by which the read items should be sorted
 * @return Vector of food item objects, each corresponding to a food item read from the
 * database.
 */
std::vector<FoodItem> readAllFoodItemsSorted(const SortMethod& sortMethod) {
  std::string sqlSortMethod;
  switch (sortMethod) {
  case SortMethod::LOW_TO_HIGH:
    sqlSortMethod = "ASC";
    break;
  case SortMethod::HIGH_TO_LOW:
    sqlSortMethod = "DESC";
    break;
  default:
    LOG(FATAL) << "Invalid sort method";
  }

  std::string readSortedQuery = "SELECT * FROM foodItems ORDER BY expirationDateYear " +
                                sqlSortMethod + ", expirationDateMonth " + sqlSortMethod +
                                ", expirationDateDay " + sqlSortMethod + ";";

  sqlite3* database = nullptr;
  openDatabase(&database);

  char* errorMessage = nullptr;

  std::vector<FoodItem> allFoodItems;
  sqlite3_exec(database, readSortedQuery.c_str(), readAllFoodItemsCallback, &allFoodItems,
               &errorMessage);

  sqlite3_close(database);
  return allFoodItems;
}

/**
 * Read a food item from the database by its id. Since the ID is the primary key in the
 * database, only one food item will be returned.
 *
 * @param id The id of the food item to be read
 * database.
 * @return The food item with the matching id
 */
FoodItem readFoodItemById(const int id) {
  sqlite3* database = nullptr;
  openDatabase(&database);

  char* errorMessage = nullptr;

  std::stringstream selectId;
  selectId << "SELECT * FROM foodItems WHERE id = " << id << ";";

  FoodItem foodItem;
  sqlite3_exec(database, selectId.str().c_str(), readFoodItemByIdCallback, &foodItem,
               &errorMessage);

  sqlite3_close(database);
  return foodItem;
}

/**
 * Callback function for reading a food item by its id.
 *
 * @param passedFoodItem The food item to put the read out data into
 * @param numColumns How many columns in the table
 * @param columns The values stored in the columns
 * @param columnNames The names of the columns
 * @return Always 0
 */
int readFoodItemByIdCallback(void* passedFoodItem,
                             int numColumns,
                             char** columns,
                             char** columnNames) {
  FoodItem* foodItem = static_cast<FoodItem*>(passedFoodItem);

  for (int i = 0; i < numColumns; i++) {
    std::string columnValue(columns[i], strlen(columns[i]));
    std::string columnName(columnNames[i], strlen(columnNames[i]));
    setFoodItem(*foodItem, columnValue, columnName);
  }
  return 0;
}

/**
 * Update the quantity of a food item.
 *
 * @param id The id of the food item to update.
 * @param newQuantity The quantity to switch to.
 */
void updateFoodItemQuantity(const int id, const int newQuantity) {
  sqlite3* database = nullptr;
  openDatabase(&database);
  char* errorMessage = nullptr;

  std::stringstream updateId;
  updateId << "UPDATE foodItems SET quantity = " << newQuantity << " WHERE id = " << id
           << ";";

  int sqlReturn =
      sqlite3_exec(database, updateId.str().c_str(), nullptr, nullptr, &errorMessage);
  if (sqlReturn != SQLITE_OK) {
    LOG(FATAL) << "Error updating food item";
  }

  sqlite3_close(database);
}

/**
 * Delete a food item from the database.
 *
 * @param id The id of the food item to delete.
 * @return None
 */
void deleteById(const int id) {
  sqlite3* database = nullptr;
  openDatabase(&database);
  char* errorMessage = nullptr;

  std::string deleteQuery =
      "DELETE FROM foodItems WHERE id = " + std::to_string(id) + ";";

  int sqlReturn =
      sqlite3_exec(database, deleteQuery.c_str(), nullptr, nullptr, &errorMessage);
  if (sqlReturn != SQLITE_OK) {
    LOG(FATAL) << "Error deleting food item";
  }

  sqlite3_close(database);
}
