#pragma once
#include <string>
#include <vector>

namespace railboard {

class StationNav {
public:
    StationNav(std::vector<std::string> nav_order, const std::string& start_id);
    const std::string& current() const;
    void next();
    void prev();
    bool jumpTo(const std::string& id);

private:
    std::vector<std::string> order_;
    size_t index_;
};

}  // namespace railboard
