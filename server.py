#!/usr/bin/env python3
"""
SmartPay Localhost Web Server & REST API
Connects the subtle light-mode web dashboard directly to the C++ project's persistent flat-file database.
Zero external pip dependencies: uses Python 3 standard library.
"""

import http.server
import socketserver
import json
import os
import urllib.parse
from datetime import datetime

PORT = 5000
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
DATA_DIR = os.path.join(BASE_DIR, "data")
PUBLIC_DIR = os.path.join(BASE_DIR, "public")

USERS_FILE = os.path.join(DATA_DIR, "users.txt")
WALLETS_FILE = os.path.join(DATA_DIR, "wallets.txt")
TXNS_FILE = os.path.join(DATA_DIR, "transactions.txt")
ADMIN_FILE = os.path.join(DATA_DIR, "admin.txt")

def read_db():
    users = {}
    wallets = {}
    transactions = []
    admins = {}

    # Read Users
    if os.path.exists(USERS_FILE):
        with open(USERS_FILE, "r", encoding="utf-8") as f:
            for line in f:
                line = line.strip()
                if not line:
                    continue
                parts = line.split("|")
                if len(parts) >= 8:
                    users[parts[0]] = {
                        "userId": parts[0],
                        "name": parts[1],
                        "email": parts[2],
                        "mobile": parts[3],
                        "role": parts[5],
                        "walletId": parts[6],
                        "createdAt": parts[7]
                    }

    # Read Wallets
    if os.path.exists(WALLETS_FILE):
        with open(WALLETS_FILE, "r", encoding="utf-8") as f:
            for line in f:
                line = line.strip()
                if not line:
                    continue
                parts = line.split("|")
                if len(parts) >= 3:
                    wId, uId, bal = parts[0], parts[1], float(parts[2])
                    wallets[wId] = {"userId": uId, "balance": bal}
                    if uId in users:
                        users[uId]["balance"] = bal

    # Read Transactions
    if os.path.exists(TXNS_FILE):
        with open(TXNS_FILE, "r", encoding="utf-8") as f:
            for line in f:
                line = line.strip()
                if not line:
                    continue
                parts = line.split("|")
                if len(parts) >= 8:
                    transactions.append({
                        "id": parts[0],
                        "timestamp": parts[1],
                        "type": parts[2],
                        "sender": parts[3],
                        "receiver": parts[4],
                        "amount": float(parts[5]),
                        "status": parts[6],
                        "description": parts[7]
                    })

    # Read Admins
    if os.path.exists(ADMIN_FILE):
        with open(ADMIN_FILE, "r", encoding="utf-8") as f:
            for line in f:
                line = line.strip()
                if not line:
                    continue
                parts = line.split("|")
                if len(parts) >= 8:
                    admins[parts[0]] = {
                        "userId": parts[0],
                        "name": parts[1],
                        "email": parts[2],
                        "mobile": parts[3],
                        "role": parts[5],
                        "adminLevel": parts[6],
                        "createdAt": parts[7]
                    }

    return users, wallets, transactions, admins

def save_wallets_and_txns(wallets, transactions):
    with open(WALLETS_FILE, "w", encoding="utf-8") as f:
        for wId, data in wallets.items():
            f.write(f"{wId}|{data['userId']}|{data['balance']:.2f}\n")

    with open(TXNS_FILE, "w", encoding="utf-8") as f:
        for t in transactions:
            f.write(f"{t['id']}|{t['timestamp']}|{t['type']}|{t['sender']}|{t['receiver']}|{t['amount']:.2f}|{t['status']}|{t['description']}\n")

def save_users(users):
    # Need to preserve password hashes from existing file
    existing_hashes = {}
    if os.path.exists(USERS_FILE):
        with open(USERS_FILE, "r", encoding="utf-8") as f:
            for line in f:
                parts = line.strip().split("|")
                if len(parts) >= 8:
                    existing_hashes[parts[0]] = parts[4]

    with open(USERS_FILE, "w", encoding="utf-8") as f:
        for uId, u in users.items():
            pwd_hash = existing_hashes.get(uId, "77b17c60d58d50d7f42412ac2a8eb6d79f291e919cbc00d5236e52dc8d355825")
            f.write(f"{u['userId']}|{u['name']}|{u['email']}|{u['mobile']}|{pwd_hash}|{u['role']}|{u['walletId']}|{u['createdAt']}\n")

