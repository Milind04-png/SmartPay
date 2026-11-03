#include "Payment.h"
#include <iomanip>

// Base Class Implementation
Payment::Payment(const std::string& category) : categoryName(category) {}
Payment::~Payment() {}

std::string Payment::getCategoryName() const { return categoryName; }

void Payment::printReceipt(const Transaction& txn,
                           const std::string& billerDetails,
                           double extraDiscountOrCashback) const {
    std::cout << "\n+-------------------------------------------------------+\n";
    std::cout << "|              SMARTPAY PAYMENT RECEIPT                 |\n";
    std::cout << "+-------------------------------------------------------+\n";
    std::cout << "  Category       : " << categoryName << "\n";
    std::cout << "  Transaction ID : " << txn.getTransactionId() << "\n";
    std::cout << "  Date & Time    : " << txn.getTimestamp() << "\n";
    std::cout << "  Paid To        : " << txn.getReceiverWalletId() << "\n";
    std::cout << "  Paid From      : " << txn.getSenderWalletId() << "\n";
    std::cout << "  Bill / Ref Info: " << billerDetails << "\n";
    std::cout << "  Paid Amount    : Rs. " << std::fixed << std::setprecision(2) << txn.getAmount() << "\n";
    if (extraDiscountOrCashback > 0.0) {
        std::cout << "  Cashback Added : Rs. " << std::fixed << std::setprecision(2) << extraDiscountOrCashback << " (Credited!)\n";
    }
    std::cout << "  Status         : " << txn.getStatus() << "\n";
    std::cout << "+-------------------------------------------------------+\n";
    std::cout << "|   Transaction Processed Successfully via SmartPay     |\n";
    std::cout << "+-------------------------------------------------------+\n\n";
}

// ShoppingPayment Implementation
ShoppingPayment::ShoppingPayment(double cashback)
    : Payment("Shopping & Dining"), cashbackRate(cashback) {}

ShoppingPayment::~ShoppingPayment() {}

bool ShoppingPayment::processPayment(Wallet& wallet,
                                     double amount,
                                     const std::string& destination,
                                     const std::string& referenceNote,
                                     Transaction& outTxn) {
    std::string desc = "Merchant: " + destination + " | Order: " + referenceNote;
    bool success = wallet.deductMoney(amount, "PAYMENT_SHOPPING", destination, desc, outTxn);
    if (!success) {
        return false;
    }

    double cashback = 0.0;
    if (amount >= 500.0) {
        cashback = amount * cashbackRate;
        Transaction cbTxn;
        wallet.creditMoney(cashback, "CASHBACK_REWARD", "SMARTPAY_PROMO", "Cashback on shopping", cbTxn);
        std::cout << " [SmartPay Reward] Congratulations! You earned Rs. "
                  << std::fixed << std::setprecision(2) << cashback << " cashback!\n";
    }

    printReceipt(outTxn, referenceNote, cashback);
    return true;
}

// BillPayment Implementation
BillPayment::BillPayment(const std::string& utilityType)
    : Payment(utilityType + " Payment"), subCategory(utilityType) {}

BillPayment::~BillPayment() {}

bool BillPayment::processPayment(Wallet& wallet,
                                 double amount,
                                 const std::string& destination,
                                 const std::string& referenceNote,
                                 Transaction& outTxn) {
    std::string desc = subCategory + " | Provider: " + destination + " | Consumer No: " + referenceNote;
    bool success = wallet.deductMoney(amount, "PAYMENT_BILL", destination, desc, outTxn);
    if (!success) {
        return false;
    }

    printReceipt(outTxn, "Consumer ID: " + referenceNote);
    return true;
}

// RechargePayment Implementation
RechargePayment::RechargePayment()
    : Payment("Mobile & DTH Recharge") {}

RechargePayment::~RechargePayment() {}

bool RechargePayment::processPayment(Wallet& wallet,
                                     double amount,
                                     const std::string& destination,
                                     const std::string& referenceNote,
                                     Transaction& outTxn) {
    std::string desc = "Recharge | Operator: " + destination + " | Number: " + referenceNote;
    bool success = wallet.deductMoney(amount, "PAYMENT_RECHARGE", destination, desc, outTxn);
    if (!success) {
        return false;
    }

    printReceipt(outTxn, "Mobile/DTH: " + referenceNote + " (" + destination + ")");
    return true;
}

