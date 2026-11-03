#include "EWalletSystem.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <limits>
#include <algorithm>
#include <regex>

// Helper to safely flush cin after failed or formatted reads
static void clearCin() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// Helper to read a non-empty string line from console
static std::string readLine(const std::string& prompt) {
    std::string input;
    std::cout << prompt;
    std::getline(std::cin, input);
    // Trim leading and trailing whitespace
    size_t first = input.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = input.find_last_not_of(" \t\r\n");
    return input.substr(first, (last - first + 1));
}

EWalletSystem::EWalletSystem()
    : currentUser(nullptr), nextWalletSeq(10001), nextTxnSeq(10001) {
    loadData();
    if (customers.empty() && admins.empty()) {
        seedInitialDemoData();
    }
}

EWalletSystem::~EWalletSystem() {
    saveData();
}

void EWalletSystem::printBanner() const {
    std::cout << "\n=======================================================\n";
    std::cout << "          SMARTPAY  |  SECURE DIGITAL E-WALLET         \n";
    std::cout << "           Next-Gen Object-Oriented Payment Platform   \n";
    std::cout << "=======================================================\n";
}

void EWalletSystem::printDivider() const {
    std::cout << "-------------------------------------------------------\n";
}

void EWalletSystem::printSuccess(const std::string& msg) const {
    std::cout << " [SUCCESS] " << msg << "\n";
}

void EWalletSystem::printError(const std::string& msg) const {
    std::cout << " [ERROR] " << msg << "\n";
}

void EWalletSystem::printInfo(const std::string& msg) const {
    std::cout << " [INFO] " << msg << "\n";
}

bool EWalletSystem::isValidEmail(const std::string& email) const {
    // Basic standard RFC-style email check: non-empty, contains '@' and '.'
    const std::regex pattern(R"(^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\.[a-zA-Z0-9-.]+$)");
    return std::regex_match(email, pattern);
}

bool EWalletSystem::isValidMobile(const std::string& mobile) const {
    // 10-digit numeric format validation
    if (mobile.length() != 10) return false;
    for (char c : mobile) {
        if (!std::isdigit(c)) return false;
    }
    return true;
}

std::string EWalletSystem::generateWalletId() {
    std::ostringstream oss;
    oss << "SP" << nextWalletSeq++;
    return oss.str();
}

std::string EWalletSystem::generateTxnId() {
    std::ostringstream oss;
    oss << "TXN" << nextTxnSeq++;
    return oss.str();
}

void EWalletSystem::seedInitialDemoData() {
    printInfo("Initializing SmartPay with pre-configured demonstration accounts...");

    // Create Demo Admin
    std::string adminId = "admin";
    std::string adminPwdHash = User::hashPassword("Admin@123");
    Admin sysAdmin(adminId, "Chief Administrator", "admin@smartpay.io", "9876543210", adminPwdHash, "2026-01-01 10:00:00", "CHIEF_SECURITY_OFFICER");
    admins[adminId] = sysAdmin;

    // Create Customer 1: Milind
    std::string wId1 = generateWalletId();
    Customer c1("user1", "Milind", "milind@smartpay.io", "9876543211", User::hashPassword("User1@123"), "2026-01-05 11:30:00", wId1, 5000.0);
    customers["user1"] = c1;
    walletToUserMap[wId1] = "user1";

    // Create Customer 2: Aarav Patel
    std::string wId2 = generateWalletId();
    Customer c2("user2", "Aarav Patel", "aarav@gmail.com", "9876543212", User::hashPassword("User2@123"), "2026-01-10 14:15:00", wId2, 3500.0);
    customers["user2"] = c2;
    walletToUserMap[wId2] = "user2";

    // Create Customer 3: Priya Singh
    std::string wId3 = generateWalletId();
    Customer c3("user3", "Priya Singh", "priya@outlook.com", "9876543213", User::hashPassword("User3@123"), "2026-01-15 09:45:00", wId3, 2000.0);
    customers["user3"] = c3;
    walletToUserMap[wId3] = "user3";

    // Add some sample audit transactions
    Transaction t1(generateTxnId(), "2026-01-05 11:35:00", "DEPOSIT", "BANK_GATEWAY", wId1, 5000.0, "SUCCESS", "Initial Bank Deposit");
    Transaction t2(generateTxnId(), "2026-01-10 14:20:00", "DEPOSIT", "BANK_GATEWAY", wId2, 3500.0, "SUCCESS", "Initial Bank Deposit");
    Transaction t3(generateTxnId(), "2026-01-15 09:50:00", "DEPOSIT", "BANK_GATEWAY", wId3, 2000.0, "SUCCESS", "Initial Bank Deposit");

    customers["user1"].getWallet().recordTransaction(t1);
    customers["user2"].getWallet().recordTransaction(t2);
    customers["user3"].getWallet().recordTransaction(t3);

    allTransactions.push_back(t1);
    allTransactions.push_back(t2);
    allTransactions.push_back(t3);

    saveData();
    printSuccess("Demo accounts seeded successfully.");
}

