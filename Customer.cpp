#include "Customer.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>

Customer::Customer()
    : User(), wallet() {}

Customer::Customer(const std::string& uId,
                   const std::string& uName,
                   const std::string& uEmail,
                   const std::string& uMobile,
                   const std::string& pwdHash,
                   const std::string& createdDate,
                   const std::string& wId,
                   double initialBalance)
    : User(uId, uName, uEmail, uMobile, pwdHash, "CUSTOMER", createdDate),
      wallet(wId, initialBalance) {}

Customer::~Customer() {}

Wallet& Customer::getWallet() { return wallet; }
const Wallet& Customer::getWallet() const { return wallet; }

void Customer::displayDashboard() {
    std::cout << "\n=======================================================\n";
    std::cout << "                 SMARTPAY CUSTOMER DASHBOARD           \n";
    std::cout << "=======================================================\n";
    std::cout << " Welcome back, " << name << "!\n";
    std::cout << " Wallet ID        : " << wallet.getWalletId() << "\n";
    std::cout << " Available Balance: Rs. " << std::fixed << std::setprecision(2) << wallet.getBalance() << "\n";
    std::cout << "-------------------------------------------------------\n";
    std::cout << "  [1] View Wallet Balance\n";
    std::cout << "  [2] Add Money (Wallet Top-Up)\n";
    std::cout << "  [3] Send Money (P2P Transfer)\n";
    std::cout << "  [4] Make Payment (Bills, Shopping, Recharge)\n";
    std::cout << "  [5] Transaction History\n";
    std::cout << "  [6] View / Edit Profile\n";
    std::cout << "  [7] Change Password\n";
    std::cout << "  [8] Logout\n";
    std::cout << "=======================================================\n";
    std::cout << " Enter your choice (1-8): ";
}

void Customer::displayUserDetails() const {
    std::cout << "\n+-------------------------------------------------------+\n";
    std::cout << "|                 CUSTOMER ACCOUNT PROFILE              |\n";
    std::cout << "+-------------------------------------------------------+\n";
    User::displayUserDetails();
    std::cout << "  Associated Wallet: " << wallet.getWalletId() << "\n";
    std::cout << "  Current Balance  : Rs. " << std::fixed << std::setprecision(2) << wallet.getBalance() << "\n";
    std::cout << "  Ledger Records   : " << wallet.getTransactions().size() << " transactions\n";
    std::cout << "+-------------------------------------------------------+\n\n";
}

std::string Customer::toFileString() const {
    std::ostringstream oss;
    oss << userId << "|"
        << name << "|"
        << email << "|"
        << mobile << "|"
        << passwordHash << "|"
        << role << "|"
        << wallet.getWalletId() << "|"
        << createdAt;
    return oss.str();
}

Customer Customer::fromFileString(const std::string& line) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(line);
    while (std::getline(tokenStream, token, '|')) {
        tokens.push_back(token);
    }

    if (tokens.size() >= 8) {
        // userId|name|email|mobile|passwordHash|CUSTOMER|walletId|createdAt
        return Customer(tokens[0], tokens[1], tokens[2], tokens[3], tokens[4], tokens[7], tokens[6], 0.0);
    }
    return Customer();
}

