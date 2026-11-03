#ifndef CUSTOMER_H
#define CUSTOMER_H

#include "User.h"
#include "Wallet.h"

/**
 * @class Customer
 * @brief Represents an individual consumer who holds a digital wallet.
 * 
 * Demonstrates:
 * - Inheritance (Customer IS-A User)
 * - Composition (Customer HAS-A Wallet)
 * - Polymorphism (Overrides displayDashboard and displayUserDetails)
 */
class Customer : public User {
private:
    Wallet wallet; // Composition: Customer has-a Wallet

public:
    // Constructors
    Customer();
    Customer(const std::string& uId,
             const std::string& uName,
             const std::string& uEmail,
             const std::string& uMobile,
             const std::string& pwdHash,
             const std::string& createdDate,
             const std::string& wId,
             double initialBalance = 0.0);

    // Destructor
    virtual ~Customer();

    // Composition Accessors
    Wallet& getWallet();
    const Wallet& getWallet() const;

    // Polymorphism: Overriding virtual functions from User
    void displayDashboard() override;
    void displayUserDetails() const override;

    // File Persistence serialization / deserialization
    std::string toFileString() const;
    static Customer fromFileString(const std::string& line);
};

#endif // CUSTOMER_H