void EWalletSystem::loadData() {
    // 1. Load Users (Customers)
    std::ifstream uf(usersFile);
    if (uf.is_open()) {
        std::string line;
        while (std::getline(uf, line)) {
            if (line.empty()) continue;
            Customer c = Customer::fromFileString(line);
            if (!c.getUserId().empty()) {
                customers[c.getUserId()] = c;
                walletToUserMap[c.getWallet().getWalletId()] = c.getUserId();
                // Sequence update
                try {
                    int wNum = std::stoi(c.getWallet().getWalletId().substr(2));
                    if (wNum >= nextWalletSeq) nextWalletSeq = wNum + 1;
                } catch (...) {}
            }
        }
        uf.close();
    }

    // 2. Load Wallets (Balances)
    std::ifstream wf(walletsFile);
    if (wf.is_open()) {
        std::string line;
        while (std::getline(wf, line)) {
            if (line.empty()) continue;
            std::istringstream ss(line);
            std::string wId, uId, balStr;
            if (std::getline(ss, wId, '|') && std::getline(ss, uId, '|') && std::getline(ss, balStr, '|')) {
                if (customers.find(uId) != customers.end()) {
                    try {
                        double bal = std::stod(balStr);
                        customers[uId].getWallet().setBalanceDirectlyFromStorage(bal);
                    } catch (...) {}
                }
            }
        }
        wf.close();
    }

    // 3. Load Transactions
    std::ifstream tf(transactionsFile);
    if (tf.is_open()) {
        std::string line;
        while (std::getline(tf, line)) {
            if (line.empty()) continue;
            Transaction txn = Transaction::fromFileString(line);
            if (!txn.getTransactionId().empty()) {
                allTransactions.push_back(txn);
                // Assign to sender and receiver wallets if present
                if (walletToUserMap.find(txn.getSenderWalletId()) != walletToUserMap.end()) {
                    std::string uId = walletToUserMap[txn.getSenderWalletId()];
                    customers[uId].getWallet().recordTransaction(txn);
                }
                if (walletToUserMap.find(txn.getReceiverWalletId()) != walletToUserMap.end()) {
                    std::string uId = walletToUserMap[txn.getReceiverWalletId()];
                    customers[uId].getWallet().recordTransaction(txn);
                }
                // Sequence update
                try {
                    int tNum = std::stoi(txn.getTransactionId().substr(3));
                    if (tNum >= nextTxnSeq) nextTxnSeq = tNum + 1;
                } catch (...) {}
            }
        }
        tf.close();
    }

    // 4. Load Admin
    std::ifstream af(adminFile);
    if (af.is_open()) {
        std::string line;
        while (std::getline(af, line)) {
            if (line.empty()) continue;
            Admin a = Admin::fromFileString(line);
            if (!a.getUserId().empty()) {
                admins[a.getUserId()] = a;
            }
        }
        af.close();
    }
}

