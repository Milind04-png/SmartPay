#ifndef WALLET_H
#define WALLET_H

#include <string>
#include <vector>
#include "Transaction.h"

/**
 * @class Wallet
 * @brief Manages the financial state, balance, and transaction log of a user.
 * 
 * Demonstrates:
 * - Encapsulation (Private balance and ledger, modified strictly through public methods)
 * - Composition (Contains a vector of Transaction objects)
 * - Function Overloading (Multiple signatures for addMoney)
 */
class Wallet {
private:
    std::string walletId;
    double balance;
    std::vector<Transaction> transactions;

public:
    // Constructors
    Wallet();
    Wallet(const std::string& wId, double initialBalance = 0.0);

    // Destructor
    ~Wallet();

    // Getters
    std::string getWalletId() const;
    double getBalance() const;
    const std::vector<Transaction>& getTransactions() const;

    // Function Overloading: Adding money with or without custom description
    bool addMoney(double amount);
    bool addMoney(double amount, const std::string& description);

    // Overloaded helper that also populates the generated Transaction object
    bool addMoney(double amount, const std::string& description, Transaction& outTxn);

    // Fund deduction and crediting for transfers and payments
    bool deductMoney(double amount,
                     const std::string& txnType,
                     const std::string& receiverWallet,
                     const std::string& description,
                     Transaction& outTxn);

    bool creditMoney(double amount,
                     const std::string& txnType,
                     const std::string& senderWallet,
                     const std::string& description,
                     Transaction& outTxn);

    // Manual record logging (e.g. when loading historical data)
    void recordTransaction(const Transaction& txn);

    // Internal initialization for file loading (bypasses transaction generation)
    void setBalanceDirectlyFromStorage(double newBalance);

    // Display helpers
    void displayBalance() const;
    void displayTransactionHistory() const;

    // Serialization helper for file storage
    std::string toFileString() const;
};

#endif // WALLET_H

