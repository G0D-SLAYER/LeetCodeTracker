#include <iostream>
#include <sqlite3.h>
#include <string>
#include <vector>

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
        const char* sql = "CREATE TABLE IF NOT EXISTS questions ("
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

private:
    sqlite3* db;
};
