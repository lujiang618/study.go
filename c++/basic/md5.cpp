
#include <boost/uuid/detail/md5.hpp>
#include <boost/algorithm/hex.hpp>
#include <iostream>
#include <string>

std::string md5(const std::string& input) {
    boost::uuids::detail::md5 hash;
    boost::uuids::detail::md5::digest_type digest;
    hash.process_bytes(input.data(), input.size());
    hash.get_digest(digest);

    const char* charDigest = reinterpret_cast<const char*>(&digest);
    std::string result;
    boost::algorithm::hex(charDigest, charDigest + sizeof(digest), std::back_inserter(result));
    return result;
}

int main() {
    std::string data = "Hello, World!";
    std::string hash = md5(data);
    std::cout << "MD5: " << hash << std::endl; // 输出: MD5: fc3ff98e8c6a0d3087d515c0473f8677
    return 0;
}