#ifndef ADMIN_H
#define ADMIN_H

#include "User.h"

/**
 * @class Admin
 * @brief Represents a system administrator with oversight privileges.
 * 
 * Demonstrates:
 * - Inheritance (Admin IS-A User)
 * - Polymorphism (Overrides displayDashboard and displayUserDetails)
 * - Principle of Least Privilege (Admin has no direct access to user passwords or wallet manipulation)
 */
class Admin : public User {
private:
    std::string adminLevel; // e.g. "SENIOR_SYSTEM_ADMIN", "FINANCIAL_AUDITOR"

public:
    // Constructors
    Admin();
    Admin(const std::string& uId,
          const std::string& uName,
          const std::string& uEmail,
          const std::string& uMobile,
          const std::string& pwdHash,
          const std::string& createdDate,
          const std::string& level = "SENIOR_SYSTEM_ADMIN");

    // Destructor
    virtual ~Admin();

    // Getters
    std::string getAdminLevel() const;

    // Polymorphism: Overriding virtual functions
    void displayDashboard() override;
    void displayUserDetails() const override;

    // File Persistence serialization / deserialization
    std::string toFileString() const;
    static Admin fromFileString(const std::string& line);
};

#endif // ADMIN_H

