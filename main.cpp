#include "attendance.h"
#include "course_ops.h"
#include "fee_tracker.h"
#include "grades.h"
#include "reports.h"
#include "student_ops.h"

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

static void clearInputLine() {
    cin.ignore(10000, '\n');
}

static string askText(const string& prompt) {
    string value;
    cout << prompt;
    getline(cin, value);
    return value;
}

static double askDouble(const string& prompt) {
    double value;
    cout << prompt;
    cin >> value;
    clearInputLine();
    return value;
}

static void printStudentRows(const vector<vector<string> >& rows) {
    cout << left << setw(15) << "Roll" << setw(24) << "Name"
         << setw(12) << "Dept" << setw(8) << "CGPA" << setw(10) << "Status" << endl;
    cout << "---------------------------------------------------------------------" << endl;

    for (int i = 0; i < (int)rows.size(); i++) {
        if ((int)rows[i].size() >= 5) {
            cout << left << setw(15) << rows[i][0]
                 << setw(24) << rows[i][1]
                 << setw(12) << rows[i][2]
                 << setw(8) << rows[i][3]
                 << setw(10) << rows[i][4] << endl;
        }
    }
}

static void studentMenu() {
    int choice = -1;

    while (choice != 0) {
        cout << "\nStudent Operations\n";
        cout << "1. Add student\n";
        cout << "2. Search by roll\n";
        cout << "3. Search by name\n";
        cout << "4. Update student\n";
        cout << "5. Soft delete student\n";
        cout << "6. List active students\n";
        cout << "0. Back\n";
        cout << "Choice: ";
        cin >> choice;
        clearInputLine();

        if (choice == 1) {
            string roll = askText("Roll: ");
            string name = askText("Name: ");
            string dept = askText("Department: ");
            double cgpa = askDouble("CGPA: ");
            cout << (addStudent(roll, name, dept, cgpa) ? "Student added.\n" : "Could not add student.\n");
        } else if (choice == 2) {
            string roll = askText("Roll: ");
            vector<vector<string> > result;
            vector<string> student = searchByRoll(roll);
            if (student.size() > 0) {
                result.push_back(student);
            }
            printStudentRows(result);
        } else if (choice == 3) {
            string name = askText("Name text: ");
            printStudentRows(searchByName(name));
        } else if (choice == 4) {
            string roll = askText("Roll: ");
            int field;
            cout << "Field 1=name, 2=dept, 3=cgpa, 4=status: ";
            cin >> field;
            clearInputLine();
            string value = askText("New value: ");
            cout << (updateStudent(roll, field, value) ? "Student updated.\n" : "Update failed.\n");
        } else if (choice == 5) {
            string roll = askText("Roll: ");
            cout << (softDelete(roll) ? "Student marked inactive.\n" : "Soft delete failed.\n");
        } else if (choice == 6) {
            printStudentRows(listActiveStudents());
        }
    }
}

static void courseMenu() {
    int choice = -1;

    while (choice != 0) {
        cout << "\nCourse Operations\n";
        cout << "1. Enroll student\n";
        cout << "2. Drop course\n";
        cout << "3. Credit load\n";
        cout << "4. Check prerequisite\n";
        cout << "5. List enrolled students\n";
        cout << "0. Back\n";
        cout << "Choice: ";
        cin >> choice;
        clearInputLine();

        if (choice == 1) {
            string roll = askText("Roll: ");
            string course = askText("Course code: ");
            string semester = askText("Semester: ");
            EnrollResult result = enrollStudent(roll, course, semester);
            cout << enrollResultMessage(result) << endl;
        } else if (choice == 2) {
            string roll = askText("Roll: ");
            string course = askText("Course code: ");
            string semester = askText("Semester: ");
            cout << (dropCourse(roll, course, semester) ? "Course dropped.\n" : "Drop not allowed or record missing.\n");
        } else if (choice == 3) {
            string roll = askText("Roll: ");
            string semester = askText("Semester: ");
            cout << "Credit load: " << getCreditLoad(roll, semester) << endl;
        } else if (choice == 4) {
            string roll = askText("Roll: ");
            string course = askText("Course code: ");
            cout << (checkPrerequisite(roll, course) ? "Prerequisite satisfied.\n" : "Prerequisite not satisfied.\n");
        } else if (choice == 5) {
            string course = askText("Course code: ");
            string semester = askText("Semester: ");
            printStudentRows(listEnrolledStudents(course, semester));
        }
    }
}

