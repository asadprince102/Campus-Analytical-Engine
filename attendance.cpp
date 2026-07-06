#include "attendance.h"

#include "course_ops.h"
#include "filehandler.h"
#include "student_ops.h"

#include <iomanip>
#include <iostream>
#include <sstream>

using namespace std;

const string ATTENDANCE_FILE = "attendance_log.txt";
const string ENROLLMENTS_FILE_FOR_ATTENDANCE = "enrollments.txt";

static vector<vector<string> > backupRows;
static bool hasBackup = false;

static vector<string> attendanceHeader() {
    vector<string> header;
    header.push_back("roll");
    header.push_back("course_code");
    header.push_back("date");
    header.push_back("status");
    return header;
}

static string formatPercent(double value) {
    stringstream out;
    out << fixed << setprecision(2) << value;
    return out.str();
}

bool markAttendance(const string& courseCode, const string& semester, const string& date) {
    vector<vector<string> > students = listEnrolledStudents(courseCode, semester);

    if (students.size() == 0) {
        return false;
    }

    backupRows = readTXT(ATTENDANCE_FILE);
    hasBackup = true;

    for (int i = 0; i < (int)students.size(); i++) {
        string status;

        do {
            cout << students[i][0] << " - " << students[i][1] << " (P/A/L): ";
            cin >> status;
        } while (status != "P" && status != "A" && status != "L");

        vector<string> row;
        row.push_back(students[i][0]);
        row.push_back(courseCode);
        row.push_back(date);
        row.push_back(status);
        appendTXT(ATTENDANCE_FILE, row);
    }

    return true;
}

double getAttendancePct(const string& roll, const string& courseCode) {
    vector<vector<string> > rows = readTXT(ATTENDANCE_FILE);
    double presentScore = 0.0;
    int totalSessions = 0;

    for (int i = 0; i < (int)rows.size(); i++) {
        if ((int)rows[i].size() >= 4 &&
            rows[i][0] == roll &&
            rows[i][1] == courseCode) {
            totalSessions++;

            if (rows[i][3] == "P") {
                presentScore += 1.0;
            } else if (rows[i][3] == "L") {
                presentScore += 0.5;
            }
        }
    }

    if (totalSessions == 0) {
        return 0.0;
    }

    return (presentScore / totalSessions) * 100.0;
}

static vector<vector<string> > shortageListByCourse(const string& courseCode, bool useFilter) {
    vector<vector<string> > enrollments = readTXT(ENROLLMENTS_FILE_FOR_ATTENDANCE);
    vector<vector<string> > shortage;

    for (int i = 0; i < (int)enrollments.size(); i++) {
        if ((int)enrollments[i].size() < 4 || enrollments[i][3] != "enrolled") {
            continue;
        }

        if (useFilter && enrollments[i][1] != courseCode) {
            continue;
        }

        bool alreadyChecked = false;
        for (int j = 0; j < i; j++) {
            if ((int)enrollments[j].size() >= 2 &&
                enrollments[j][0] == enrollments[i][0] &&
                enrollments[j][1] == enrollments[i][1]) {
                alreadyChecked = true;
            }
        }

        if (alreadyChecked) {
            continue;
        }

        double pct = getAttendancePct(enrollments[i][0], enrollments[i][1]);

        if (pct < 75.0) {
            vector<string> row;
            row.push_back(enrollments[i][0]);
            row.push_back(enrollments[i][1]);
            row.push_back(formatPercent(pct));
            shortage.push_back(row);
        }
    }

    return shortage;
}

vector<vector<string> > getShortageList() {
    return shortageListByCourse("", false);
}

vector<vector<string> > getShortageList(const string& courseCode) {
    return shortageListByCourse(courseCode, true);
}

bool undoLastSession() {
    if (!hasBackup) {
        return false;
    }

    writeTXT(ATTENDANCE_FILE, attendanceHeader(), backupRows);
    hasBackup = false;
    return true;
}

void printDailySheet(const string& courseCode, const string& date) {
    vector<vector<string> > enrollments = readTXT(ENROLLMENTS_FILE_FOR_ATTENDANCE);
    vector<vector<string> > attendance = readTXT(ATTENDANCE_FILE);

    cout << left << setw(15) << "Roll" << setw(22) << "Name" << setw(12) << "Course" << setw(8) << "Status" << endl;
    cout << "----------------------------------------------------------" << endl;

    for (int i = 0; i < (int)enrollments.size(); i++) {
        if ((int)enrollments[i].size() < 4 ||
            enrollments[i][1] != courseCode ||
            enrollments[i][3] != "enrolled") {
            continue;
        }

        vector<string> student = searchByRoll(enrollments[i][0]);
        if (student.size() < 5 || student[4] != "active") {
            continue;
        }

        string status = "-";
        for (int j = 0; j < (int)attendance.size(); j++) {
            if ((int)attendance[j].size() >= 4 &&
                attendance[j][0] == enrollments[i][0] &&
                attendance[j][1] == courseCode &&
                attendance[j][2] == date) {
                status = attendance[j][3];
            }
        }

        cout << left << setw(15) << student[0]
             << setw(22) << student[1]
             << setw(12) << courseCode
             << setw(8) << status << endl;
    }
}
