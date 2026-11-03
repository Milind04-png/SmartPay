#include "Wallet.h"
#include <iostream>
#include <iomanip>
#include <random>
#include <sstream>

// Helper to generate a random or sequential unique transaction ID
static std::string makeTxnId() {
    static int counter = 10001;
    std::ostringstream oss;
    oss << "TXN" << counter++;
    return oss.str();
}

Wallet::Wallet() : walletId(""), balance(0.0) {}

Wallet::Wallet(const std::string& wId, double initialBalance)
    : walletId(wId), balance(initialBalance >= 0.0 ? initialBalance : 0.0) {}

Wallet::~Wallet() {}

std::string Wallet::getWalletId() const { return walletId; }
double Wallet::getBalance() const { return balance; }
const std::vector<Transaction>& Wallet::getTransactions() const { return transactions; }

// Function Overloading 1: addMoney with amount only
bool Wallet::addMoney(double amount) {
    return addMoney(amount, "Direct Wallet Top-up");
}

// Function Overloading 2: addMoney with amount and description
bool Wallet::addMoney(double amount, const std::string& description) {
    Transaction dummy;
    return addMoney(amount, description, dummy);
}

// Function Overloading 3: addMoney with amount, description, and output transaction
bool Wallet::addMoney(double amount, const std::string& description, Transaction& outTxn) {
    if (amount <= 0.0) {
        std::cout << " [Error] Deposit amount must be strictly positive.\n";
        return false;
    }

    balance += amount;
    std::string txnId = makeTxnId();
    std::string timestamp = Transaction::getCurrentTimestamp();

    outTxn = Transaction(txnId, timestamp, "DEPOSIT", "BANK_GATEWAY", walletId, amount, "SUCCESS", description);
    transactions.push_back(outTxn);
    return true;
}

bool Wallet::deductMoney(double amount,
                         const std::string& txnType,
                         const std::string& receiverWallet,
                         const std::string& description,
                         Transaction& outTxn) {
    if (amount <= 0.0) {
        std::cout << " [Error] Transaction amount must be positive.\n";
        return false;
    }

    if (balance < amount) {
        std::cout << " [Error] Insufficient wallet balance!\n";
        std::cout << "         Available: Rs. " << std::fixed << std::setprecision(2) << balance
                  << " | Required: Rs. " << amount << "\n";
        return false;
    }

    balance -= amount;
    std::string txnId = makeTxnId();
    std::string timestamp = Transaction::getCurrentTimestamp();

    outTxn = Transaction(txnId, timestamp, txnType, walletId, receiverWallet, amount, "SUCCESS", description);
    transactions.push_back(outTxn);
    return true;
}

bool Wallet::creditMoney(double amount,
                         const std::string& txnType,
                         const std::string& senderWallet,
                         const std::string& description,
                         Transaction& outTxn) {
    if (amount <= 0.0) {
        return false;
    }

    balance += amount;
    std::string txnId = makeTxnId();
    std::string timestamp = Transaction::getCurrentTimestamp();

    outTxn = Transaction(txnId, timestamp, txnType, senderWallet, walletId, amount, "SUCCESS", description);
    transactions.push_back(outTxn);
    return true;
}

void Wallet::recordTransaction(const Transaction& txn) {
    transactions.push_back(txn);
}

void Wallet::setBalanceDirectlyFromStorage(double newBalance) {
    balance = newBalance;
}

void Wallet::displayBalance() const {
    std::cout << "  Wallet ID        : " << walletId << "\n";
    std::cout << "  Available Balance: Rs. " << std::fixed << std::setprecision(2) << balance << "\n";
}

void Wallet::displayTransactionHistory() const {
    if (transactions.empty()) {
        std::cout << "  No transaction records found for Wallet ID: " << walletId << ".\n";
        return;
    }

    std::cout << "\n+-------------+--------------------+-------------------+------------+------------+---------------+----------+\n";
    std::cout << "| TXN ID      | Date & Time        | Type              | Sender     | Receiver   | Amount        | Status   |\n";
    std::cout << "+-------------+--------------------+-------------------+------------+------------+---------------+----------+\n";
    
    // Demonstrate Operator Overloading: cout << txn
    for (const auto& txn : transactions) {
        std::cout << txn << "\n";
    }
    
    std::cout << "+-------------+--------------------+-------------------+------------+------------+---------------+----------+\n";
    std::cout << " Total Recorded Transactions: " << transactions.size() << "\n\n";
}

std::string Wallet::toFileString() const {
    std::ostringstream oss;
    oss << walletId << "|" << std::fixed << std::setprecision(2) << balance;
    return oss.str();
}

