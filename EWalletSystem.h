#ifndef EWALLETSYSTEM_H
#define EWALLETSYSTEM_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "User.h"
#include "Customer.h"
#include "Admin.h"
#include "Wallet.h"
#include "Transaction.h"
#include "Payment.h"

/**
 * @class EWalletSystem
 * @brief Central controller and orchestrator for SmartPay.
 * 
 * Responsibilities:
 * - User registration, authentication, and session management
 * - Financial transactions (top-ups, P2P transfers, merchant payments)
 * - Administrative auditing and analytics
 * - File I/O persistence across app restarts
 * - Console UI rendering, menus, and input validation
 */
class EWalletSystem {
private:
    std::unordered_map<std::string, Customer> customers;        // Key: userId
    std::unordered_map<std::string, std::string> walletToUserMap; // Key: walletId -> userId
    std::unordered_map<std::string, Admin> admins;              // Key: adminId
    std::vector<Transaction> allTransactions;                   // Global audit ledger

    User* currentUser; // Polymorphic pointer: points to logged-in Customer or Admin

    // File path definitions
    const std::string usersFile = "data/users.txt";
    const std::string walletsFile = "data/wallets.txt";
    const std::string transactionsFile = "data/transactions.txt";
    const std::string adminFile = "data/admin.txt";

    // Auto-increment sequence generators
    int nextWalletSeq;
    int nextTxnSeq;

    // Internal helper methods
    void printBanner() const;
    void printDivider() const;
    void printSuccess(const std::string& msg) const;
    void printError(const std::string& msg) const;
    void printInfo(const std::string& msg) const;

    bool isValidEmail(const std::string& email) const;
    bool isValidMobile(const std::string& mobile) const;

    std::string generateWalletId();
    std::string generateTxnId();

    // Data persistence routines
    void loadData();
    void saveData();
    void seedInitialDemoData();

    // Customer operations
    void handleCustomerMenu();
    void handleAddMoney();
    void handleSendMoney();
    void handleMakePayment();
    void handleViewBalance();
    void handleTransactionHistory();
    void handleViewProfile();
    void handleEditProfile();
    void handleChangePassword();

    // Admin operations
    void handleAdminMenu();
    void adminViewUsers();
    void adminViewAccountSummary();
    void adminViewAllTransactions();
    void adminSearchUserByWallet();
    void adminViewSystemBalance();

public:
    // Constructor & Destructor
    EWalletSystem();
    ~EWalletSystem();

    // Primary entry point for application workflow
    void run();

    // Authentication workflows
    void handleRegistration();
    void handleLogin();
    void logout();
};

#endif // EWALLETSYSTEM_H

