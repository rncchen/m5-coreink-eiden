#include "station_nav.h"

namespace railboard {

StationNav::StationNav(std::vector<std::string> nav_order, const std::string& start_id)
    : order_(std::move(nav_order)), index_(0) {
    jumpTo(start_id);
}

const std::string& StationNav::current() const {
    return order_[index_];
}

void StationNav::next() {
    index_ = (index_ + 1) % order_.size();
}

void StationNav::prev() {
    index_ = (index_ == 0) ? order_.size() - 1 : index_ - 1;
}

bool StationNav::jumpTo(const std::string& id) {
    for (size_t i = 0; i < order_.size(); ++i) {
        if (order_[i] == id) {
            index_ = i;
            return true;
        }
    }
    return false;
}

}  // namespace railboard
