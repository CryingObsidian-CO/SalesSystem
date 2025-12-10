#include <vector>
#include "saleStruct.h"


//增加商品//
void add_product(ShoppingCart &cart, const int id, const int quantity)
{
    CartItem item =  CartItem();
    item.quantity=quantity;
    item.subtotal=get_price_from_id(id) * quantity;
    cart.items.push_back(item);
    cart.total_price += item.subtotal;
    item.product.stock-=quantity;
}
//修改购物车里商品数量//
void change_number(ShoppingCart &cart,int number, const int id){
    auto it = findProductFromId(cart.items,id);
    it->product.stock=it->product.stock-number+it->quantity;
    cart.total_price=cart.total_price+it->product.price * (number-it->quantity);
    it->quantity=number;
}
//在购物车里移除商品//
void remove_produt(ShoppingCart &cart, const int id){
    const auto it = findProductFromId(cart.items,id);
    cart.total_price-=it->subtotal;
    cart.items.erase(it);
}
