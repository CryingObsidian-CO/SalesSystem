#include "database.h"
#include <cstdio>
#include <ctime>
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
        "stock INTEGER NOT NULL,"
        "alert_threshold INTEGER DEFAULT 10 NOT NULL"
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
        "returned_quantity INTEGER NOT NULL DEFAULT 0 CHECK(returned_quantity >= 0),"
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
    
    // 为现有数据库添加returned_quantity列
    const char* sql_alter_cart_items = 
        "ALTER TABLE cart_items ADD COLUMN IF NOT EXISTS returned_quantity INTEGER NOT NULL DEFAULT 0;";
    rc = sqlite3_exec(db, sql_alter_cart_items, nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "修改cart_items表失败: %s\n", err_msg);
        sqlite3_free(err_msg);
        // 继续执行，不中断初始化
    }
    
    // 创建退货表
    const char* sql_create_returns =
        "CREATE TABLE IF NOT EXISTS returns ("
        "return_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "transaction_id INTEGER NOT NULL,"
        "product_id INTEGER NOT NULL,"
        "quantity INTEGER NOT NULL CHECK(quantity > 0),"
        "reason TEXT,"
        "return_time INTEGER NOT NULL,"
        "FOREIGN KEY(transaction_id) REFERENCES transactions(transaction_id),"
        "FOREIGN KEY(product_id) REFERENCES products(id)"
        ");";
    rc = sqlite3_exec(db, sql_create_returns, nullptr, nullptr, &err_msg);
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