static void attendanceMenu() {
    int choice = -1;

    while (choice != 0) {
        cout << "\nAttendance\n";
        cout << "1. Mark attendance\n";
        cout << "2. Attendance percentage\n";
        cout << "3. Shortage list\n";
        cout << "4. Undo last session\n";
        cout << "5. Print daily sheet\n";
        cout << "0. Back\n";
        cout << "Choice: ";
        cin >> choice;
        clearInputLine();

        if (choice == 1) {
            string course = askText("Course code: ");
            string semester = askText("Semester: ");
            string date = askText("Date DD-MM-YYYY: ");
            cout << (markAttendance(course, semester, date) ? "Attendance saved.\n" : "No enrolled students found.\n");
        } else if (choice == 2) {
            string roll = askText("Roll: ");
            string course = askText("Course code: ");
            cout << fixed << setprecision(2) << getAttendancePct(roll, course) << "%\n";
        } else if (choice == 3) {
            vector<vector<string> > rows = getShortageList();
            cout << left << setw(15) << "Roll" << setw(14) << "Course" << setw(10) << "Percent" << endl;
            for (int i = 0; i < (int)rows.size(); i++) {
                cout << left << setw(15) << rows[i][0] << setw(14) << rows[i][1] << setw(10) << rows[i][2] << endl;
            }
        } else if (choice == 4) {
            cout << (undoLastSession() ? "Last attendance session undone.\n" : "No backup found.\n");
        } else if (choice == 5) {
            string course = askText("Course code: ");
            string date = askText("Date DD-MM-YYYY: ");
            printDailySheet(course, date);
        }
    }
}

static void gradesMenu() {
    int choice = -1;

    while (choice != 0) {
        cout << "\nGrades\n";
        cout << "1. Enter marks\n";
        cout << "2. Semester GPA\n";
        cout << "3. Class statistics\n";
        cout << "0. Back\n";
        cout << "Choice: ";
        cin >> choice;
        clearInputLine();

        if (choice == 1) {
            string roll = askText("Roll: ");
            string course = askText("Course code: ");
            string semester = askText("Semester: ");
            cout << (enterMarks(roll, course, semester) ? "Marks saved.\n" : "Marks rejected.\n");
        } else if (choice == 2) {
            string roll = askText("Roll: ");
            string semester = askText("Semester: ");
            cout << fixed << setprecision(2) << "GPA: " << computeGPA(roll, semester) << endl;
        } else if (choice == 3) {
            string course = askText("Course code: ");
            string semester = askText("Semester: ");
            Stats stats = computeClassState(course, semester);
            cout << fixed << setprecision(2);
            cout << "Highest: " << stats.highest << endl;
            cout << "Lowest : " << stats.lowest << endl;
            cout << "Mean   : " << stats.mean << endl;
            cout << "Median : " << stats.median << endl;
        }
    }
}