def get_next_txn_id(transactions):
    max_id = 10000
    for t in transactions:
        if t["id"].startswith("TXN"):
            try:
                num = int(t["id"][3:])
                if num > max_id:
                    max_id = num
            except ValueError:
                pass
    return f"TXN{max_id + 1}"

class SmartPayHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=PUBLIC_DIR, **kwargs)

    def end_headers(self):
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "Content-Type")
        self.send_header("Cache-Control", "no-cache, no-store, must-revalidate")
        super().end_headers()

    def do_OPTIONS(self):
        self.send_response(200)
        self.end_headers()

    def send_json(self, data, status_code=200):
        self.send_response(status_code)
        self.send_header("Content-Type", "application/json")
        self.end_headers()
        self.wfile.write(json.dumps(data, indent=2).encode("utf-8"))

    def do_GET(self):
        parsed = urllib.parse.urlparse(self.path)
        if parsed.path == "/api/state":
            users, wallets, transactions, admins = read_db()
            total_liquidity = sum(w["balance"] for w in wallets.values())
            self.send_json({
                "status": "connected",
                "systemLiquidity": total_liquidity,
                "users": users,
                "wallets": wallets,
                "transactions": transactions,
                "admins": admins
            })
            return

        # Default static file serving from public directory
        if parsed.path == "/" or parsed.path == "":
            self.path = "/index.html"
        return super().do_GET()

    def do_POST(self):
        parsed = urllib.parse.urlparse(self.path)
        content_length = int(self.headers.get("Content-Length", 0))
        post_data = self.rfile.read(content_length)

        try:
            payload = json.loads(post_data.decode("utf-8")) if post_data else {}
        except Exception:
            payload = {}

        users, wallets, transactions, admins = read_db()
        now_str = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

        # 1. ADD MONEY
        if parsed.path == "/api/add-money":
            wallet_id = payload.get("walletId")
            amount = float(payload.get("amount", 0.0))
            description = payload.get("description", "Direct Wallet Top-up")

            if amount <= 0:
                self.send_json({"success": False, "error": "Amount must be strictly positive."}, 400)
                return

            if wallet_id not in wallets:
                self.send_json({"success": False, "error": f"Wallet ID '{wallet_id}' not found."}, 404)
                return

            wallets[wallet_id]["balance"] += amount
            txn_id = get_next_txn_id(transactions)
            new_txn = {
                "id": txn_id,
                "timestamp": now_str,
                "type": "DEPOSIT",
                "sender": "BANK_GATEWAY",
                "receiver": wallet_id,
                "amount": amount,
                "status": "SUCCESS",
                "description": description
            }
            transactions.insert(0, new_txn)
            save_wallets_and_txns(wallets, transactions)

            self.send_json({
                "success": True,
                "message": f"Successfully credited Rs. {amount:.2f} to {wallet_id}.",
                "newBalance": wallets[wallet_id]["balance"],
                "transaction": new_txn
            })
            return

        # 2. P2P MONEY TRANSFER
        if parsed.path == "/api/transfer":
            sender_wId = payload.get("senderWalletId")
            receiver_wId = payload.get("receiverWalletId")
            amount = float(payload.get("amount", 0.0))
            note = payload.get("note", "P2P Peer Transfer")

            if amount <= 0:
                self.send_json({"success": False, "error": "Amount must be strictly positive."}, 400)
                return

            if sender_wId == receiver_wId:
                self.send_json({"success": False, "error": "Cannot transfer money to yourself."}, 400)
                return

            if sender_wId not in wallets:
                self.send_json({"success": False, "error": "Sender wallet not found."}, 404)
                return

            if receiver_wId not in wallets:
                self.send_json({"success": False, "error": f"Recipient wallet ID '{receiver_wId}' does not exist."}, 404)
                return

            if wallets[sender_wId]["balance"] < amount:
                self.send_json({"success": False, "error": f"Insufficient wallet balance. Available: Rs. {wallets[sender_wId]['balance']:.2f}"}, 400)
                return

            wallets[sender_wId]["balance"] -= amount
            wallets[receiver_wId]["balance"] += amount

            txn_id = get_next_txn_id(transactions)
            new_txn = {
                "id": txn_id,
                "timestamp": now_str,
                "type": "TRANSFER_SENT",
                "sender": sender_wId,
                "receiver": receiver_wId,
                "amount": amount,
                "status": "SUCCESS",
                "description": note
            }
            transactions.insert(0, new_txn)
            save_wallets_and_txns(wallets, transactions)

            receiver_name = users.get(wallets[receiver_wId]["userId"], {}).get("name", receiver_wId)
            self.send_json({
                "success": True,
                "message": f"Transferred Rs. {amount:.2f} to {receiver_name} ({receiver_wId}).",
                "newBalance": wallets[sender_wId]["balance"],
                "transaction": new_txn
            })
            return

        # 3. PAYMENT (Shopping / Bill / Recharge)
        if parsed.path == "/api/pay":
            wallet_id = payload.get("walletId")
            category = payload.get("category", "shopping")
            destination = payload.get("destination", "Merchant")
            reference = payload.get("reference", "N/A")
            amount = float(payload.get("amount", 0.0))

            if amount <= 0:
                self.send_json({"success": False, "error": "Amount must be strictly positive."}, 400)
                return

            if wallet_id not in wallets:
                self.send_json({"success": False, "error": "Wallet not found."}, 404)
                return

            if wallets[wallet_id]["balance"] < amount:
                self.send_json({"success": False, "error": "Insufficient balance for payment."}, 400)
                return

            wallets[wallet_id]["balance"] -= amount

            cashback = 0.0
            txn_type = "PAYMENT_BILL"
            if category == "shopping":
                txn_type = "PAYMENT_SHOPPING"
                if amount >= 500.0:
                    cashback = amount * 0.05
                    wallets[wallet_id]["balance"] += cashback
            elif category == "recharge":
                txn_type = "PAYMENT_RECHARGE"

            txn_id = get_next_txn_id(transactions)
            new_txn = {
                "id": txn_id,
                "timestamp": now_str,
                "type": txn_type,
                "sender": wallet_id,
                "receiver": destination,
                "amount": amount,
                "status": "SUCCESS",
                "description": f"{category.capitalize()} | Ref: {reference}"
            }
            transactions.insert(0, new_txn)

            cb_txn = None
            if cashback > 0.0:
                cb_id = get_next_txn_id(transactions)
                cb_txn = {
                    "id": cb_id,
                    "timestamp": now_str,
                    "type": "CASHBACK_REWARD",
                    "sender": "SMARTPAY_PROMO",
                    "receiver": wallet_id,
                    "amount": cashback,
                    "status": "SUCCESS",
                    "description": "5% Shopping reward credited to wallet"
                }
                transactions.insert(0, cb_txn)

            save_wallets_and_txns(wallets, transactions)

            self.send_json({
                "success": True,
                "message": f"Payment of Rs. {amount:.2f} to {destination} completed.",
                "cashback": cashback,
                "newBalance": wallets[wallet_id]["balance"],
                "transaction": new_txn,
                "cashbackTransaction": cb_txn
            })
            return

        # 4. PROFILE UPDATE
        if parsed.path == "/api/profile":
            user_id = payload.get("userId")
            if user_id not in users:
                self.send_json({"success": False, "error": "User not found."}, 404)
                return

            if "name" in payload and payload["name"]:
                users[user_id]["name"] = payload["name"]
            if "email" in payload and payload["email"]:
                users[user_id]["email"] = payload["email"]
            if "mobile" in payload and payload["mobile"]:
                users[user_id]["mobile"] = payload["mobile"]

            save_users(users)
            self.send_json({"success": True, "message": "Profile updated successfully.", "user": users[user_id]})
            return

        self.send_json({"error": "Endpoint not found"}, 404)

def run_server():
    socketserver.TCPServer.allow_reuse_address = True
    with socketserver.TCPServer(("", PORT), SmartPayHandler) as httpd:
        print(f"=======================================================")
        print(f"   SmartPay E-Wallet Platform - Localhost Web Server   ")
        print(f"=======================================================")
        print(f"   URL: http://localhost:{PORT}")
        print(f"   Listening on: 0.0.0.0:{PORT}")
        print(f"   Database Sync: C++ data/ flat files")
        print(f"=======================================================")
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\nShutting down server cleanly...")
            httpd.shutdown()

if __name__ == "__main__":
    run_server()

