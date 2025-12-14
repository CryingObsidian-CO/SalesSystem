#ifndef SALE_STRUCT_H
#define SALE_STRUCT_H

#include <vector>
#include <string>



/* ========== 1. 定义商品结构体 ========== */
typedef struct {
    int id;             // 商品编号
    std::string name;        // 商品名称
    float price;        // 商品单价
    int stock;          // 商品库存
} Product;

/* ========== 2. 定义购物车项结构体 ========== */
typedef struct {
    Product product;    // 商品信息
    int quantity;       // 购买数量
    float subtotal;     // 小计金额 = price * quantity
} CartItem;

/* ========== 3. 定义购物车结构体 ========== */
typedef struct {
    std::vector<CartItem> items; // 购物车最多放20种商品
    float total_price;  // 购物车总金额
} ShoppingCart;

/* ========== 4. 定义交易结构体 ========== */
typedef struct {
    int transaction_id;     // 交易编号
    ShoppingCart cart;      // 本次交易的购物车
    time_t create_time;     // 交易创建时间
    bool is_paid;           // 是否已支付：false未付，true已付
    float total_price;      // 交易总金额
    float amount_paid;      // 实际支付金额
    float change;           // 找零金额
} Transaction;


#endif // SALE_STRUCT_H
