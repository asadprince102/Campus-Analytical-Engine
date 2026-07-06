#include "grades.h"

#include "attendance.h"
#include "filehandler.h"
#include "student_ops.h"

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace std;

const string GRADES_FILE = "grades.txt";
const string COURSES_FILE_FOR_GRADES = "courses.txt";

static vector<string> gradesHeader() {
    vector<string> header;
    header.push_back("roll");
    header.push_back("course_code");
    header.push_back("semester");
    header.push_back("quiz_avg");
    header.push_back("assignment_avg");
    header.push_back("mid");
    header.push_back("final");
    header.push_back("total");
    header.push_back("letter");
    header.push_back("points");
    header.push_back("credits");
    return header;
}

static string formatDouble(double value) {
    stringstream out;
    out << fixed << setprecision(2) << value;
    return out.str();
}

static double averageMarks(const vector<double>& marks) {
    if (marks.size() == 0) {
        return 0.0;
    }

    double total = 0.0;
    for (int i = 0; i < (int)marks.size(); i++) {
        total += marks[i];
    }

    return total / marks.size();
}

static int courseCreditsFromFile(const string& courseCode) {
    vector<string> course = findRow(COURSES_FILE_FOR_GRADES, 0, courseCode);

    if (course.size() < 3) {
        return 0;
    }

    return atoi(course[2].c_str());
}

static bool allMarksInRange(const vector<double>& marks, double maxValue) {
    for (int i = 0; i < (int)marks.size(); i++) {
        if (marks[i] < 0.0 || marks[i] > maxValue) {
            return false;
        }
    }

    return true;
}

bool enterMarks(const string& roll, const string& courseCode, const string& semester) {
    int quizCount;
    int assignmentCount;
    double mid;
    double finalExam;
    vector<double> quizzes;
    vector<double> assignments;

    cout << "How many quizzes? (0-5): ";
    cin >> quizCount;

    if (quizCount < 0 || quizCount > 5) {
        return false;
    }

    for (int i = 0; i < quizCount; i++) {
        double mark;
        cout << "Quiz " << i + 1 << " out of 100: ";
        cin >> mark;
        quizzes.push_back(mark);
    }

    cout << "How many assignments? ";
    cin >> assignmentCount;

    if (assignmentCount < 0) {
        return false;
    }

    for (int i = 0; i < assignmentCount; i++) {
        double mark;
        cout << "Assignment " << i + 1 << " out of 100: ";
        cin >> mark;
        assignments.push_back(mark);
    }

    cout << "Mid marks out of 40: ";
    cin >> mid;
    cout << "Final marks out of 60: ";
    cin >> finalExam;

    return saveMarks(roll, courseCode, semester, quizzes, assignments, mid, finalExam);
}

bool saveMarks(const string& roll,
               const string& courseCode,
               const string& semester,
               const vector<double>& quizzes,
               const vector<double>& assignments,
               double mid,
               double finalExam) {
    vector<string> student = searchByRoll(roll);

    if (student.size() < 5 || student[4] != "active") {
        return false;
    }

    if (quizzes.size() > 5 || !allMarksInRange(quizzes, 100.0) || !allMarksInRange(assignments, 100.0)) {
        return false;
    }

    if (mid < 0.0 || mid > 40.0 || finalExam < 0.0 || finalExam > 60.0) {
        return false;
    }

    double quizAvg = bestThreeOfFive(quizzes);
    double assignmentAvg = averageMarks(assignments);
    double midPercent = (mid / 40.0) * 100.0;
    double finalPercent = (finalExam / 60.0) * 100.0;
    double total = computeWeightedTotal(quizAvg, assignmentAvg, midPercent, finalPercent);
    string letter = getLetterGrade(total);
    letter = applyAttendancePenalty(roll, courseCode, letter);
    double points = gradePointForLetter(letter);
    int credits = courseCreditsFromFile(courseCode);

    vector<string> row;
    row.push_back(roll);
    row.push_back(courseCode);
    row.push_back(semester);
    row.push_back(formatDouble(quizAvg));
    row.push_back(formatDouble(assignmentAvg));
    row.push_back(formatDouble(mid));
    row.push_back(formatDouble(finalExam));
    row.push_back(formatDouble(total));
    row.push_back(letter);
    row.push_back(formatDouble(points));
    row.push_back(formatDouble((double)credits));

    vector<vector<string> > grades = readTXT(GRADES_FILE);
    bool updated = false;

    for (int i = 0; i < (int)grades.size(); i++) {
        if ((int)grades[i].size() >= 3 &&
            grades[i][0] == roll &&
            grades[i][1] == courseCode &&
            grades[i][2] == semester) {
            grades[i] = row;
            updated = true;
        }
    }

    if (!updated) {
        grades.push_back(row);
    }

    writeTXT(GRADES_FILE, gradesHeader(), grades);
    return true;
}

