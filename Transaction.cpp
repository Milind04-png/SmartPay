#include "Transaction.h"
#include <sstream>
#include <chrono>
#include <ctime>
#include <vector>

Transaction::Transaction()
    : transactionId(""), timestamp(""), type("UNKNOWN"),
      senderWalletId("N/A"), receiverWalletId("N/A"),
      amount(0.0), status("PENDING"), description("None") {}

Transaction::Transaction(const std::string& txnId,
                         const std::string& timeStr,
                         const std::string& txnType,
                         const std::string& senderWId,
                         const std::string& receiverWId,
                         double amt,
                         const std::string& stat,
                         const std::string& desc)
    : transactionId(txnId), timestamp(timeStr), type(txnType),
      senderWalletId(senderWId), receiverWalletId(receiverWId),
      amount(amt), status(stat), description(desc) {}

Transaction::~Transaction() {}

std::string Transaction::getTransactionId() const { return transactionId; }
std::string Transaction::getTimestamp() const { return timestamp; }
std::string Transaction::getType() const { return type; }
std::string Transaction::getSenderWalletId() const { return senderWalletId; }
std::string Transaction::getReceiverWalletId() const { return receiverWalletId; }
double Transaction::getAmount() const { return amount; }
std::string Transaction::getStatus() const { return status; }
std::string Transaction::getDescription() const { return description; }

void Transaction::setStatus(const std::string& newStatus) { status = newStatus; }
void Transaction::setDescription(const std::string& newDesc) { description = newDesc; }

// Operator Overloading for tabular stream insertion
std::ostream& operator<<(std::ostream& os, const Transaction& txn) {
    os << "| " << std::left << std::setw(12) << txn.transactionId
       << "| " << std::setw(19) << txn.timestamp
       << "| " << std::setw(18) << txn.type
       << "| " << std::setw(11) << txn.senderWalletId
       << "| " << std::setw(11) << txn.receiverWalletId
       << "| Rs." << std::right << std::setw(9) << std::fixed << std::setprecision(2) << txn.amount << " "
       << "| " << std::left << std::setw(9) << txn.status
       << "|";
    return os;
}

std::string Transaction::toFileString() const {
    std::ostringstream oss;
    oss << transactionId << "|"
        << timestamp << "|"
        << type << "|"
        << senderWalletId << "|"
        << receiverWalletId << "|"
        << std::fixed << std::setprecision(2) << amount << "|"
        << status << "|"
        << description;
    return oss.str();
}

Transaction Transaction::fromFileString(const std::string& line) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(line);
    while (std::getline(tokenStream, token, '|')) {
        tokens.push_back(token);
    }

    if (tokens.size() >= 8) {
        double amt = 0.0;
        try {
            amt = std::stod(tokens[5]);
        } catch (...) {
            amt = 0.0;
        }
        return Transaction(tokens[0], tokens[1], tokens[2], tokens[3], tokens[4], amt, tokens[6], tokens[7]);
    }
    return Transaction();
}

void Transaction::printReceipt(const std::string& extraDetails) const {
    std::cout << "\n+-------------------------------------------------------+\n";
    std::cout << "|                 SMARTPAY OFFICIAL RECEIPT             |\n";
    std::cout << "+-------------------------------------------------------+\n";
    std::cout << "  Transaction ID : " << transactionId << "\n";
    std::cout << "  Date & Time    : " << timestamp << "\n";
    std::cout << "  Payment Type   : " << type << "\n";
    std::cout << "  Sender Wallet  : " << senderWalletId << "\n";
    std::cout << "  Receiver ID    : " << receiverWalletId << "\n";
    std::cout << "  Amount         : Rs. " << std::fixed << std::setprecision(2) << amount << "\n";
    std::cout << "  Status         : " << status << "\n";
    std::cout << "  Remarks        : " << description << "\n";
    if (!extraDetails.empty()) {
        std::cout << "  Note           : " << extraDetails << "\n";
    }
    std::cout << "+-------------------------------------------------------+\n";
    std::cout << "|           Thank you for choosing SmartPay!            |\n";
    std::cout << "+-------------------------------------------------------+\n\n";
}

std::string Transaction::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm ltm;
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&ltm, &now_time);
#else
    localtime_r(&now_time, &ltm);
#endif
    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &ltm);
    return std::string(buffer);
}