void EWalletSystem::saveData() {
    // 1. Save Users
    std::ofstream uf(usersFile, std::ios::trunc);
    if (uf.is_open()) {
        for (const auto& pair : customers) {
            uf << pair.second.toFileString() << "\n";
        }
        uf.close();
    }

    // 2. Save Wallets
    std::ofstream wf(walletsFile, std::ios::trunc);
    if (wf.is_open()) {
        for (const auto& pair : customers) {
            wf << pair.second.getWallet().getWalletId() << "|"
               << pair.second.getUserId() << "|"
               << std::fixed << std::setprecision(2) << pair.second.getWallet().getBalance() << "\n";
        }
        wf.close();
    }

    // 3. Save Transactions
    std::ofstream tf(transactionsFile, std::ios::trunc);
    if (tf.is_open()) {
        for (const auto& txn : allTransactions) {
            tf << txn.toFileString() << "\n";
        }
        tf.close();
    }

    // 4. Save Admin
    std::ofstream af(adminFile, std::ios::trunc);
    if (af.is_open()) {
        for (const auto& pair : admins) {
            af << pair.second.toFileString() << "\n";
        }
        af.close();
    }
}

void EWalletSystem::handleRegistration() {
    printBanner();
    std::cout << "\n >>> USER REGISTRATION <<<\n\n";

    std::string uId = readLine("  Enter Desired User ID: ");
    if (uId.empty()) {
        printError("User ID cannot be empty.");
        return;
    }

    // Check duplicate
    if (customers.find(uId) != customers.end() || admins.find(uId) != admins.end()) {
        printError("User ID '" + uId + "' already exists! Please choose a unique ID.");
        return;
    }

    std::string name = readLine("  Enter Full Name: ");
    if (name.empty()) {
        printError("Name cannot be empty.");
        return;
    }

    std::string email = readLine("  Enter Email Address: ");
    if (!isValidEmail(email)) {
        printError("Invalid email format! Example: user@domain.com");
        return;
    }

    std::string mobile = readLine("  Enter 10-Digit Mobile Number: ");
    if (!isValidMobile(mobile)) {
        printError("Mobile number must be exactly 10 digits.");
        return;
    }

    std::string password = readLine("  Enter Account Password (min 6 chars): ");
    if (password.length() < 6) {
        printError("Password must be at least 6 characters long.");
        return;
    }

    std::string confirmPwd = readLine("  Confirm Password: ");
    if (password != confirmPwd) {
        printError("Passwords do not match!");
        return;
    }

    // Generate unique wallet ID
    std::string walletId = generateWalletId();
    std::string nowTime = Transaction::getCurrentTimestamp();
    std::string pwdHash = User::hashPassword(password);

    // Initial deposit
    double initialDeposit = 0.0;
    std::string depStr = readLine("  Initial Wallet Deposit Amount (Rs., enter 0 for none): ");
    try {
        if (!depStr.empty()) initialDeposit = std::stod(depStr);
    } catch (...) {
        initialDeposit = 0.0;
    }

    if (initialDeposit < 0.0) initialDeposit = 0.0;

    // Create Customer
    Customer newCustomer(uId, name, email, mobile, pwdHash, nowTime, walletId, initialDeposit);
    if (initialDeposit > 0.0) {
        Transaction initTxn(generateTxnId(), nowTime, "DEPOSIT", "BANK_GATEWAY", walletId, initialDeposit, "SUCCESS", "Initial Registration Deposit");
        newCustomer.getWallet().recordTransaction(initTxn);
        allTransactions.push_back(initTxn);
    }

    customers[uId] = newCustomer;
    walletToUserMap[walletId] = uId;
    saveData();

    printDivider();
    printSuccess("Registration completed successfully!");
    std::cout << "  Assigned Wallet ID : " << walletId << "\n";
    std::cout << "  Starting Balance   : Rs. " << std::fixed << std::setprecision(2) << initialDeposit << "\n";
    std::cout << "  You can now log in using your User ID and Password.\n";
    printDivider();
}

