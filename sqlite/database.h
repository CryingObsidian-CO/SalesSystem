#ifndef DATABASE_H
#define DATABASE_H
#include <string>
#include "saleStruct.h"

bool init_db();
int getIdFromName(const std::string& name);
bool add_product(const std::string& name, double price, int stock, int alert_threshold = 10);
Product query_product(int id);
Product query_product(const std::string& name);
bool update_stock(int id, int new_stock);
int update_stock(const std::string& name, int new_stock);
std::vector<Product> get_all_products();
bool save_transaction(const Transaction& transaction);
std::vector<Transaction> get_all_transactions();
std::vector<CartItem> get_cart_items_by_transaction_id(int transaction_id);
std::vector<Product> get_low_stock_products();
bool delete_product(int id);
bool delete_product(const std::string& name);
bool update_product(int id, const std::string& name, double price, int stock, int alert_threshold = 10, std::string* errorMsg = nullptr);
bool update_product(const std::string& old_name, const std::string& new_name, double price, int stock, int alert_threshold = 10, std::string* errorMsg = nullptr);
bool set_product_alert_threshold(int id, int threshold);
bool set_product_alert_threshold(const std::string& name, int threshold);
int get_product_alert_threshold(int id);
int get_product_alert_threshold(const std::string& name);


#endif // DATABASE_H
