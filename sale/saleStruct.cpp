#include "saleStruct.h"

#include <algorithm>
#include <stdexcept>

auto findProductFromId(std::vector<CartItem>& items, int id)-> decltype(items.begin()){
    const auto it = std::ranges::find_if(items, [id](const CartItem& item) {
        return item.product.id == id;
    });
    if (it != items.end()) {
        return it;
    }
    throw std::runtime_error("Product not found");
}

float get_price_from_id(int id){
    // 数据库查
    return 0.0;
}
