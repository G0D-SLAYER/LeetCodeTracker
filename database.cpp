#include <iostream>
#include <sqlite3.h>
#include <string>
#include <vector>
#include <openssl/sha.h>    // For SHA-256 hashing
#include <sstream>          // For string stream
#include <iomanip>           // For hex formatting

#include "question.h" // Include the Question struct definition

using namespace std;

class Database {
public:
    Database(const string& dbName) {
        if (sqlite3_open(dbName.c_str(), &db) != SQLITE_OK) {
            cerr << "Cannot open database: " << sqlite3_errmsg(db) << endl;
        } else {
            createTable();
        }
    }

    ~Database() {
        sqlite3_close(db);
    }

    void createTable() {
        const char* sql = "CREATE TABLE IF NOT EXISTS users ("
                         "username TEXT PRIMARY KEY,"
                         "password TEXT NOT NULL);"
                         "CREATE TABLE IF NOT EXISTS questions ("
                         "number TEXT PRIMARY KEY,"
                         "text TEXT NOT NULL,"
                         "status TEXT NOT NULL);";
        char* errMsg;
        if (sqlite3_exec(db, sql, nullptr, 0, &errMsg) != SQLITE_OK) {
            cerr << "SQL error: " << errMsg << endl;
            sqlite3_free(errMsg);
        }
    }

    void addQuestion(const string& number, const string& text, const string& status) {
    cout << "Adding question: " << text << " with number: " << number << " and status: " << status << endl; // Debug output
        const char* sql = "INSERT INTO questions (number, text, status) VALUES (?, ?, ?);";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, number.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, text.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, status.c_str(), -1, SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    vector<Question> getQuestions() { // Ensure this returns a vector of Question objects
    cout << "Retrieving questions from the database..." << endl; // Debug output
        vector<Question> questions; // Change to store Question objects
        const char* sql = "SELECT * FROM questions;";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            string number = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            string text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            string status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            questions.push_back({text, status, number}); // Create Question object
        }
        sqlite3_finalize(stmt);
        return questions;
    }

    void updateQuestionInDB(const string& questionNumber, const string& newStatus) {
        const char* sql = "UPDATE questions SET status = ? WHERE number = ?;";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, newStatus.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, questionNumber.c_str(), -1, SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    void deleteQuestionFromDB(const string& questionNumber) {
        const char* sql = "DELETE FROM questions WHERE number = ?;";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, questionNumber.c_str(), -1, SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    void deleteAllQuestionsFromDB() {
        const char* sql = "DELETE FROM questions;";
        char* errMsg;
        if (sqlite3_exec(db, sql, nullptr, 0, &errMsg) != SQLITE_OK) {
            cerr << "SQL error: " << errMsg << endl;
            sqlite3_free(errMsg);
        }
    }

    bool userExists() {
        const char* sql = "SELECT COUNT(*) FROM users;";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        int count = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
        return count > 0;
    }

    bool createUser(const string& username, const string& password) {
        // Create SHA-256 hash of password
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        SHA256_Update(&sha256, password.c_str(), password.length());
        SHA256_Final(hash, &sha256);
        
        // Convert hash to hex string
        std::stringstream ss;
        for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        string hashedPassword = ss.str();
        const char* sql = "INSERT INTO users (username, password) VALUES (?, ?);";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, hashedPassword.c_str(), -1, SQLITE_STATIC);
        int result = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return result == SQLITE_DONE;
    }

    bool authenticateUser(const string& username, const string& password) {
        const char* sql = "SELECT password FROM users WHERE username = ?;";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        bool authenticated = false;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const char* storedHash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            // Hash input password and compare with stored hash
            unsigned char hash[SHA256_DIGEST_LENGTH];
            SHA256_CTX sha256;
            SHA256_Init(&sha256);
            SHA256_Update(&sha256, password.c_str(), password.length());
            SHA256_Final(hash, &sha256);
            
            // Convert hash to hex string
            std::stringstream ss;
            for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
                ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
            }
            string inputHash = ss.str();
            authenticated = (inputHash == storedHash);
        }
        sqlite3_finalize(stmt);
        return authenticated;
    }

private:
    sqlite3* db;
};
