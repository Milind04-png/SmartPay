#ifndef USER_H
#define USER_H

#include <string>
#include <iostream>

/**
 * @class User
 * @brief Abstract Base Class representing a general user in the SmartPay ecosystem.
 * 
 * Demonstrates:
 * - Inheritance (Base for Customer and Admin)
 * - Encapsulation (Private/Protected fields with controlled accessor & mutator methods)
 * - Polymorphism (Virtual functions displayDashboard and displayUserDetails)
 * - Password Security (SHA-256 hashing without storing plaintext passwords)
 */
class User {
protected:
    std::string userId;
    std::string name;
    std::string email;
    std::string mobile;
    std::string passwordHash;
    std::string role;       // "CUSTOMER" or "ADMIN"
    std::string createdAt;

public:
    // Constructors
    User();
    User(const std::string& uId,
         const std::string& uName,
         const std::string& uEmail,
         const std::string& uMobile,
         const std::string& pwdHash,
         const std::string& userRole,
         const std::string& createdDate);

    // Virtual Destructor (Crucial for polymorphic base classes)
    virtual ~User();

    // Getters
    std::string getUserId() const;
    std::string getName() const;
    std::string getEmail() const;
    std::string getMobile() const;
    std::string getRole() const;
    std::string getCreatedAt() const;
    std::string getPasswordHash() const;

    // Setters (with validation in system manager)
    void setName(const std::string& newName);
    void setEmail(const std::string& newEmail);
    void setMobile(const std::string& newMobile);

    // Authentication methods
    bool verifyPassword(const std::string& plainInputPassword) const;
    void setPassword(const std::string& newPlainPassword);

    // Pure Virtual Function: Forces derived classes (Customer/Admin) to implement their unique dashboard
    virtual void displayDashboard() = 0;

    // Virtual Function: Displays base user details, overridden in derived classes
    virtual void displayUserDetails() const;

    // Static SHA-256 cryptographic password hasher
    static std::string hashPassword(const std::string& plainText);
};

#endif // USER_H

