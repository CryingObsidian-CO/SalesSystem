#ifndef DATABASE_H
#define DATABASE_H
#include <string>
#include "saleStruct.h"

bool init_db();
int getIdFromName(const std::string& name);
bool add_product(const std::string& name, double price, int stock);
Product query_product(int id);
Product query_product(const std::string& name);
bool update_stock(int id, int new_stock);
int update_stock(const std::string& name, int new_stock);
std::vector<Product> get_all_products();
bool save_transaction(const Transaction& transaction);


#endif // DATABASE_H
