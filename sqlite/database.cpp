#include "database.h"
#include <cstdio>
#include <sqlite3.h>


sqlite3* db;
char* err_msg;
sqlite3_stmt* stmt_ap;

bool init_db()
{
    int rc = sqlite3_open("sales.db", &db);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return false;
    }
    rc = sqlite3_exec(db, "PRAGMA foreign_keys = ON;", 0, 0, &err_msg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }
    const char* sql_create_products =
        "CREATE TABLE IF NOT EXISTS products ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "price REAL NOT NULL,"
        "stock INTEGER NOT NULL"
        ");";
    const char* sql_create_transactions =
        "CREATE TABLE IF NOT EXISTS transactions ("
        "transaction_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "create_time INTEGER NOT NULL,"
        "is_paid INTEGER NOT NULL CHECK(is_paid IN (0,1)),"
        "total_price REAL NOT NULL,"
        "amount_paid REAL NOT NULL,"
        "change REAL NOT NULL"
        ");";
    const char* sql_create_cart_items =
        "CREATE TABLE IF NOT EXISTS cart_items ("
        "item_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "transaction_id INTEGER NOT NULL,"
        "product_id INTEGER NOT NULL,"
        "quantity INTEGER NOT NULL CHECK(quantity > 0),"
        "subtotal REAL NOT NULL,"
        "FOREIGN KEY(transaction_id) REFERENCES transactions(transaction_id),"
        "FOREIGN KEY(product_id) REFERENCES products(id)"
        ");";
    rc = sqlite3_exec(db, sql_create_products, nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }
    rc = sqlite3_exec(db, sql_create_transactions, nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }
    rc = sqlite3_exec(db, sql_create_cart_items, nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }
    return true;
}

int getIdFromName(const std::string& name)
{
    const std::string sql = "SELECT id FROM products WHERE name = '" + name + "';";
    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "查询商品ID失败: %s\n", err_msg);
        sqlite3_free(err_msg);
        return -1;
    }

    return id;
}

int getTransactionIdFromCreateTime(const std::string& create_time)
{
    const std::string sql = "SELECT transaction_id FROM transactions WHERE create_time = '" +
        create_time + "';";
    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "查询交易ID失败: %s\n", err_msg);
        sqlite3_free(err_msg);
        return -1;
    }

    return transaction_id;
}

int getCartItemIdFromCreateTime(const std::string& create_time)
{
    const std::string sql = "SELECT item_id FROM cart_items WHERE transaction_id = (SELECT "
        "transaction_id FROM transactions WHERE create_time = '" + create_time + "');";
    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "查询购物车项ID失败: %s\n", err_msg);
        sqlite3_free(err_msg);
        return -1;
    }

    return item_id;
}

bool add_product(const std::string& name, const double price, const int stock)
{
    const std::string insert_sql = "INSERT INTO products (name, price, stock) VALUES (" +
        std::string(name) + "," + std::to_string(price) + "," + std::to_string(stock) + ");";
    if (sqlite3_exec(db, insert_sql.c_str(), nullptr, nullptr, &err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "插入商品失败: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }
    printf("商品%s添加成功: \n", name.c_str());
    return true;
}

bool add_transaction(const std::string& create_time, const bool is_paid,
                     const double total_price, const double amount_paid, const double change)
{
    const std::string insert_sql = "INSERT INTO transactions (create_time, is_paid, total_price, "
        "amount_paid, change) VALUES (" + create_time + "," + std::to_string(is_paid) + "," +
        std::to_string(total_price) + "," + std::to_string(amount_paid) + "," +
        std::to_string(change) + ");";
    if (sqlite3_exec(db, insert_sql.c_str(), nullptr, nullptr, &err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "插入交易记录失败: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }
    printf("交易记录添加成功\n");
    return true;
}

bool add_cart_item(const int transaction_id, const int product_id,
                   const int quantity, const double subtotal)
{
    const std::string insert_sql = "INSERT INTO cart_items (transaction_id, product_id, "
        "quantity, subtotal) VALUES (" + std::to_string(transaction_id) + "," +
        std::to_string(product_id) + "," + std::to_string(quantity) + "," +
        std::to_string(subtotal) + ");";
    if (sqlite3_exec(db, insert_sql.c_str(), nullptr, nullptr, &err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "插入购物车项失败: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }
    printf("购物车项添加成功\n");
    return true;
}

bool query_product(const int id)
{
    const std::string sql = "SELECT * FROM products WHERE id = " + std::to_string(id) + ";";
    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "查询商品失败: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }

    printf("商品查询完成\n");
    return true;
}

int query_products(const std::string& name)
{
    query_product(getIdFromName(name));
}

bool query_transaction(const int transaction_id)
{
    const std::string sql = "SELECT * FROM transactions WHERE transaction_id = " +
        std::to_string(transaction_id) + ";";
    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "查询交易记录失败: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }

    printf("交易记录查询完成\n");
    return true;
}

int query_transaction(const std::string& create_time)
{
    query_transaction(getIdFromName(create_time));
}

bool query_cart_items(const int transaction_id)
{
    const std::string sql = "SELECT * FROM cart_items WHERE transaction_id = " +
        std::to_string(transaction_id) + ";";
    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "查询购物车项失败: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }

    printf("购物车项查询完成\n");
    return true;
}

int query_cart_items(const std::string& create_time)
{
    query_cart_items(getIdFromName(create_time));
}

bool update_stock(const int id, const int new_stock)
{
    const std::string sql = "UPDATE products SET stock = " + std::to_string(new_stock) +
        " WHERE id = " + std::to_string(id) + ";";
    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "更新库存失败: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }
    printf("商品ID %d 库存更新为 %d 成功\n", id, new_stock);
    return true;
}

int update_stock(const std::string& name, const int new_stock)
{
    return update_stock(getIdFromName(name), new_stock);
}

bool update_transaction_payment(const int transaction_id, const bool is_paid,
                                const double amount_paid, const double change)
{
    const std::string sql = "UPDATE transactions SET is_paid = " + std::to_string(is_paid) +
        ", amount_paid = " + std::to_string(amount_paid) + ", change = " +
        std::to_string(change) + " WHERE transaction_id = " +
        std::to_string(transaction_id) + ";";
    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "更新交易支付信息失败: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }
    printf("交易ID %d 支付信息更新成功\n", transaction_id);
    return true;
}

int update_transaction_payment(const std::string& create_time, const bool is_paid,
                               const double amount_paid, const double change)
{
    return update_transaction_payment(getTransactionIdFromCreateTime(create_time), is_paid,
                                      amount_paid, change);
}

bool update_cart_item_quantity(const int item_id, const int new_quantity, const double new_subtotal)
{
    const std::string sql = "UPDATE cart_items SET quantity = " + std::to_string(new_quantity) +
        ", subtotal = " + std::to_string(new_subtotal) + " WHERE item_id = " +
        std::to_string(item_id) + ";";
    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "更新购物车项失败: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }
    printf("购物车项ID %d 更新成功\n", item_id);
    return true;
}

int update_cart_item_quantity(const std::string& create_time, const int new_quantity,
                              const double new_subtotal)
{
    return update_cart_item_quantity(getCartItemIdFromCreateTime(create_time), new_quantity, new_subtotal);
}
