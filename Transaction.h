#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <string>
#include <iostream>
#include <iomanip>

/**
 * @class Transaction
 * @brief Represents a financial ledger entry within the SmartPay system.
 * 
 * Demonstrates:
 * - Encapsulation of transaction attributes
 * - Operator Overloading (operator<<) for formatted transaction representation
 * - Object serialization for file persistence
 */
class Transaction {
private:
    std::string transactionId;
    std::string timestamp;
    std::string type;             // DEPOSIT, TRANSFER_SENT, TRANSFER_RECEIVED, PAYMENT_SHOPPING, etc.
    std::string senderWalletId;   // Source wallet or external gateway
    std::string receiverWalletId; // Destination wallet or merchant
    double amount;
    std::string status;           // SUCCESS, FAILED
    std::string description;

public:
    // Constructors
    Transaction();
    Transaction(const std::string& txnId,
                const std::string& timeStr,
                const std::string& txnType,
                const std::string& senderWId,
                const std::string& receiverWId,
                double amt,
                const std::string& stat,
                const std::string& desc = "N/A");

    // Destructor
    ~Transaction();

    // Getters
    std::string getTransactionId() const;
    std::string getTimestamp() const;
    std::string getType() const;
    std::string getSenderWalletId() const;
    std::string getReceiverWalletId() const;
    double getAmount() const;
    std::string getStatus() const;
    std::string getDescription() const;

    // Setters
    void setStatus(const std::string& newStatus);
    void setDescription(const std::string& newDesc);

    // Operator Overloading: outputs tabular summary of transaction
    friend std::ostream& operator<<(std::ostream& os, const Transaction& txn);

    // File persistence serialization / deserialization
    std::string toFileString() const;
    static Transaction fromFileString(const std::string& line);

    // Detailed formatted receipt printer
    void printReceipt(const std::string& extraDetails = "") const;

    // Utility helper to get current wall-clock timestamp
    static std::string getCurrentTimestamp();
};

#endif // TRANSACTION_H

