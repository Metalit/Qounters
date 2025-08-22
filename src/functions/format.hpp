#pragma once
#include <string>
#include "../shared/types.hpp" // for Qounters::Types::Separators

namespace Format {
    inline std::string FormatNumber(int value, int separator) {
        std::string seperatorString;
        switch ((Qounters::Types::Separators) separator) {
            case Qounters::Types::Separators::None:
                return std::to_string(value);
            case Qounters::Types::Separators::Gap:
                seperatorString = " ";
                break;
            case Qounters::Types::Separators::Comma:
                seperatorString = ",";
                break;
            case Qounters::Types::Separators::Period:
                seperatorString = ".";
                break;
        }

        std::string num = std::to_string(std::abs(value));
        int insertPosition = static_cast<int>(num.length()) - 3;
        while (insertPosition > 0) {
            num.insert(insertPosition, seperatorString);
            insertPosition -= 3;
        }
        return (value < 0 ? "-" + num : num);
    }

}