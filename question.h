#ifndef QUESTION_H
#define QUESTION_H

#include <string>

struct Question {
    std::string text;
    std::string status; // Status can be "Submitted", "Under Review", or "Not Understood"
    std::string number; // Question number
};

#endif // QUESTION_H
