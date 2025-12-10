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
    int id = -1;
    const std::string sql = "SELECT id FROM products WHERE name = '" + name + "';";
    if (sqlite3_exec(db, sql.c_str(),
                     [](void* data, int argc, char** argv, char** col_name) -> int
                     {
                         int* id_ptr = static_cast<int*>(data);
                         for (int i = 0; i < argc; i++)
                         {
                             *id_ptr = std::stoi(argv[i]);
                         }
                         return 0;
                     }, &id, &err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "查询商品ID失败: %s\n", err_msg);
        sqlite3_free(err_msg);
        return -1;
    }

    return id;
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

Product query_product(const int id)
{
    Product product = {-1, "", 0.0f, 0};
    const std::string sql = "SELECT * FROM products WHERE id = " + std::to_string(id) + ";";
    if (sqlite3_exec(db, sql.c_str(),
                     [](void* data, int argc, char** argv, char** col_name) -> int
                     {
                         auto* product_ptr = static_cast<Product*>(data);
                         for (int i = 0; i < argc; i++)
                         {
                             product_ptr->id = std::stoi(argv[0]);
                             product_ptr->name = argv[1];
                             product_ptr->price = std::stof(argv[2]);
                             product_ptr->stock = std::stoi(argv[3]);
                         }
                         return 0;
                     }, &product, &err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "查询商品失败: %s\n", err_msg);
        sqlite3_free(err_msg);
        return product;
    }

    printf("商品查询完成\n");
    return product;
}

Product query_products(const std::string& name)
{
    return query_product(getIdFromName(name));
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