static void feeMenu() {
    int choice = -1;

    while (choice != 0) {
        cout << "\nFee Tracker\n";
        cout << "1. Record payment\n";
        cout << "2. Late fine\n";
        cout << "3. Receipt\n";
        cout << "4. Defaulters\n";
        cout << "0. Back\n";
        cout << "Choice: ";
        cin >> choice;
        clearInputLine();

        if (choice == 1) {
            string roll = askText("Roll: ");
            string semester = askText("Semester: ");
            double amount = askDouble("Payment amount: ");
            string date = askText("Paid date DD-MM-YYYY: ");
            cout << (recordPayment(roll, semester, amount, date) ? "Payment saved.\n" : "Payment failed.\n");
        } else if (choice == 2) {
            string roll = askText("Roll: ");
            string semester = askText("Semester: ");
            cout << fixed << setprecision(2) << "Late fine: " << computeLateFine(roll, semester) << endl;
        } else if (choice == 3) {
            string roll = askText("Roll: ");
            string semester = askText("Semester: ");
            generateReceipt(roll, semester);
        } else if (choice == 4) {
            string today = askText("Today DD-MM-YYYY: ");
            vector<vector<string> > rows = getDefaulters(today);
            cout << left << setw(15) << "Roll" << setw(24) << "Name"
                 << setw(16) << "Balance" << setw(10) << "Weeks" << endl;
            for (int i = 0; i < (int)rows.size(); i++) {
                cout << left << setw(15) << rows[i][0]
                     << setw(24) << rows[i][1]
                     << setw(16) << rows[i][2]
                     << setw(10) << rows[i][3] << endl;
            }
        }
    }
}

static void reportsMenu() {
    int choice = -1;

    while (choice != 0) {
        cout << "\nReports\n";
        cout << "1. Merit list\n";
        cout << "2. Attendance defaulters\n";
        cout << "3. Fee defaulters\n";
        cout << "4. Semester result\n";
        cout << "5. Department summary\n";
        cout << "6. Export report to file\n";
        cout << "0. Back\n";
        cout << "Choice: ";
        cin >> choice;
        clearInputLine();

        if (choice == 1) {
            printMeritList();
        } else if (choice == 2) {
            printAttendanceDefaulters();
        } else if (choice == 3) {
            string today = askText("Today DD-MM-YYYY: ");
            printFeeDefaulters(today);
        } else if (choice == 4) {
            string semester = askText("Semester: ");
            printSemesterResult(semester);
        } else if (choice == 5) {
            printDepartmentSummary();
        } else if (choice == 6) {
            string report = askText("Report name merit/attendance/fees/semester/department: ");
            string fileName = askText("Output txt file name: ");
            string argument = askText("Argument if needed (today or semester): ");
            cout << (exportReportToFile(report, fileName, argument) ? "Report exported.\n" : "Export failed.\n");
        }
    }
}

static bool prefixMatches(const string& value, const string& prefix) {
    if (prefix.length() > value.length()) {
        return false;
    }

    return value.substr(0, prefix.length()) == prefix;
}

static void bonusSearchAsYouType() {
    string prefix = "";
    string entry;

    cout << "Type one character per line. Use # to stop and < to remove last character.\n";

    while (true) {
        cout << "Current prefix [" << prefix << "] next char: ";
        getline(cin, entry);

        if (entry.length() == 0) {
            continue;
        }

        char ch = entry[0];
        if (ch == '#') {
            break;
        }

        if (ch == '<') {
            if (prefix.length() > 0) {
                prefix = prefix.substr(0, prefix.length() - 1);
            }
        } else {
            prefix += ch;
        }

        vector<vector<string> > students = listActiveStudents();
        vector<vector<string> > matches;

        for (int i = 0; i < (int)students.size(); i++) {
            if ((int)students[i].size() >= 2 && prefixMatches(students[i][1], prefix)) {
                matches.push_back(students[i]);
            }
        }

        printStudentRows(matches);
    }
}

int main() {
    int choice = -1;

    while (choice != 0) {
        cout << "\nCampus Analytics Engine\n";
        cout << "1. Student operations\n";
        cout << "2. Course operations\n";
        cout << "3. Attendance\n";
        cout << "4. Grades\n";
        cout << "5. Fee tracker\n";
        cout << "6. Reports\n";
        cout << "7. Bonus search as you type\n";
        cout << "0. Exit\n";
        cout << "Choice: ";
        cin >> choice;
        clearInputLine();

        if (choice == 1) {
            studentMenu();
        } else if (choice == 2) {
            courseMenu();
        } else if (choice == 3) {
            attendanceMenu();
        } else if (choice == 4) {
            gradesMenu();
        } else if (choice == 5) {
            feeMenu();
        } else if (choice == 6) {
            reportsMenu();
        } else if (choice == 7) {
            bonusSearchAsYouType();
        }
    }

    return 0;
}
