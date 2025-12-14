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
    const std::string insert_sql = "INSERT INTO products (name, price, stock) VALUES ('" +
        std::string(name) + "'," + std::to_string(price) + "," + std::to_string(stock) + ");";
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

Product query_product(const std::string& name)
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

std::vector<Product> get_all_products()
{
    std::vector<Product> products;
    const char* sql = "SELECT * FROM products;";
    
    if (sqlite3_exec(db, sql, 
                     [](void* data, int argc, char** argv, char** col_name) -> int 
                     {
                         auto* products_ptr = static_cast<std::vector<Product>*>(data);
                         Product product;
                         product.id = std::stoi(argv[0]);
                         product.name = argv[1];
                         product.price = std::stof(argv[2]);
                         product.stock = std::stoi(argv[3]);
                         products_ptr->push_back(product);
                         return 0;
                     }, &products, &err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "查询所有商品失败: %s\n", err_msg);
        sqlite3_free(err_msg);
        return products;
    }
    
    return products;
}

bool save_transaction(const Transaction& transaction)
{
    // 开启事务
    if (sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, &err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "开启事务失败: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }
    
    // 插入交易记录
    char insert_transaction_sql[512];
    snprintf(insert_transaction_sql, sizeof(insert_transaction_sql),
             "INSERT INTO transactions (create_time, is_paid, total_price, amount_paid, change) VALUES (%ld, %d, %.2f, %.2f, %.2f);",
             transaction.create_time, transaction.is_paid, transaction.total_price,
             transaction.amount_paid, transaction.change);
    
    if (sqlite3_exec(db, insert_transaction_sql, nullptr, nullptr, &err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "插入交易记录失败: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_exec(db, "ROLLBACK TRANSACTION;", nullptr, nullptr, nullptr);
        return false;
    }
    
    // 获取生成的transaction_id
    int transaction_id = sqlite3_last_insert_rowid(db);
    
    // 插入购物车项
    for (const auto& item : transaction.cart.items)
    {
        char insert_cart_item_sql[512];
        snprintf(insert_cart_item_sql, sizeof(insert_cart_item_sql),
                 "INSERT INTO cart_items (transaction_id, product_id, quantity, subtotal) VALUES (%d, %d, %d, %.2f);",
                 transaction_id, item.product.id, item.quantity, item.subtotal);
        
        if (sqlite3_exec(db, insert_cart_item_sql, nullptr, nullptr, &err_msg) != SQLITE_OK)
        {
            fprintf(stderr, "插入购物车项失败: %s\n", err_msg);
            sqlite3_free(err_msg);
            sqlite3_exec(db, "ROLLBACK TRANSACTION;", nullptr, nullptr, nullptr);
            return false;
        }
        
        // 更新商品库存
        int new_stock = item.product.stock - item.quantity;
        if (!update_stock(item.product.id, new_stock))
        {
            fprintf(stderr, "更新商品库存失败，商品ID: %d\n", item.product.id);
            sqlite3_exec(db, "ROLLBACK TRANSACTION;", nullptr, nullptr, nullptr);
            return false;
        }
    }
    
    // 提交事务
    if (sqlite3_exec(db, "COMMIT TRANSACTION;", nullptr, nullptr, &err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "提交事务失败: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_exec(db, "ROLLBACK TRANSACTION;", nullptr, nullptr, nullptr);
        return false;
    }
    
    printf("交易记录保存成功，交易ID: %d\n", transaction_id);
    return true;
}

std::vector<Transaction> get_all_transactions()
{
    std::vector<Transaction> transactions;
    const char* sql = "SELECT * FROM transactions ORDER BY create_time DESC;";
    
    if (sqlite3_exec(db, sql, 
                     [](void* data, int argc, char** argv, char** col_name) -> int 
                     {   
                         auto* transactions_ptr = static_cast<std::vector<Transaction>*>(data);
                         Transaction transaction;
                         transaction.transaction_id = std::stoi(argv[0]);
                         transaction.create_time = std::stol(argv[1]);
                         transaction.is_paid = std::stoi(argv[2]) != 0;
                         transaction.total_price = std::stof(argv[3]);
                         transaction.amount_paid = std::stof(argv[4]);
                         transaction.change = std::stof(argv[5]);
                         transactions_ptr->push_back(transaction);
                         return 0;
                     }, &transactions, &err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "查询所有交易记录失败: %s\n", err_msg);
        sqlite3_free(err_msg);
        return transactions;
    }
    
    return transactions;
}

std::vector<CartItem> get_cart_items_by_transaction_id(const int transaction_id)
{
    std::vector<CartItem> cart_items;
    const std::string sql = "SELECT ci.*, p.id, p.name, p.price, p.stock "
                           "FROM cart_items ci "
                           "JOIN products p ON ci.product_id = p.id "
                           "WHERE ci.transaction_id = " + std::to_string(transaction_id) + ";";
    
    if (sqlite3_exec(db, sql.c_str(), 
                     [](void* data, int argc, char** argv, char** col_name) -> int 
                     {   
                         auto* cart_items_ptr = static_cast<std::vector<CartItem>*>(data);
                         CartItem cart_item;
                         
                         // cart_items表的列：item_id, transaction_id, product_id, quantity, subtotal (5列)
                         // 所以商品信息从索引5开始
                         Product product;
                         product.id = std::stoi(argv[5]);
                         product.name = argv[6];
                         product.price = std::stof(argv[7]);
                         product.stock = std::stoi(argv[8]);
                         
                         cart_item.product = product;
                         cart_item.quantity = std::stoi(argv[3]);
                         cart_item.subtotal = std::stof(argv[4]);
                         
                         cart_items_ptr->push_back(cart_item);
                         return 0;
                     }, &cart_items, &err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "查询购物车项失败: %s\n", err_msg);
        sqlite3_free(err_msg);
        return cart_items;
    }
    
    return cart_items;
}

std::vector<Product> get_low_stock_products(const int threshold)
{
    std::vector<Product> low_stock_products;
    const std::string sql = "SELECT * FROM products WHERE stock <= " + std::to_string(threshold) + ";";
    
    if (sqlite3_exec(db, sql.c_str(), 
                     [](void* data, int argc, char** argv, char** col_name) -> int 
                     {   
                         auto* products_ptr = static_cast<std::vector<Product>*>(data);
                         Product product;
                         product.id = std::stoi(argv[0]);
                         product.name = argv[1];
                         product.price = std::stof(argv[2]);
                         product.stock = std::stoi(argv[3]);
                         products_ptr->push_back(product);
                         return 0;
                     }, &low_stock_products, &err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "查询低库存商品失败: %s\n", err_msg);
        sqlite3_free(err_msg);
        return low_stock_products;
    }
    
    return low_stock_products;
}

bool delete_product(const int id)
{
    const std::string delete_sql = "DELETE FROM products WHERE id = " + std::to_string(id) + ";";
    if (sqlite3_exec(db, delete_sql.c_str(), nullptr, nullptr, &err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "删除商品失败: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }
    
    // 检查是否有记录被删除
    int changes = sqlite3_changes(db);
    if (changes == 0)
    {
        fprintf(stderr, "未找到ID为 %d 的商品\n", id);
        return false;
    }
    
    printf("商品ID %d 删除成功\n", id);
    return true;
}

bool delete_product(const std::string& name)
{
    const std::string delete_sql = "DELETE FROM products WHERE name = '" + name + "';";
    if (sqlite3_exec(db, delete_sql.c_str(), nullptr, nullptr, &err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "删除商品失败: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }
    
    // 检查是否有记录被删除
    int changes = sqlite3_changes(db);
    if (changes == 0)
    {
        fprintf(stderr, "未找到名称为 '%s' 的商品\n", name.c_str());
        return false;
    }
    
    printf("商品 '%s' 删除成功\n", name.c_str());
    return true;
}