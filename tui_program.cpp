#include <ncurses.h>
#include <vector>
#include <string>
#include "database.cpp" // Include the database header
#include <cstring> // Include for strlen
#include <cctype>  // Include for isdigit
#include <algorithm> // Include for transform

using namespace std;

class TUI {
public:
    TUI(Database& db) : db(db) {
        initscr();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);
        curs_set(0);
    }

    ~TUI() {
        endwin();
    }

    void run() {
        if (!db.userExists()) {
            createUser();
        } else {
            authenticate();
        }
    }

private:
    Database& db;

    void authenticate() {
        vector<string> authOptions = {"Login", "Create User", "Delete User", "Quit"};
        int selected = 0;
        while (true) {
            int choice = printMenu(authOptions, selected);
            if (choice == -1) {
                selected = (selected + 1) % authOptions.size();
            } else if (choice == 0) {
                login();
                break;
            } else if (choice == 1) {
                createUser();
                break;
            } else if (choice == 2) {
                deleteUser();
            } else if (choice == 3) {
                endwin();
                exit(0);
            }
        }
    }

    void deleteUser() {
        vector<string> users = db.getAllUsers();
        if (users.empty()) {
            showMessage("No users found!");
            return;
        }

        int selected = 0;
        string username;
        while (true) {
            clear();
            printw("Select user to delete (ESC to cancel):\n\n");
            for (size_t i = 0; i < users.size(); i++) {
                if (i == selected) {
                    attron(A_REVERSE);
                }
                printw("%ld. %s\n", i + 1, users[i].c_str());
                if (i == selected) {
                    attroff(A_REVERSE);
                }
            }
            refresh();

            int ch = getch();
            if (ch == KEY_UP) {
                selected = (selected - 1 + users.size()) % users.size();
            } else if (ch == KEY_DOWN) {
                selected = (selected + 1) % users.size();
            } else if (ch == 10) { // Enter
                username = users[selected];
                break;
            } else if (ch == 27) { // ESC
                return;
            }
        }

        char password[100];
        echo();
        curs_set(1);
        printw("Enter password for %s: ", username.c_str());
        getstr(password);
        noecho();
        curs_set(0);

        if (db.deleteUser(username, password)) {
            showMessage("User deleted successfully!");
        } else {
            showMessage("Failed to delete user. Incorrect password?");
        }
        authenticate(); // Return to authentication menu
    }

    void createUser() {
        char username[100];
        char password[100];
        echo();
        curs_set(1);
        printw("Enter username: ");
        getstr(username);
        printw("Enter password: ");
        getstr(password);
        noecho();
        curs_set(0);

        if (db.createUser(username, password)) {
            showMessage("User created successfully!");
        } else {
            showMessage("Failed to create user. Username may already exist.");
        }
    }

    void login() {
        char username[100];
        char password[100];
        echo();
        curs_set(1);
        printw("Username: ");
        getstr(username);
        printw("Password: ");
        getstr(password);
        noecho();
        curs_set(0);

        if (db.authenticateUser(username, password)) {
            showMessage("Login successful!");
            mainMenu();
        } else {
            showMessage("Login failed. Incorrect username or password.");
        }
    }

    void mainMenu() {
        vector<string> options = {"Add Question", "Show Questions", "Search Question", "Delete All Questions", "Exit"};
        int selected = 0;
        while (true) {
            int choice = printMenu(options, selected);
            if (choice == -1) {
                selected = (selected + 1) % options.size();
            } else if (choice == 0) {
                addQuestion();
            } else if (choice == 1) {
                showQuestions();
            } else if (choice == 2) {
                searchQuestion();
            } else if (choice == 3) {
                deleteAllQuestions();
            } else if (choice == 4) {
                break;
            }
        }
    }

    void addQuestion() {
        // Existing addQuestion implementation
    }

    void showQuestions() {
        // Existing showQuestions implementation
    }

    void searchQuestion() {
        // Existing searchQuestion implementation
    }

    void deleteAllQuestions() {
        // Existing deleteAllQuestions implementation
    }

    void showMessage(const string& message) {
        clear();
        printw("%s\nPress any key to continue...", message.c_str());
        getch();
    }

    int printMenu(const vector<string>& options, int selected) {
        clear();
        printw("Use arrow keys to navigate, Enter to select\n\n");
        for (size_t i = 0; i < options.size(); i++) {
            if (i == selected) {
                attron(A_REVERSE);
            }
            printw("%ld. %s\n", i + 1, options[i].c_str());
            if (i == selected) {
                attroff(A_REVERSE);
            }
        }
        refresh();

        int ch = getch();
        if (ch == KEY_UP) {
            return -1;
        } else if (ch == KEY_DOWN) {
            return -1;
        } else if (ch == 10) {
            return selected;
        }
        return -2;
    }
};

int main() {
    Database db("questions.db");
    TUI tui(db);
    tui.run();
    return 0;
}