void EWalletSystem::handleLogin() {
    printBanner();
    std::cout << "\n >>> USER AUTHENTICATION / LOGIN <<<\n\n";

    std::string uId = readLine("  Enter User ID: ");
    std::string password = readLine("  Enter Password: ");

    if (uId.empty() || password.empty()) {
        printError("User ID and Password cannot be blank.");
        return;
    }

    // 1. Check Customer accounts
    auto cIt = customers.find(uId);
    if (cIt != customers.end()) {
        if (cIt->second.verifyPassword(password)) {
            currentUser = &(cIt->second); // Polymorphic assignment
            printSuccess("Authentication successful! Welcome, " + currentUser->getName() + ".");
            handleCustomerMenu();
            return;
        } else {
            printError("Invalid password entered! Access denied.");
            return;
        }
    }

    // 2. Check Admin accounts
    auto aIt = admins.find(uId);
    if (aIt != admins.end()) {
        if (aIt->second.verifyPassword(password)) {
            currentUser = &(aIt->second); // Polymorphic assignment
            printSuccess("Administrator authentication verified.");
            handleAdminMenu();
            return;
        } else {
            printError("Invalid administrator credentials! Access denied.");
            return;
        }
    }

    printError("User ID '" + uId + "' not found in the SmartPay system.");
}

void EWalletSystem::logout() {
    if (currentUser) {
        printInfo("User '" + currentUser->getUserId() + "' logged out securely.");
        currentUser = nullptr;
    }
}

// ----------------- CUSTOMER DASHBOARD & OPERATIONS -----------------

void EWalletSystem::handleCustomerMenu() {
    Customer* cust = dynamic_cast<Customer*>(currentUser);
    if (!cust) return;

    int choice = 0;
    while (currentUser != nullptr) {
        // Polymorphism in action: calling virtual displayDashboard() on User* pointer!
        currentUser->displayDashboard();

        if (!(std::cin >> choice)) {
            clearCin();
            printError("Invalid input! Please enter a number from 1 to 8.");
            continue;
        }
        clearCin();

        switch (choice) {
            case 1:
                handleViewBalance();
                break;
            case 2:
                handleAddMoney();
                break;
            case 3:
                handleSendMoney();
                break;
            case 4:
                handleMakePayment();
                break;
            case 5:
                handleTransactionHistory();
                break;
            case 6:
                handleViewProfile();
                break;
            case 7:
                handleChangePassword();
                break;
            case 8:
                logout();
                break;
            default:
                printError("Invalid choice! Please select an option between 1 and 8.");
        }
    }
}

void EWalletSystem::handleViewBalance() {
    Customer* cust = dynamic_cast<Customer*>(currentUser);
    if (!cust) return;

    std::cout << "\n+-------------------------------------------------------+\n";
    std::cout << "|                 CURRENT WALLET BALANCE                |\n";
    std::cout << "+-------------------------------------------------------+\n";
    cust->getWallet().displayBalance();
    std::cout << "+-------------------------------------------------------+\n";
}

void EWalletSystem::handleAddMoney() {
    Customer* cust = dynamic_cast<Customer*>(currentUser);
    if (!cust) return;

    std::cout << "\n=======================================================\n";
    std::cout << "                  ADD VIRTUAL MONEY                    \n";
    std::cout << "=======================================================\n";
    std::cout << " Current Balance: Rs. " << std::fixed << std::setprecision(2) << cust->getWallet().getBalance() << "\n";

    std::string amtStr = readLine(" Enter Amount to Add (Rs.): ");
    double amount = 0.0;
    try {
        amount = std::stod(amtStr);
    } catch (...) {
        printError("Invalid amount format entered.");
        return;
    }

    if (amount <= 0.0) {
        printError("Amount must be greater than zero.");
        return;
    }

    std::string desc = readLine(" Enter Note / Source (e.g. NetBanking, UPI, Debit Card): ");
    if (desc.empty()) desc = "Direct Wallet Top-up";

    // Demonstrates Function Overloading: addMoney(amount, desc, outTxn)
    Transaction txn;
    bool success = cust->getWallet().addMoney(amount, desc, txn);
    if (success) {
        allTransactions.push_back(txn);
        saveData();
        printSuccess("Virtual money credited to your wallet successfully!");
        txn.printReceipt("Wallet Top-up");
    }
}

