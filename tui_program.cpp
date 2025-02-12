#include <ncurses.h>
#include <vector>
#include <string>
#include "database.cpp" // Include the database header
#include <cstring> // Include for strlen
#include <cctype>  // Include for isdigit
#include <algorithm> // Include for remove_if

using namespace std;

#include "question.h" // Include the Question struct definition

class TUI {
public:
    void run() {
        questions = db.getQuestions(); // Load questions from the database on startup
        initscr(); // Initialize ncurses
        cbreak(); // Disable line buffering
        noecho(); // Don't echo input
        keypad(stdscr, TRUE); // Enable special keys
        mousemask(ALL_MOUSE_EVENTS, NULL); // Enable mouse events

        int choice = 0;
        vector<string> options = {"Add Question", "Show Questions", "Search Question", "Delete All Questions", "Exit"};
        while (true) {
            clear();
            printMenu(options, choice);
            int ch = getch();
            if (ch == KEY_UP) {
                choice = (choice - 1 + options.size()) % options.size();
            } else if (ch == KEY_DOWN) {
                choice = (choice + 1) % options.size();
            } else if (ch == 10) { // Enter key
                if (choice == 0) {
                    addQuestion();
                } else if (choice == 1) {
                    showQuestions();
                } else if (choice == 2) {
                    searchQuestion();
                } else if (choice == 3) {
                    deleteAllQuestions();
                } else if (choice == 4) {
                    clear(); // Clear the screen before exiting
                    printSubmittedCount(); // Print count of submitted questions
                    getch(); // Wait for user input before exiting
                    break; // Exit
                }
            } else if (ch == KEY_MOUSE) {
                MEVENT event;
                if (getmouse(&event) == OK) {
                    if (event.x >= 0 && event.x < COLS && event.y >= 0 && event.y < LINES) {
                        // Check if mouse click is on the menu options
                        if (event.y == 1) { // Assuming the first option is on the second line
                            addQuestion();
                        } else if (event.y == 2) { // Assuming the second option is on the third line
                            showQuestions();
                        } else if (event.y == 3) { // Assuming the third option is on the fourth line
                            searchQuestion();
                        } else if (event.y == 4) { // Assuming the fourth option is on the fifth line
                            clear(); // Clear the screen before exiting
                            printSubmittedCount(); // Print count of submitted questions
                            getch(); // Wait for user input before exiting
                            break; // Exit
                        }
                    }
                }
            }
        }

        endwin(); // End ncurses mode
    }

private:
    Database db{"questions.db"}; // Initialize the database
    vector<Question> questions; // Store questions with their statuses

    void printSubmittedCount() {
        int count = 0;
        for (const auto& question : questions) {
            if (question.status == "Submitted") {
                count++;
            }
        }
        mvprintw(1, 1, "Total Submitted Questions: %d", count);
    }

    void addQuestion() {
        char questionText[256]; // Initialize a character array with a buffer size
        char questionNumber[10]; // Initialize a character array for question number
        clear();
        
        // Prompt for question number
        while (true) {
            printw("Enter question number (numeric only): ");
            echo(); // Enable echoing of input characters
            getstr(questionNumber); // Read input into the buffer
            noecho(); // Disable echoing again

            // Validate that the input is numeric
            bool isValid = true;
            for (int i = 0; questionNumber[i] != '\0'; i++) {
                if (!isdigit(questionNumber[i])) {
                    isValid = false;
                    break;
                }
            }

            if (isValid) {
                break; // Valid input, exit the loop
            } else {
                showPopup("Invalid input. Please enter a numeric question number. Press any key to try again.");
            }
        }

        // Prompt for question text
        printw("Enter your question: ");
        echo(); // Enable echoing of input characters
        getstr(questionText); // Read input into the buffer
        noecho(); // Disable echoing again

        // Validate inputs
        if (strlen(questionText) == 0) {
            showPopup("Question text is required. Press any key to return.");
            return;
        }

        // Prompt for status
        string status;
        int statusChoice = 0;
        vector<string> statusOptions = {"Submitted", "Under Review", "Not Understood", "Cancel"};
        while (true) {
            clear();
            printw("Set status for the question:\n");
            for (size_t i = 0; i < statusOptions.size(); ++i) {
                if (i == statusChoice) {
                    attron(A_REVERSE); // Highlight selected option
                }
                printw("%d. %s\n", i + 1, statusOptions[i].c_str());
                if (i == statusChoice) {
                    attroff(A_REVERSE); // Remove highlight
                }
            }
            int ch = getch();
            if (ch == KEY_UP) {
                statusChoice = (statusChoice - 1 + statusOptions.size()) % statusOptions.size();
            } else if (ch == KEY_DOWN) {
                statusChoice = (statusChoice + 1) % statusOptions.size();
            } else if (ch == 10) { // Enter key
                if (statusChoice == 3) { // Cancel option
                    return; // Exit the function without saving
                }
                status = statusOptions[statusChoice];
                break; // Exit the loop if a valid choice is made
            } else if (ch == KEY_MOUSE) {
                MEVENT event;
                if (getmouse(&event) == OK) {
                    if (event.y >= 1 && event.y <= 4) { // Check if mouse click is on the status options
                        statusChoice = event.y - 1; // Set the choice based on the mouse click
                        if (statusChoice == 3) { // Cancel option
                            return; // Exit the function without saving
                        }
                        status = statusOptions[statusChoice];
                        break; // Exit the loop if a valid choice is made
                    }
                }
            }
        }

        // Store the question and its status
        db.addQuestion(string(questionNumber), string(questionText), status); // Store in database
        questions = db.getQuestions(); // Refresh the questions list to update the count
        showPopup("Question added: " + string(questionText) + "\nStatus: " + status);
    }

