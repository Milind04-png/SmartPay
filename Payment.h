#ifndef PAYMENT_H
#define PAYMENT_H

#include <string>
#include <iostream>
#include "Wallet.h"
#include "Transaction.h"

/**
 * @class Payment
 * @brief Abstract Base Class defining the contract for digital payment processing.
 * 
 * Demonstrates:
 * - Abstraction (Pure virtual processPayment forces derived classes to specify payment logic)
 * - Polymorphism (Enables uniform treatment of distinct payment categories via Payment*)
 * - Virtual Destructor (Ensures correct cleanup of dynamically allocated derived payment objects)
 */
class Payment {
protected:
    std::string categoryName;

public:
    // Constructor & Virtual Destructor
    Payment(const std::string& category);
    virtual ~Payment();

    std::string getCategoryName() const;

    /**
     * @brief Pure virtual function to process payment from user's wallet.
     * @param wallet Reference to customer's wallet.
     * @param amount Payment amount in Rs.
     * @param destination Merchant or utility biller name.
     * @param referenceNote Additional metadata (e.g. consumer number or order ID).
     * @param outTxn Output reference to generated Transaction.
     * @return true if payment successful, false otherwise.
     */
    virtual bool processPayment(Wallet& wallet,
                                double amount,
                                const std::string& destination,
                                const std::string& referenceNote,
                                Transaction& outTxn) = 0;

    // Common receipt display helper
    virtual void printReceipt(const Transaction& txn,
                              const std::string& billerDetails,
                              double extraDiscountOrCashback = 0.0) const;
};

/**
 * @class ShoppingPayment
 * @brief Handles e-commerce, retail, and food orders with promotional cashback.
 */
class ShoppingPayment : public Payment {
private:
    double cashbackRate; // e.g. 0.05 for 5% cashback on orders > Rs. 500

public:
    ShoppingPayment(double cashback = 0.05);
    virtual ~ShoppingPayment();

    bool processPayment(Wallet& wallet,
                        double amount,
                        const std::string& destination,
                        const std::string& referenceNote,
                        Transaction& outTxn) override;
};

/**
 * @class BillPayment
 * @brief Handles utility bill payments (Electricity, Water, Gas, Internet).
 */
class BillPayment : public Payment {
private:
    std::string subCategory; // e.g. "Electricity", "Internet", "Water"

public:
    BillPayment(const std::string& utilityType);
    virtual ~BillPayment();

    bool processPayment(Wallet& wallet,
                        double amount,
                        const std::string& destination,
                        const std::string& referenceNote,
                        Transaction& outTxn) override;
};

/**
 * @class RechargePayment
 * @brief Handles mobile, DTH, and data plan recharges.
 */
class RechargePayment : public Payment {
public:
    RechargePayment();
    virtual ~RechargePayment();

    bool processPayment(Wallet& wallet,
                        double amount,
                        const std::string& destination,
                        const std::string& referenceNote,
                        Transaction& outTxn) override;
};

#endif // PAYMENT_H