void EWalletSystem::handleSendMoney() {
    Customer* sender = dynamic_cast<Customer*>(currentUser);
    if (!sender) return;

    std::cout << "\n=======================================================\n";
    std::cout << "            SEND MONEY (P2P WALLET TRANSFER)           \n";
    std::cout << "=======================================================\n";
    std::cout << " Your Wallet ID     : " << sender->getWallet().getWalletId() << "\n";
    std::cout << " Available Balance  : Rs. " << std::fixed << std::setprecision(2) << sender->getWallet().getBalance() << "\n";
    printDivider();

    std::string receiverWId = readLine(" Enter Receiver's Wallet ID (e.g. SP10002): ");
    if (receiverWId.empty()) {
        printError("Receiver Wallet ID cannot be blank.");
        return;
    }

    // Validation 1: Cannot transfer to self
    if (receiverWId == sender->getWallet().getWalletId()) {
        printError("Invalid Operation: You cannot transfer money to your own wallet ID.");
        return;
    }

    // Validation 2: Receiver wallet must exist
    auto rIt = walletToUserMap.find(receiverWId);
    if (rIt == walletToUserMap.end()) {
        printError("Receiver Wallet ID '" + receiverWId + "' does not exist in the system!");
        return;
    }

    std::string receiverUserId = rIt->second;
    Customer& receiver = customers[receiverUserId];

    std::cout << "  Recipient Found: " << receiver.getName() << " (" << receiverWId << ")\n";

    std::string amtStr = readLine(" Enter Transfer Amount (Rs.): ");
    double amount = 0.0;
    try {
        amount = std::stod(amtStr);
    } catch (...) {
        printError("Invalid amount format entered.");
        return;
    }

    // Validation 3: Amount > 0
    if (amount <= 0.0) {
        printError("Transfer amount must be strictly greater than zero.");
        return;
    }

    // Validation 4: Sender has sufficient balance
    if (sender->getWallet().getBalance() < amount) {
        printError("Insufficient wallet balance for this transfer!");
        std::cout << "  Available: Rs. " << sender->getWallet().getBalance()
                  << " | Requested: Rs. " << amount << "\n";
        return;
    }

    std::string note = readLine(" Enter Transfer Remarks (optional): ");
    if (note.empty()) note = "P2P Peer Transfer";

    // Deduct from sender
    Transaction txnSent;
    bool deducted = sender->getWallet().deductMoney(amount, "TRANSFER_SENT", receiverWId, note, txnSent);
    if (!deducted) {
        printError("Transaction deduction failed.");
        return;
    }

    // Credit to receiver
    Transaction txnRecv;
    receiver.getWallet().creditMoney(amount, "TRANSFER_RECEIVED", sender->getWallet().getWalletId(), note, txnRecv);

    // Save to global audit log
    allTransactions.push_back(txnSent);

    // Persist immediately
    saveData();

    printDivider();
    printSuccess("Funds transferred successfully to " + receiver.getName() + " (" + receiverWId + ")!");
    txnSent.printReceipt("Transfer Confirmation");
}