    void deleteAllQuestions() {
        clear();
        printw("Are you sure you want to delete all questions? (y/n): ");
        char confirm = getch();
        if (confirm == 'y' || confirm == 'Y') {
            db.deleteAllQuestionsFromDB(); // Assuming this method exists in the Database class
            questions = db.getQuestions(); // Refresh the questions list to update the count
            showPopup("All questions deleted successfully.");
            return; // Return to the menu after deletion
        } else {
            showPopup("Deletion canceled.");
        }
    }

    void showQuestions() {
        clear();
        questions = db.getQuestions(); // Retrieve questions from the database
        if (questions.empty()) {
            printw("No questions available. Press ESC to return to the menu.\n");
            getch();
            return;
        }

        // Display filter options
        int filterChoice = 0;
        vector<string> filterOptions = {"Show Submitted", "Show Under Review", "Show Not Understood", "Show All", "Cancel"};
        while (true) {
            clear();
            printw("Filter options:\n");
            for (size_t i = 0; i < filterOptions.size(); ++i) {
                if (i == filterChoice) {
                    attron(A_REVERSE); // Highlight selected option
                }
                printw("%d. %s\n", i + 1, filterOptions[i].c_str());
                if (i == filterChoice) {
                    attroff(A_REVERSE); // Remove highlight
                }
            }
            int ch = getch();
            if (ch == KEY_UP) {
                filterChoice = (filterChoice - 1 + filterOptions.size()) % filterOptions.size();
            } else if (ch == KEY_DOWN) {
                filterChoice = (filterChoice + 1) % filterOptions.size();
            } else if (ch == 10) { // Enter key
                if (filterChoice == 4) { // Cancel option
                    return; // Exit the function without saving
                }
                // Handle filtering based on the selected option
                if (filterChoice == 0) {
                    showFilteredQuestions("Submitted");
                } else if (filterChoice == 1) {
                    showFilteredQuestions("Under Review");
                } else if (filterChoice == 2) {
                    showFilteredQuestions("Not Understood");
                } else if (filterChoice == 3) {
                    showAllQuestions();
                }
                break; // Exit the loop after handling the choice
            } else if (ch == KEY_MOUSE) {
                MEVENT event;
                if (getmouse(&event) == OK) {
                    if (event.y >= 1 && event.y <= 5) { // Check if mouse click is on the filter options
                        filterChoice = event.y - 1; // Set the choice based on the mouse click
                        if (filterChoice == 4) { // Cancel option
                            return; // Exit the function without saving
                        }
                        // Handle filtering based on the selected option
                        if (filterChoice == 0) {
                            showFilteredQuestions("Submitted");
                        } else if (filterChoice == 1) {
                            showFilteredQuestions("Under Review");
                        } else if (filterChoice == 2) {
                            showFilteredQuestions("Not Understood");
                        } else if (filterChoice == 3) {
                            showAllQuestions();
                        }
                        break; // Exit the loop after handling the choice
                    }
                }
            }
        }
    }

    void showFilteredQuestions(const string& status) {
        clear();
        bool found = false;
        printw("Questions with status: %s\n", status.c_str()); // Show heading for filtered questions
        for (const auto& question : questions) {
            if (question.status == status) {
                printw("%s: %s\n", question.number.c_str(), question.text.c_str()); // Show question number and text
                found = true;
            }
        }
        if (!found) {
            printw("No questions with status: %s. Press ESC to return to the menu.\n", status.c_str());
        }
        printw("\nPress ESC to return to the menu...");
        while (getch() != 27); // Wait for ESC key
    }

    void showAllQuestions() {
        clear();
        if (questions.empty()) {
            printw("No questions available. Press ESC to return to the menu.\n");
        } else {
            printw("All Questions:\n"); // Show heading for all questions
            for (size_t i = 0; i < questions.size(); ++i) {
                printw("%s: %s | Status: %s\n", questions[i].number.c_str(), questions[i].text.c_str(), questions[i].status.c_str()); // Show question number, text, and status
            }
        }
        printw("\nPress ESC to return to the menu...");
        while (getch() != 27); // Wait for ESC key
    }

