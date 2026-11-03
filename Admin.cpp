#include "Admin.h"
#include <iostream>
#include <sstream>
#include <vector>

Admin::Admin()
    : User(), adminLevel("STANDARD_ADMIN") {}

Admin::Admin(const std::string& uId,
             const std::string& uName,
             const std::string& uEmail,
             const std::string& uMobile,
             const std::string& pwdHash,
             const std::string& createdDate,
             const std::string& level)
    : User(uId, uName, uEmail, uMobile, pwdHash, "ADMIN", createdDate),
      adminLevel(level) {}

Admin::~Admin() {}

std::string Admin::getAdminLevel() const { return adminLevel; }

void Admin::displayDashboard() {
    std::cout << "\n=======================================================\n";
    std::cout << "                 SMARTPAY ADMINISTRATOR PORTAL         \n";
    std::cout << "=======================================================\n";
    std::cout << " Logged in as     : " << name << " (" << adminLevel << ")\n";
    std::cout << " Administrator ID : " << userId << "\n";
    std::cout << "-------------------------------------------------------\n";
    std::cout << "  [1] View Registered Users List\n";
    std::cout << "  [2] View Total Accounts & Summary\n";
    std::cout << "  [3] View All System Transactions (Audit Log)\n";
    std::cout << "  [4] Search User by Wallet ID\n";
    std::cout << "  [5] View Overall System Wallet Balance\n";
    std::cout << "  [6] View Admin Profile\n";
    std::cout << "  [7] Change Admin Password\n";
    std::cout << "  [8] Logout\n";
    std::cout << "=======================================================\n";
    std::cout << " Enter your choice (1-8): ";
}

void Admin::displayUserDetails() const {
    std::cout << "\n+-------------------------------------------------------+\n";
    std::cout << "|              ADMINISTRATOR ACCOUNT PROFILE            |\n";
    std::cout << "+-------------------------------------------------------+\n";
    User::displayUserDetails();
    std::cout << "  Admin Level   : " << adminLevel << "\n";
    std::cout << "  Access Rights : Read-Only Auditing & User Management\n";
    std::cout << "+-------------------------------------------------------+\n\n";
}

std::string Admin::toFileString() const {
    std::ostringstream oss;
    oss << userId << "|"
        << name << "|"
        << email << "|"
        << mobile << "|"
        << passwordHash << "|"
        << role << "|"
        << adminLevel << "|"
        << createdAt;
    return oss.str();
}

Admin Admin::fromFileString(const std::string& line) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(line);
    while (std::getline(tokenStream, token, '|')) {
        tokens.push_back(token);
    }

    if (tokens.size() >= 8) {
        // userId|name|email|mobile|passwordHash|ADMIN|adminLevel|createdAt
        return Admin(tokens[0], tokens[1], tokens[2], tokens[3], tokens[4], tokens[7], tokens[6]);
    }
    return Admin();
}