void EWalletSystem::handleMakePayment() {
    Customer* cust = dynamic_cast<Customer*>(currentUser);
    if (!cust) return;

    std::cout << "\n=======================================================\n";
    std::cout << "                 MAKE PAYMENT / PAY BILLS              \n";
    std::cout << "=======================================================\n";
    std::cout << " Available Balance: Rs. " << std::fixed << std::setprecision(2) << cust->getWallet().getBalance() << "\n";
    std::cout << " Select Category:\n";
    std::cout << "  [1] Shopping (Online & Retail Stores)\n";
    std::cout << "  [2] Food & Dining (Restaurants & Delivery)\n";
    std::cout << "  [3] Electricity Bill\n";
    std::cout << "  [4] Mobile Recharge\n";
    std::cout << "  [5] Internet / Broadband Bill\n";
    std::cout << "  [6] Other Utility Services\n";
    std::cout << "  [7] Return to Dashboard\n";
    printDivider();

    std::string choiceStr = readLine(" Enter Choice (1-7): ");
    int catChoice = 0;
    try {
        catChoice = std::stoi(choiceStr);
    } catch (...) {
        printError("Invalid choice.");
        return;
    }

    if (catChoice == 7) return;

    // Demonstrates Abstraction & Polymorphism:
    // Base class pointer Payment* points to dynamically instantiated derived payment objects!
    std::unique_ptr<Payment> paymentProcessor = nullptr;
    std::string destinationPrompt = " Enter Merchant / Biller Name: ";
    std::string refPrompt = " Enter Reference Note: ";

    switch (catChoice) {
        case 1:
            paymentProcessor = std::make_unique<ShoppingPayment>(0.05); // 5% cashback on large orders
            destinationPrompt = " Enter Store / Merchant (e.g. Amazon, Zara, Flipkart): ";
            refPrompt = " Enter Order ID: ";
            break;
        case 2:
            paymentProcessor = std::make_unique<ShoppingPayment>(0.03); // 3% dining cashback
            destinationPrompt = " Enter Restaurant / Service (e.g. Zomato, Swiggy, Starbucks): ";
            refPrompt = " Enter Bill / Order Number: ";
            break;
        case 3:
            paymentProcessor = std::make_unique<BillPayment>("Electricity");
            destinationPrompt = " Enter Electricity Board (e.g. State Power Corp, Tata Power): ";
            refPrompt = " Enter Consumer Account Number: ";
            break;
        case 4:
            paymentProcessor = std::make_unique<RechargePayment>();
            destinationPrompt = " Enter Telecom Operator (e.g. Jio, Airtel, Vodafone Idea): ";
            refPrompt = " Enter 10-Digit Mobile Number: ";
            break;
        case 5:
            paymentProcessor = std::make_unique<BillPayment>("Internet Broadband");
            destinationPrompt = " Enter ISP Provider (e.g. Airtel Xstream, JioFiber, ACT): ";
            refPrompt = " Enter Broadband Account / Landline No: ";
            break;
        case 6:
            paymentProcessor = std::make_unique<BillPayment>("Other Utility");
            destinationPrompt = " Enter Service Department / Agency: ";
            refPrompt = " Enter Reference / Invoice ID: ";
            break;
        default:
            printError("Invalid category selected.");
            return;
    }

    std::string destination = readLine(destinationPrompt);
    if (destination.empty()) {
        printError("Destination / Biller name cannot be empty.");
        return;
    }

    std::string reference = readLine(refPrompt);
    if (reference.empty()) {
        printError("Reference identifier cannot be empty.");
        return;
    }

    // Additional validation for Mobile Recharge
    if (catChoice == 4 && !isValidMobile(reference)) {
        printError("Mobile recharge requires a valid 10-digit number.");
        return;
    }

    std::string amtStr = readLine(" Enter Payment Amount (Rs.): ");
    double amount = 0.0;
    try {
        amount = std::stod(amtStr);
    } catch (...) {
        printError("Invalid amount format.");
        return;
    }

    if (amount <= 0.0) {
        printError("Payment amount must be greater than zero.");
        return;
    }

    if (cust->getWallet().getBalance() < amount) {
        printError("Insufficient wallet balance for this payment!");
        return;
    }

    // Confirmation
    std::string confirm = readLine(" Confirm payment of Rs. " + amtStr + " to " + destination + "? (y/n): ");
    if (confirm != "y" && confirm != "Y") {
        printInfo("Payment cancelled by user.");
        return;
    }

    // Polymorphic method invocation
    Transaction outTxn;
    bool success = paymentProcessor->processPayment(cust->getWallet(), amount, destination, reference, outTxn);
    if (success) {
        allTransactions.push_back(outTxn);
        saveData();
        printSuccess("Payment executed successfully!");
    }
}

void EWalletSystem::handleTransactionHistory() {
    Customer* cust = dynamic_cast<Customer*>(currentUser);
    if (!cust) return;

    cust->getWallet().displayTransactionHistory();
}

void EWalletSystem::handleViewProfile() {
    // Polymorphism: virtual function call on User*
    currentUser->displayUserDetails();

    std::string opt = readLine(" Would you like to edit your profile details? (y/n): ");
    if (opt == "y" || opt == "Y") {
        handleEditProfile();
    }
}

