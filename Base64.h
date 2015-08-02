#include <string>

std::wstring base64_encode(char const* in, size_t len);
std::string base64_decode(std::wstring const& s);
