#include <string>
#include "../common.hpp"

DLLEXPORT std::string base64_encode(unsigned char const* , unsigned int len);
DLLEXPORT std::string base64_decode(std::string const& s);