void EWalletSystem::handleEditProfile() {
    std::cout << "\n >>> UPDATE PROFILE INFORMATION <<<\n";
    std::cout << " (Press Enter without typing to keep current value)\n\n";

    std::string newName = readLine(" Enter New Name [" + currentUser->getName() + "]: ");
    if (!newName.empty()) {
        currentUser->setName(newName);
    }

    std::string newEmail = readLine(" Enter New Email [" + currentUser->getEmail() + "]: ");
    if (!newEmail.empty()) {
        if (isValidEmail(newEmail)) {
            currentUser->setEmail(newEmail);
        } else {
            printError("Invalid email format! Kept existing email.");
        }
    }

    std::string newMobile = readLine(" Enter New Mobile [" + currentUser->getMobile() + "]: ");
    if (!newMobile.empty()) {
        if (isValidMobile(newMobile)) {
            currentUser->setMobile(newMobile);
        } else {
            printError("Invalid mobile number! Kept existing mobile.");
        }
    }

    saveData();
    printSuccess("Profile information updated successfully.");
}

void EWalletSystem::handleChangePassword() {
    std::cout << "\n >>> CHANGE ACCOUNT PASSWORD <<<\n\n";

    std::string currPwd = readLine(" Enter Current Password: ");
    if (!currentUser->verifyPassword(currPwd)) {
        printError("Current password verification failed!");
        return;
    }

    std::string newPwd = readLine(" Enter New Password (min 6 characters): ");
    if (newPwd.length() < 6) {
        printError("New password must be at least 6 characters.");
        return;
    }

    std::string confirmPwd = readLine(" Confirm New Password: ");
    if (newPwd != confirmPwd) {
        printError("Passwords do not match!");
        return;
    }

    currentUser->setPassword(newPwd);
    saveData();
    printSuccess("Password updated successfully! Please remember your new password.");
}

// ----------------- ADMIN DASHBOARD & OPERATIONS -----------------

void EWalletSystem::handleAdminMenu() {
    Admin* admin = dynamic_cast<Admin*>(currentUser);
    if (!admin) return;

    int choice = 0;
    while (currentUser != nullptr) {
        // Polymorphism: calling virtual displayDashboard()
        currentUser->displayDashboard();

        if (!(std::cin >> choice)) {
            clearCin();
            printError("Invalid input! Please enter a number from 1 to 8.");
            continue;
        }
        clearCin();

        switch (choice) {
            case 1:
                adminViewUsers();
                break;
            case 2:
                adminViewAccountSummary();
                break;
            case 3:
                adminViewAllTransactions();
                break;
            case 4:
                adminSearchUserByWallet();
                break;
            case 5:
                adminViewSystemBalance();
                break;
            case 6:
                currentUser->displayUserDetails(); // Polymorphism
                break;
            case 7:
                handleChangePassword();
                break;
            case 8:
                logout();
                break;
            default:
                printError("Invalid option! Please select between 1 and 8.");
        }
    }
}

void EWalletSystem::adminViewUsers() {
    std::cout << "\n=========================================================================================\n";
    std::cout << "                             REGISTERED CUSTOMERS AUDIT LOG                              \n";
    std::cout << "=========================================================================================\n";
    std::cout << "| " << std::left << std::setw(10) << "User ID"
              << "| " << std::setw(20) << "Full Name"
              << "| " << std::setw(22) << "Email"
              << "| " << std::setw(12) << "Mobile"
              << "| " << std::setw(10) << "Wallet ID"
              << "| " << std::setw(12) << "Balance" << "|\n";
    std::cout << "+-----------+---------------------+-----------------------+-------------+-----------+-------------+\n";

    for (const auto& pair : customers) {
        const Customer& c = pair.second;
        std::cout << "| " << std::left << std::setw(10) << c.getUserId()
                  << "| " << std::setw(20) << c.getName()
                  << "| " << std::setw(22) << c.getEmail()
                  << "| " << std::setw(12) << c.getMobile()
                  << "| " << std::setw(10) << c.getWallet().getWalletId()
                  << "| Rs." << std::right << std::setw(8) << std::fixed << std::setprecision(2) << c.getWallet().getBalance() << " |\n";
    }
    std::cout << "+-----------+---------------------+-----------------------+-------------+-----------+-------------+\n";
    std::cout << " Total Registered Customers: " << customers.size() << "\n";
    std::cout << " [Note] User passwords are strictly hashed and inaccessible even to administrators.\n\n";
}

