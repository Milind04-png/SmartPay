#include "User.h"
#include <iomanip>
#include <sstream>
#include <vector>
#include <cstdint>
#include <cstring>

// Standard SHA-256 implementation for secure password hashing
namespace CryptoSHA256 {
    inline uint32_t rotr(uint32_t x, uint32_t n) {
        return (x >> n) | (x << (32 - n));
    }
    inline uint32_t choose(uint32_t e, uint32_t f, uint32_t g) {
        return (e & f) ^ (~e & g);
    }
    inline uint32_t majority(uint32_t a, uint32_t b, uint32_t c) {
        return (a & b) ^ (a & c) ^ (b & c);
    }
    inline uint32_t sig0(uint32_t x) {
        return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
    }
    inline uint32_t sig1(uint32_t x) {
        return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
    }
    inline uint32_t theta0(uint32_t x) {
        return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
    }
    inline uint32_t theta1(uint32_t x) {
        return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
    }

    static const uint32_t K[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };

    std::string compute(const std::string& input) {
        uint64_t bitLen = input.length() * 8ULL;
        std::vector<uint8_t> msg(input.begin(), input.end());
        msg.push_back(0x80);
        while ((msg.size() % 64) != 56) {
            msg.push_back(0x00);
        }
        for (int i = 7; i >= 0; --i) {
            msg.push_back(static_cast<uint8_t>((bitLen >> (i * 8)) & 0xFF));
        }

        uint32_t H[8] = {
            0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
            0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
        };

        for (size_t chunk = 0; chunk < msg.size(); chunk += 64) {
            uint32_t W[64];
            for (int t = 0; t < 16; ++t) {
                W[t] = (msg[chunk + t * 4] << 24) |
                       (msg[chunk + t * 4 + 1] << 16) |
                       (msg[chunk + t * 4 + 2] << 8) |
                       (msg[chunk + t * 4 + 3]);
            }
            for (int t = 16; t < 64; ++t) {
                W[t] = theta1(W[t - 2]) + W[t - 7] + theta0(W[t - 15]) + W[t - 16];
            }

            uint32_t a = H[0], b = H[1], c = H[2], d = H[3];
            uint32_t e = H[4], f = H[5], g = H[6], h = H[7];

            for (int t = 0; t < 64; ++t) {
                uint32_t T1 = h + sig1(e) + choose(e, f, g) + K[t] + W[t];
                uint32_t T2 = sig0(a) + majority(a, b, c);
                h = g;
                g = f;
                f = e;
                e = d + T1;
                d = c;
                c = b;
                b = a;
                a = T1 + T2;
            }

            H[0] += a; H[1] += b; H[2] += c; H[3] += d;
            H[4] += e; H[5] += f; H[6] += g; H[7] += h;
        }

        std::ostringstream result;
        for (int i = 0; i < 8; ++i) {
            result << std::hex << std::setw(8) << std::setfill('0') << H[i];
        }
        return result.str();
    }
}

User::User()
    : userId(""), name(""), email(""), mobile(""),
      passwordHash(""), role("CUSTOMER"), createdAt("") {}

User::User(const std::string& uId,
           const std::string& uName,
           const std::string& uEmail,
           const std::string& uMobile,
           const std::string& pwdHash,
           const std::string& userRole,
           const std::string& createdDate)
    : userId(uId), name(uName), email(uEmail), mobile(uMobile),
      passwordHash(pwdHash), role(userRole), createdAt(createdDate) {}

User::~User() {}

std::string User::getUserId() const { return userId; }
std::string User::getName() const { return name; }
std::string User::getEmail() const { return email; }
std::string User::getMobile() const { return mobile; }
std::string User::getRole() const { return role; }
std::string User::getCreatedAt() const { return createdAt; }
std::string User::getPasswordHash() const { return passwordHash; }

void User::setName(const std::string& newName) { name = newName; }
void User::setEmail(const std::string& newEmail) { email = newEmail; }
void User::setMobile(const std::string& newMobile) { mobile = newMobile; }

bool User::verifyPassword(const std::string& plainInputPassword) const {
    return hashPassword(plainInputPassword) == passwordHash;
}

void User::setPassword(const std::string& newPlainPassword) {
    passwordHash = hashPassword(newPlainPassword);
}

std::string User::hashPassword(const std::string& plainText) {
    // Salt is added to avoid rainbow table attacks
    const std::string salt = "SmartPay_Salt_2026_#SecOps";
    return CryptoSHA256::compute(salt + plainText);
}

void User::displayUserDetails() const {
    std::cout << "  User ID       : " << userId << "\n";
    std::cout << "  Full Name     : " << name << "\n";
    std::cout << "  Email Address : " << email << "\n";
    std::cout << "  Mobile Number : " << mobile << "\n";
    std::cout << "  Account Role  : " << role << "\n";
    std::cout << "  Registered On : " << createdAt << "\n";
}

