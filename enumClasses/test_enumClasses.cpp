
#include <iostream>


enum class EnumValues : int
{
    A = 0x01,
    B = 0x02,
    C = 0x04,
    D = 0x08
};



inline EnumValues operator| (EnumValues lhs, EnumValues rhs)
{
    return (EnumValues)(static_cast<std::underlying_type_t<EnumValues>>(lhs) | static_cast<std::underlying_type_t<EnumValues>>(rhs));
}

inline EnumValues operator|= (EnumValues& lhs, EnumValues rhs)
{
    lhs = (EnumValues)(static_cast<std::underlying_type_t<EnumValues>>(lhs) | static_cast<std::underlying_type_t<EnumValues>>(rhs));
    return lhs;
}

inline EnumValues operator& (EnumValues lhs, EnumValues rhs)
{
    return (EnumValues)(static_cast<std::underlying_type_t<EnumValues>>(lhs) & static_cast<std::underlying_type_t<EnumValues>>(rhs));
}

inline EnumValues operator&= (EnumValues& lhs, EnumValues rhs)
{
    lhs = (EnumValues)(static_cast<std::underlying_type_t<EnumValues>>(lhs) & static_cast<std::underlying_type_t<EnumValues>>(rhs));
    return lhs;
}


template <typename T>
    bool isFlagSet (T flag, T value)
{
    return (int)(value & flag) != 0;
}



int main ()
{
    EnumValues eValues = EnumValues::A | EnumValues::B;

    std::cout << "isFlagSet (A, eValues) = " << isFlagSet (EnumValues::A, eValues) << std::endl;
    std::cout << "isFlagSet (B, eValues) = " << isFlagSet (EnumValues::B, eValues) << std::endl;
    std::cout << "isFlagSet (C, eValues) = " << isFlagSet (EnumValues::C, eValues) << std::endl;
    std::cout << "isFlagSet (D, eValues) = " << isFlagSet (EnumValues::D, eValues) << std::endl;
}