void EWalletSystem::adminViewAccountSummary() {
    std::cout << "\n=======================================================\n";
    std::cout << "             SMARTPAY PLATFORM SYSTEM SUMMARY          \n";
    std::cout << "=======================================================\n";
    std::cout << " Total Registered Customers   : " << customers.size() << "\n";
    std::cout << " Total Registered Admins      : " << admins.size() << "\n";
    std::cout << " Total Active Wallets         : " << walletToUserMap.size() << "\n";
    std::cout << " Total Recorded Transactions  : " << allTransactions.size() << "\n";

    double totalLiquidity = 0.0;
    for (const auto& pair : customers) {
        totalLiquidity += pair.second.getWallet().getBalance();
    }

    std::cout << " Total System Liquidity Pool  : Rs. " << std::fixed << std::setprecision(2) << totalLiquidity << "\n";
    std::cout << "=======================================================\n\n";
}

void EWalletSystem::adminViewAllTransactions() {
    std::cout << "\n=======================================================================================================\n";
    std::cout << "                                  SMARTPAY GLOBAL TRANSACTION LEDGER                                   \n";
    std::cout << "=======================================================================================================\n";
    std::cout << "| " << std::left << std::setw(12) << "TXN ID"
              << "| " << std::setw(19) << "Date & Time"
              << "| " << std::setw(18) << "Type"
              << "| " << std::setw(11) << "Sender"
              << "| " << std::setw(11) << "Receiver"
              << "| " << std::setw(13) << "Amount"
              << "| " << std::setw(9) << "Status" << "|\n";
    std::cout << "+-------------+--------------------+-------------------+------------+------------+---------------+----------+\n";

    for (const auto& txn : allTransactions) {
        // Operator Overloading: std::cout << txn
        std::cout << txn << "\n";
    }
    std::cout << "+-------------+--------------------+-------------------+------------+------------+---------------+----------+\n";
    std::cout << " Total System Transactions: " << allTransactions.size() << "\n\n";
}

void EWalletSystem::adminSearchUserByWallet() {
    std::cout << "\n >>> SEARCH USER BY WALLET ID <<<\n\n";
    std::string searchWId = readLine(" Enter Wallet ID to lookup (e.g. SP10001): ");

    auto it = walletToUserMap.find(searchWId);
    if (it == walletToUserMap.end()) {
        printError("No customer found matching Wallet ID '" + searchWId + "'.");
        return;
    }

    const Customer& cust = customers[it->second];
    std::cout << "\n [Match Found]\n";
    cust.displayUserDetails();
}

void EWalletSystem::adminViewSystemBalance() {
    double totalBalance = 0.0;
    for (const auto& pair : customers) {
        totalBalance += pair.second.getWallet().getBalance();
    }

    std::cout << "\n+-------------------------------------------------------+\n";
    std::cout << "|          SYSTEM-WIDE VIRTUAL WALLET BALANCES          |\n";
    std::cout << "+-------------------------------------------------------+\n";
    std::cout << "  Total Active Customer Wallets : " << customers.size() << "\n";
    std::cout << "  Aggregated Virtual Reserves   : Rs. " << std::fixed << std::setprecision(2) << totalBalance << "\n";
    std::cout << "  System Reserve Health         : OPTIMAL\n";
    std::cout << "+-------------------------------------------------------+\n\n";
}

// ----------------- MAIN WORKFLOW LOOP -----------------

void EWalletSystem::run() {
    int mainChoice = 0;
    while (true) {
        printBanner();
        std::cout << "\n MAIN MENU:\n";
        std::cout << "  [1] Login (Customer / Admin)\n";
        std::cout << "  [2] Register New Customer Account\n";
        std::cout << "  [3] Exit System\n";
        printDivider();
        std::cout << " Select an option (1-3): ";

        if (!(std::cin >> mainChoice)) {
            clearCin();
            printError("Invalid input! Please enter 1, 2, or 3.");
            continue;
        }
        clearCin();

        switch (mainChoice) {
            case 1:
                handleLogin();
                break;
            case 2:
                handleRegistration();
                break;
            case 3:
                saveData();
                std::cout << "\n Thank you for using SmartPay E-Wallet System.\n Goodbye!\n\n";
                return;
            default:
                printError("Invalid option! Please select 1, 2, or 3.");
        }
    }
}