double bestThreeOfFive(const vector<double>& marks) {
    if (marks.size() == 0) {
        return 0.0;
    }

    if (marks.size() < 3) {
        return averageMarks(marks);
    }

    int lowOne = -1;
    int lowTwo = -1;

    for (int i = 0; i < (int)marks.size(); i++) {
        if (lowOne == -1 || marks[i] < marks[lowOne]) {
            lowTwo = lowOne;
            lowOne = i;
        } else if (lowTwo == -1 || marks[i] < marks[lowTwo]) {
            lowTwo = i;
        }
    }

    double total = 0.0;
    int count = 0;

    for (int i = 0; i < (int)marks.size(); i++) {
        if (i != lowOne && i != lowTwo) {
            total += marks[i];
            count++;
        }
    }

    if (count == 0) {
        return 0.0;
    }

    return total / count;
}

double computeWeightedTotal(double quizAvg, double assignmentAvg, double midPercent, double finalPercent) {
    return quizAvg * 0.10 + assignmentAvg * 0.10 + midPercent * 0.30 + finalPercent * 0.50;
}

string getLetterGrade(double total) {
    if (total >= 85.0) {
        return "A";
    }
    if (total >= 80.0) {
        return "B+";
    }
    if (total >= 70.0) {
        return "B";
    }
    if (total >= 65.0) {
        return "C+";
    }
    if (total >= 60.0) {
        return "C";
    }
    if (total >= 50.0) {
        return "D";
    }

    return "F";
}

double gradePointForLetter(const string& letter) {
    if (letter == "A") {
        return 4.0;
    }
    if (letter == "B+") {
        return 3.3;
    }
    if (letter == "B") {
        return 3.0;
    }
    if (letter == "C+") {
        return 2.3;
    }
    if (letter == "C") {
        return 2.0;
    }
    if (letter == "D") {
        return 1.0;
    }

    return 0.0;
}

double computeGPA(const string& roll, const string& semester) {
    vector<vector<string> > grades = readTXT(GRADES_FILE);
    double weightedPoints = 0.0;
    int totalCredits = 0;

    for (int i = 0; i < (int)grades.size(); i++) {
        if ((int)grades[i].size() >= 10 &&
            grades[i][0] == roll &&
            grades[i][2] == semester) {
            double points = atof(grades[i][9].c_str());
            int credits = 0;

            if ((int)grades[i].size() >= 11) {
                credits = atoi(grades[i][10].c_str());
            } else {
                credits = courseCreditsFromFile(grades[i][1]);
            }

            weightedPoints += points * credits;
            totalCredits += credits;
        }
    }

    if (totalCredits == 0) {
        return 0.0;
    }

    return weightedPoints / totalCredits;
}

Stats computeClassState(const string& courseCode, const string& semester) {
    vector<vector<string> > grades = readTXT(GRADES_FILE);
    vector<double> totals;
    Stats stats;
    stats.highest = 0.0;
    stats.lowest = 0.0;
    stats.mean = 0.0;
    stats.median = 0.0;

    for (int i = 0; i < (int)grades.size(); i++) {
        if ((int)grades[i].size() >= 8 &&
            grades[i][1] == courseCode &&
            grades[i][2] == semester) {
            totals.push_back(atof(grades[i][7].c_str()));
        }
    }

    if (totals.size() == 0) {
        return stats;
    }

    for (int i = 0; i < (int)totals.size(); i++) {
        int smallest = i;

        for (int j = i + 1; j < (int)totals.size(); j++) {
            if (totals[j] < totals[smallest]) {
                smallest = j;
            }
        }

        if (smallest != i) {
            double temp = totals[i];
            totals[i] = totals[smallest];
            totals[smallest] = temp;
        }
    }

    double sum = 0.0;
    for (int i = 0; i < (int)totals.size(); i++) {
        sum += totals[i];
    }

    stats.lowest = totals[0];
    stats.highest = totals[totals.size() - 1];
    stats.mean = sum / totals.size();

    int middle = (int)totals.size() / 2;
    if (totals.size() % 2 == 0) {
        stats.median = (totals[middle - 1] + totals[middle]) / 2.0;
    } else {
        stats.median = totals[middle];
    }

    return stats;
}

string applyAttendancePenalty(const string& roll, const string& courseCode, const string& letter) {
    double pct = getAttendancePct(roll, courseCode);

    if (pct < 75.0) {
        return "F";
    }

    return letter;
}
