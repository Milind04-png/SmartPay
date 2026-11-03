/**
 * ============================================================================
 * Project: SmartPay – E-Wallet Payment System
 * Course : BTech Computer Science & Engineering - Object Oriented Programming
 * Standard: ISO C++17
 * ============================================================================
 * 
 * Main Entry Point:
 * Instantiates the EWalletSystem application controller and initiates the
 * menu-driven console workflow.
 */

#include "EWalletSystem.h"
#include <iostream>

int main() {
    // Configure standard I/O synchronization
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    try {
        EWalletSystem smartPayApp;
        smartPayApp.run();
    } catch (const std::exception& ex) {
        std::cerr << "\n [Fatal Error] An unhandled exception occurred: " << ex.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "\n [Fatal Error] An unexpected unknown exception terminated the program.\n";
        return 1;
    }

    return 0;
}