bool add_product(const std::string& name, const double price, const int stock, int alert_threshold)
{
    // 使用sprintf确保小数点分隔符是点，而非逗号
    char sql_buffer[512];
    snprintf(sql_buffer, sizeof(sql_buffer), 
             "INSERT INTO products (name, price, stock, alert_threshold) VALUES ('%s', %.2f, %d, %d);",
             name.c_str(), price, stock, alert_threshold);
    
    const std::string insert_sql(sql_buffer);
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
                         product_ptr->id = std::stoi(argv[0]);
                         product_ptr->name = argv[1];
                         product_ptr->price = std::stof(argv[2]);
                         product_ptr->stock = std::stoi(argv[3]);
                         // 注意：alert_threshold字段存在于数据库中，但Product结构体中没有对应字段
                         // 这里忽略该字段，因为我们会通过专门的函数获取预警阈值
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
                         // 注意：alert_threshold字段存在于数据库中，但Product结构体中没有对应字段
                         // 这里忽略该字段，因为我们会通过专门的函数获取预警阈值
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
                         
                         // cart_items表的列：item_id, transaction_id, product_id, quantity, returned_quantity, subtotal (6列)
                         // 所以商品信息从索引6开始
                         Product product;
                         product.id = std::stoi(argv[6]);
                         product.name = argv[7];
                         product.price = std::stof(argv[8]);
                         product.stock = std::stoi(argv[9]);
                         
                         cart_item.product = product;
                         cart_item.quantity = std::stoi(argv[3]);
                         cart_item.returned_quantity = std::stoi(argv[4]);
                         cart_item.subtotal = std::stof(argv[5]);
                         
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

std::vector<Product> get_low_stock_products()
{
    std::vector<Product> low_stock_products;
    // 查询库存低于或等于其预警阈值的商品
    const char* sql = "SELECT * FROM products WHERE stock <= alert_threshold;";
    
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

bool update_product(int id, const std::string& name, double price, int stock, int alert_threshold, std::string* errorMsg)
{
    // 先查询商品是否存在
    Product existingProduct = query_product(id);
    if (existingProduct.id == -1) {
        std::string err = "更新商品失败: 未找到ID为 " + std::to_string(id) + " 的商品";
        fprintf(stderr, "%s\n", err.c_str());
        if (errorMsg) *errorMsg = err;
        return false;
    }
    
    // 使用sprintf确保小数点分隔符是点，而非逗号
    char sql_buffer[512];
    snprintf(sql_buffer, sizeof(sql_buffer), 
             "UPDATE products SET name = '%s', price = %.2f, stock = %d, alert_threshold = %d WHERE id = %d;",
             name.c_str(), price, stock, alert_threshold, id);
    
    const std::string update_sql(sql_buffer);
    printf("执行SQL: %s\n", update_sql.c_str());
    
    // 执行SQL语句
    int rc = sqlite3_exec(db, update_sql.c_str(), nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK)
    {
        std::string err = "更新商品失败: " + std::string(err_msg);
        fprintf(stderr, "%s\n", err.c_str());
        sqlite3_free(err_msg);
        if (errorMsg) *errorMsg = err;
        return false;
    }
    
    // 检查是否有记录被更新
    int changes = sqlite3_changes(db);
    printf("SQL执行影响的行数: %d\n", changes);
    
    if (changes == 0)
    {
        // 没有记录被更新，可能是因为所有字段值都没有变化
        fprintf(stderr, "更新商品提示: ID为 %d 的商品没有任何字段值变化\n", id);
        // 返回true，因为商品信息已经是最新的
        return true;
    }
    
    printf("商品ID %d 更新成功\n", id);
    return true;
}

bool update_product(const std::string& old_name, const std::string& new_name, double price, int stock, int alert_threshold, std::string* errorMsg)
{
    // 先查询商品是否存在
    int productId = getIdFromName(old_name);
    if (productId == -1) {
        std::string err = "更新商品失败: 未找到名称为 '" + old_name + "' 的商品";
        fprintf(stderr, "%s\n", err.c_str());
        if (errorMsg) *errorMsg = err;
        return false;
    }
    
    // 调用第一个update_product函数来执行更新
    return update_product(productId, new_name, price, stock, alert_threshold, errorMsg);
}

bool set_product_alert_threshold(int id, int threshold)
{
    // 先查询商品是否存在
    Product existingProduct = query_product(id);
    if (existingProduct.id == -1) {
        fprintf(stderr, "更新商品预警阈值失败: 未找到ID为 %d 的商品\n", id);
        return false;
    }
    
    // 使用sprintf确保SQL语句格式正确
    char sql_buffer[512];
    snprintf(sql_buffer, sizeof(sql_buffer), 
             "UPDATE products SET alert_threshold = %d WHERE id = %d;",
             threshold, id);
    
    const std::string update_sql(sql_buffer);
    
    if (sqlite3_exec(db, update_sql.c_str(), nullptr, nullptr, &err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "更新商品预警阈值失败: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }
    
    // 检查是否有记录被更新
    int changes = sqlite3_changes(db);
    if (changes == 0)
    {
        // 没有记录被更新，可能是因为预警阈值没有变化
        fprintf(stderr, "更新商品预警阈值提示: ID为 %d 的商品预警阈值没有变化\n", id);
        // 返回true，因为预警阈值已经是最新的
        return true;
    }
    
    printf("商品ID %d 预警阈值更新成功\n", id);
    return true;
}

bool set_product_alert_threshold(const std::string& name, int threshold)
{
    // 先通过名称获取商品ID
    int productId = getIdFromName(name);
    if (productId == -1) {
        fprintf(stderr, "更新商品预警阈值失败: 未找到名称为 '%s' 的商品\n", name.c_str());
        return false;
    }
    
    // 使用ID来更新，更可靠
    return set_product_alert_threshold(productId, threshold);
}

int get_product_alert_threshold(int id)
{
    int threshold = -1;
    const std::string sql = "SELECT alert_threshold FROM products WHERE id = " + std::to_string(id) + ";";
    
    if (sqlite3_exec(db, sql.c_str(),
                     [](void* data, int argc, char** argv, char** col_name) -> int
                     {
                         int* threshold_ptr = static_cast<int*>(data);
                         *threshold_ptr = std::stoi(argv[0]);
                         return 0;
                     }, &threshold, &err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "查询商品预警阈值失败: %s\n", err_msg);
        sqlite3_free(err_msg);
        return -1;
    }
    
    return threshold;
}

int get_product_alert_threshold(const std::string& name)
{
    int id = getIdFromName(name);
    if (id == -1)
    {
        return -1;
    }
    
    return get_product_alert_threshold(id);
}

// 退货相关函数实现

bool add_return(int transaction_id, int product_id, int quantity, const std::string& reason)
{
    // 开启事务
    if (sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, &err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "开启事务失败: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }
    
    // 1. 查询当前购物车项的已退货数量
    char select_cart_item_sql[512];
    snprintf(select_cart_item_sql, sizeof(select_cart_item_sql),
             "SELECT item_id, returned_quantity FROM cart_items WHERE transaction_id = %d AND product_id = %d;",
             transaction_id, product_id);
    
    int results[2] = {-1, 0}; // [0] = cart_item_id, [1] = current_returned
    
    if (sqlite3_exec(db, select_cart_item_sql, 
                     [](void* data, int argc, char** argv, char** col_name) -> int 
                     {
                         int* results = static_cast<int*>(data);
                         results[0] = std::stoi(argv[0]); // item_id
                         results[1] = std::stoi(argv[1]); // returned_quantity
                         return 0;
                     }, results, &err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "查询购物车项失败: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_exec(db, "ROLLBACK TRANSACTION;", nullptr, nullptr, nullptr);
        return false;
    }
    
    int cart_item_id = results[0];
    int current_returned = results[1];
    
    if (cart_item_id == -1)
    {
        fprintf(stderr, "购物车项不存在\n");
        sqlite3_exec(db, "ROLLBACK TRANSACTION;", nullptr, nullptr, nullptr);
        return false;
    }
    
    // 2. 添加退货记录
    char insert_return_sql[512];
    time_t now = time(nullptr);
    snprintf(insert_return_sql, sizeof(insert_return_sql),
             "INSERT INTO returns (transaction_id, product_id, quantity, reason, return_time) VALUES (%d, %d, %d, '%s', %ld);",
             transaction_id, product_id, quantity, reason.c_str(), now);
    
    if (sqlite3_exec(db, insert_return_sql, nullptr, nullptr, &err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "插入退货记录失败: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_exec(db, "ROLLBACK TRANSACTION;", nullptr, nullptr, nullptr);
        return false;
    }
    
    // 3. 更新购物车项的已退货数量
    int new_returned = current_returned + quantity;
    char update_cart_item_sql[512];
    snprintf(update_cart_item_sql, sizeof(update_cart_item_sql),
             "UPDATE cart_items SET returned_quantity = %d WHERE item_id = %d;",
             new_returned, cart_item_id);
    
    if (sqlite3_exec(db, update_cart_item_sql, nullptr, nullptr, &err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "更新购物车项退货数量失败: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_exec(db, "ROLLBACK TRANSACTION;", nullptr, nullptr, nullptr);
        return false;
    }
    
    // 4. 查询当前商品库存
    Product product = query_product(product_id);
    if (product.id == -1)
    {
        fprintf(stderr, "查询商品失败: 未找到ID为 %d 的商品\n", product_id);
        sqlite3_exec(db, "ROLLBACK TRANSACTION;", nullptr, nullptr, nullptr);
        return false;
    }
    
    // 5. 更新商品库存（增加退货数量）
    int newStock = product.stock + quantity;
    if (!update_stock(product_id, newStock))
    {
        fprintf(stderr, "更新商品库存失败，商品ID: %d\n", product_id);
        sqlite3_exec(db, "ROLLBACK TRANSACTION;", nullptr, nullptr, nullptr);
        return false;
    }
    
    // 6. 更新交易总金额
    // 计算退货金额
    float returnAmount = product.price * quantity;
    
    // 查询当前交易信息
    char select_transaction_sql[512];
    snprintf(select_transaction_sql, sizeof(select_transaction_sql),
             "SELECT total_price, amount_paid, change FROM transactions WHERE transaction_id = %d;",
             transaction_id);
    
    float currentTotal = 0.0f;
    float currentPaid = 0.0f;
    float currentChange = 0.0f;
    
    if (sqlite3_exec(db, select_transaction_sql, 
                     [](void* data, int argc, char** argv, char** col_name) -> int 
                     {
                         float* results = static_cast<float*>(data);
                         results[0] = std::stof(argv[0]); // total_price
                         results[1] = std::stof(argv[1]); // amount_paid
                         results[2] = std::stof(argv[2]); // change
                         return 0;
                     }, &currentTotal, &err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "查询交易信息失败: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_exec(db, "ROLLBACK TRANSACTION;", nullptr, nullptr, nullptr);
        return false;
    }
    
    // 计算新的总金额、支付金额和找零
    float newTotal = currentTotal - returnAmount;
    // 支付金额和找零保持不变，因为这是实际的支付情况
    // 只有总金额需要调整为扣除退货后的金额
    
    // 更新交易记录
    char update_transaction_sql[512];
    snprintf(update_transaction_sql, sizeof(update_transaction_sql),
             "UPDATE transactions SET total_price = %.2f WHERE transaction_id = %d;",
             newTotal, transaction_id);
    
    if (sqlite3_exec(db, update_transaction_sql, nullptr, nullptr, &err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "更新交易总金额失败: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_exec(db, "ROLLBACK TRANSACTION;", nullptr, nullptr, nullptr);
        return false;
    }
    
    // 提交事务
    if (sqlite3_exec(db, "COMMIT TRANSACTION;", nullptr, nullptr, &err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "提交事务失败: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_exec(db, "ROLLBACK TRANSACTION;", nullptr, nullptr, nullptr);
        return false;
    }
    
    printf("退货记录添加成功，交易ID: %d, 商品ID: %d, 数量: %d, 退货金额: %.2f\n", 
           transaction_id, product_id, quantity, returnAmount);
    return true;
}

std::vector<ReturnItem> get_all_returns()
{
    std::vector<ReturnItem> returnItems;
    const char* sql = "SELECT * FROM returns ORDER BY return_time DESC;";
    
    if (sqlite3_exec(db, sql, 
                     [](void* data, int argc, char** argv, char** col_name) -> int 
                     {
                         auto* returns_ptr = static_cast<std::vector<ReturnItem>*>(data);
                         ReturnItem return_item;
                         return_item.return_id = std::stoi(argv[0]);
                         return_item.transaction_id = std::stoi(argv[1]);
                         return_item.product_id = std::stoi(argv[2]);
                         return_item.quantity = std::stoi(argv[3]);
                         return_item.reason = argv[4] ? argv[4] : "";
                         return_item.return_time = std::stol(argv[5]);
                         returns_ptr->push_back(return_item);
                         return 0;
                     }, &returnItems, &err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "查询所有退货记录失败: %s\n", err_msg);
        sqlite3_free(err_msg);
        return returnItems;
    }
    
    return returnItems;
}

std::vector<ReturnItem> get_returns_by_transaction_id(int transaction_id)
{
    std::vector<ReturnItem> returns;
    const std::string sql = "SELECT * FROM returns WHERE transaction_id = " + std::to_string(transaction_id) + " ORDER BY return_time DESC;";
    
    if (sqlite3_exec(db, sql.c_str(), 
                     [](void* data, int argc, char** argv, char** col_name) -> int 
                     {   
                         auto* returns_ptr = static_cast<std::vector<ReturnItem>*>(data);
                         ReturnItem return_item;
                         return_item.return_id = std::stoi(argv[0]);
                         return_item.transaction_id = std::stoi(argv[1]);
                         return_item.product_id = std::stoi(argv[2]);
                         return_item.quantity = std::stoi(argv[3]);
                         return_item.reason = argv[4] ? argv[4] : "";
                         return_item.return_time = std::stol(argv[5]);
                         returns_ptr->push_back(return_item);
                         return 0;
                     }, &returns, &err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "查询交易退货记录失败: %s\n", err_msg);
        sqlite3_free(err_msg);
        return returns;
    }
    
    return returns;
}

std::vector<ReturnItem> get_returns_by_product_id(int product_id)
{
    std::vector<ReturnItem> returns;
    const std::string sql = "SELECT * FROM returns WHERE product_id = " + std::to_string(product_id) + " ORDER BY return_time DESC;";
    
    if (sqlite3_exec(db, sql.c_str(), 
                     [](void* data, int argc, char** argv, char** col_name) -> int 
                     {   
                         auto* returns_ptr = static_cast<std::vector<ReturnItem>*>(data);
                         ReturnItem return_item;
                         return_item.return_id = std::stoi(argv[0]);
                         return_item.transaction_id = std::stoi(argv[1]);
                         return_item.product_id = std::stoi(argv[2]);
                         return_item.quantity = std::stoi(argv[3]);
                         return_item.reason = argv[4] ? argv[4] : "";
                         return_item.return_time = std::stol(argv[5]);
                         returns_ptr->push_back(return_item);
                         return 0;
                     }, &returns, &err_msg) != SQLITE_OK)
    {
        fprintf(stderr, "查询商品退货记录失败: %s\n", err_msg);
        sqlite3_free(err_msg);
        return returns;
    }
    
    return returns;
}