    void searchQuestion() {
        questions = db.getQuestions(); // Ensure questions are loaded before searching
        clear();
        if (questions.empty()) {
            printw("No questions available. Press ESC to return to the menu.\n");
            getch();
            return;
        }

        // Prompt for question number
        char searchNumber[10]; // Initialize a character array for search input
        printw("Enter question number to search: ");
        echo(); // Enable echoing of input characters
        getstr(searchNumber); // Read input into the buffer
        noecho(); // Disable echoing again

        // Search for the question
        bool found = false;
        Question foundQuestion;
        for (const auto& question : questions) {
            if (question.number == searchNumber) {
                foundQuestion = question; // Store found question
                found = true;
                break;
            }
        }

        if (!found) {
            printw("No question found with number: %s. Press ESC to return to the menu.\n", searchNumber);
        } else {
            clear();
            printw("Found Question:\n");
            printw("Number: %s\nText: %s\nStatus: %s\n", foundQuestion.number.c_str(), foundQuestion.text.c_str(), foundQuestion.status.c_str());
            printw("\nOptions:\n");
            vector<string> options = {"Update Question", "Delete Question", "Back to Menu"};
            int selected = 0;

            while (true) {
                for (size_t i = 0; i < options.size(); ++i) {
                    if (i == selected) {
                        attron(A_REVERSE); // Highlight selected option
                    }
                    printw("%d. %s\n", i + 1, options[i].c_str());
                    if (i == selected) {
                        attroff(A_REVERSE); // Remove highlight
                    }
                }

                int ch = getch();
                if (ch == KEY_UP) {
                    selected = (selected - 1 + options.size()) % options.size();
                } else if (ch == KEY_DOWN) {
                    selected = (selected + 1) % options.size();
                } else if (ch == 10) { // Enter key
                    if (selected == 0) {
                        clear();
                        printw("Updating Question:\n");
                        printw("Number: %s\nText: %s\nStatus: %s\n", foundQuestion.number.c_str(), foundQuestion.text.c_str(), foundQuestion.status.c_str());
                        updateQuestion(foundQuestion.number); // Pass the question number to update
                        return; // Back to Menu
                    } else if (selected == 1) {
                        clear();
                        printw("Deleting Question:\n");
                        printw("Number: %s\nText: %s\nStatus: %s\n", foundQuestion.number.c_str(), foundQuestion.text.c_str(), foundQuestion.status.c_str());
                        deleteQuestion(foundQuestion.number);
                        return; // Back to Menu
                    } else {
                        return; // Back to Menu
                    }
                } else if (ch == KEY_MOUSE) {
                    MEVENT event;
                    if (getmouse(&event) == OK) {
                        if (event.y >= 0 && event.y < options.size()) {
                            selected = event.y; // Set the selected option based on mouse click
                        }
                    }
                }
                clear(); // Clear the screen for the next iteration
            }
        }
    }

    void updateQuestion(const string& questionNumber) {
        string newStatus;
        clear();
        printw("Updating Question:\n");
        // Retrieve the question from the database
        for (const auto& question : questions) {
            if (question.number == questionNumber) {
                printw("Current Text: %s\n", question.text.c_str());

                // Prompt for new status
                int statusChoice = 0;
                vector<string> statusOptions = {"Submitted", "Under Review", "Not Understood", "Cancel"};
                while (true) {
                    clear();
                    printw("Set new status for the question:\n");
                    for (size_t i = 0; i < statusOptions.size(); ++i) {
                        if (i == statusChoice) {
                            attron(A_REVERSE); // Highlight selected option
                        }
                        printw("%d. %s\n", i + 1, statusOptions[i].c_str());
                        if (i == statusChoice) {
                            attroff(A_REVERSE); // Remove highlight
                        }
                    }
                    int ch = getch();
                    if (ch == KEY_UP) {
                        statusChoice = (statusChoice - 1 + statusOptions.size()) % statusOptions.size();
                    } else if (ch == KEY_DOWN) {
                        statusChoice = (statusChoice + 1) % statusOptions.size();
                    } else if (ch == 10) { // Enter key
                        if (statusChoice == 3) { // Cancel option
                            return; // Exit the function without saving
                        }
                        newStatus = statusOptions[statusChoice];
                        break; // Exit the loop if a valid choice is made
                    }
                }

                if (!newStatus.empty()) {
                    db.updateQuestionInDB(questionNumber, newStatus); // Update in database
                }

                showPopup("Question status updated to: " + newStatus);
                return; // Exit after updating
            }
        }
        showPopup("Question not found.");
    }

    void deleteQuestion(const string& questionNumber) {
        db.deleteQuestionFromDB(questionNumber); // Delete from database
        showPopup("Question deleted successfully.");
    }

    void showPopup(const string& message) {
        clear();
        mvprintw(1, 1, "%s", message.c_str()); // Display at fixed vertical position
        printw("\nPress any key to return to the menu...");
        getch();
    }

    void printMenu(const vector<string>& options, int selected) {
        for (size_t i = 0; i < options.size(); ++i) {
            if (i == selected) {
                attron(A_REVERSE); // Highlight selected option
            }
            mvprintw(i + 1, 1, options[i].c_str());
            if (i == selected) {
                attroff(A_REVERSE); // Remove highlight
            }
        }
    }
};

int main() {
    TUI tui;
    tui.run();
    return 0;
}
