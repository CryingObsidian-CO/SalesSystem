#ifndef DATABASE_H
#define DATABASE_H
#include <string>

bool init_db();
int getIdFromName(const std::string& name);
bool add_product(const std::string& name, const double price, const int stock);
bool query_product(const int id);
bool update_stock(const int id, const int new_stock);
int update_stock(const std::string& name, const int new_stock);


#endif // DATABASE_H
