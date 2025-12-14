#include "saleStruct.h"
#include <ctime>

// 创建新交易
Transaction createNewTransaction(const ShoppingCart& cart)
{
    // 实现创建新交易的逻辑
    Transaction transaction;
    transaction.transaction_id = 0; // uuid
    transaction.cart = cart;
    transaction.create_time = time(nullptr); // time
    transaction.is_paid = false;
    transaction.total_price = cart.total_price;
    transaction.amount_paid = 0.0f;
    transaction.change = 0.0f;
    return transaction;
}

// 完成交易
Transaction completeTransaction(Transaction& transaction, const float amountPaid)
{
    // 实现完成交易的逻辑
    transaction.amount_paid = amountPaid;
    transaction.total_price = transaction.cart.total_price;
    transaction.change = amountPaid - transaction.total_price;
    transaction.is_paid = true;

    // 这里需要更新库存，后续会调用库存管理模块的函数

    return transaction;
}


// 获取销售记录列表
std::vector<Transaction> getSalesRecords()
{
    // 返回销售记录列表，数据库交互部分暂不实现
    std::vector<Transaction> transactions;
    return transactions;
}

// 根据时间范围获取销售记录
std::vector<Transaction> getSalesRecordsByTimeRange(time_t startTime, time_t endTime)
{
    // 实现按时间范围查询销售记录，数据库交互部分暂不实现
    std::vector<Transaction> transactions;
    return transactions;
}

// // 添加商品到购物车
// bool addItemToCart(ShoppingCart& cart, const Product& product, int quantity) {
//     // 实现添加商品到购物车的逻辑
//     if (cart.item_count >= 20) {
//         return false; // 购物车已满
//     }
//
//     CartItem item;
//     item.product = product;
//     item.quantity = quantity;
//     item.subtotal = product.price * quantity;
//
//     cart.items.push_back(item);
//     cart.item_count++;
//     cart.total_price += item.subtotal;
//
//     return true;
// }
