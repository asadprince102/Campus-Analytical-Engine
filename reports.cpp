#include "reports.h"

#include "attendance.h"
#include "fee_tracker.h"
#include "filehandler.h"
#include "grades.h"
#include "student_ops.h"

#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace std;

static string formatDoubleForReports(double value) {
    stringstream out;
    out << fixed << setprecision(2) << value;
    return out.str();
}

void printMeritList() {
    vector<vector<string> > students = listActiveStudents();

    for (int i = 0; i < (int)students.size(); i++) {
        int highest = i;

        for (int j = i + 1; j < (int)students.size(); j++) {
            double a = atof(students[j][3].c_str());
            double b = atof(students[highest][3].c_str());

            if (a > b) {
                highest = j;
            }
        }

        if (highest != i) {
            vector<string> temp = students[i];
            students[i] = students[highest];
            students[highest] = temp;
        }
    }

    cout << left << setw(8) << "Rank" << setw(15) << "Roll" << setw(24) << "Name"
         << setw(12) << "Dept" << setw(8) << "CGPA" << endl;
    cout << "-------------------------------------------------------------------" << endl;

    for (int i = 0; i < (int)students.size(); i++) {
        cout << left << setw(8) << i + 1
             << setw(15) << students[i][0]
             << setw(24) << students[i][1]
             << setw(12) << students[i][2]
             << setw(8) << students[i][3] << endl;
    }
}

void printAttendanceDefaulters() {
    vector<vector<string> > shortage = getShortageList();

    cout << left << setw(15) << "Roll" << setw(24) << "Name"
         << setw(14) << "Course" << setw(12) << "Percent" << endl;
    cout << "-----------------------------------------------------------------" << endl;

    for (int i = 0; i < (int)shortage.size(); i++) {
        vector<string> student = searchByRoll(shortage[i][0]);
        string name = "Unknown";

        if (student.size() >= 2) {
            name = student[1];
        }

        cout << left << setw(15) << shortage[i][0]
             << setw(24) << name
             << setw(14) << shortage[i][1]
             << setw(12) << shortage[i][2] << endl;
    }
}

void printFeeDefaulters(const string& today) {
    vector<vector<string> > defaulters = getDefaulters(today);

    cout << left << setw(15) << "Roll" << setw(24) << "Name"
         << setw(16) << "Outstanding" << setw(10) << "Weeks" << endl;
    cout << "-----------------------------------------------------------------" << endl;

    for (int i = 0; i < (int)defaulters.size(); i++) {
        cout << left << setw(15) << defaulters[i][0]
             << setw(24) << defaulters[i][1]
             << setw(16) << defaulters[i][2]
             << setw(10) << defaulters[i][3] << endl;
    }
}

void printSemesterResult(const string& semester) {
    vector<vector<string> > grades = readTXT("grades.txt");

    cout << left << setw(15) << "Roll" << setw(22) << "Name"
         << setw(12) << "Course" << setw(10) << "Total"
         << setw(8) << "Grade" << setw(8) << "GPA" << setw(12) << "Attn" << endl;
    cout << "--------------------------------------------------------------------------------" << endl;

    for (int i = 0; i < (int)grades.size(); i++) {
        if ((int)grades[i].size() < 9 || grades[i][2] != semester) {
            continue;
        }

        vector<string> student = searchByRoll(grades[i][0]);
        string name = "Unknown";

        if (student.size() >= 2) {
            name = student[1];
        }

        double pct = getAttendancePct(grades[i][0], grades[i][1]);
        string attendanceStatus = "OK";

        if (pct < 75.0) {
            attendanceStatus = "Short";
        }

        cout << left << setw(15) << grades[i][0]
             << setw(22) << name
             << setw(12) << grades[i][1]
             << setw(10) << grades[i][7]
             << setw(8) << grades[i][8]
             << setw(8) << formatDoubleForReports(computeGPA(grades[i][0], semester))
             << setw(12) << attendanceStatus << endl;
    }
}

void printDepartmentSummary() {
    vector<vector<string> > students = listActiveStudents();
    vector<string> departments;
    vector<int> counts;
    vector<double> cgpaTotals;
    vector<int> passCounts;

    for (int i = 0; i < (int)students.size(); i++) {
        string dept = students[i][2];
        int index = -1;

        for (int j = 0; j < (int)departments.size(); j++) {
            if (departments[j] == dept) {
                index = j;
            }
        }

        if (index == -1) {
            departments.push_back(dept);
            counts.push_back(0);
            cgpaTotals.push_back(0.0);
            passCounts.push_back(0);
            index = (int)departments.size() - 1;
        }

        counts[index]++;
        cgpaTotals[index] += atof(students[i][3].c_str());

        if (atof(students[i][3].c_str()) >= 2.0) {
            passCounts[index]++;
        }
    }

    cout << left << setw(12) << "Dept" << setw(10) << "Count"
         << setw(12) << "Avg CGPA" << setw(12) << "Pass Rate" << endl;
    cout << "------------------------------------------------" << endl;

    for (int i = 0; i < (int)departments.size(); i++) {
        double avgCgpa = 0.0;
        double passRate = 0.0;

        if (counts[i] > 0) {
            avgCgpa = cgpaTotals[i] / counts[i];
            passRate = ((double)passCounts[i] / counts[i]) * 100.0;
        }

        cout << left << setw(12) << departments[i]
             << setw(10) << counts[i]
             << setw(12) << formatDoubleForReports(avgCgpa)
             << setw(12) << formatDoubleForReports(passRate) << endl;
    }
}

bool exportReportToFile(const string& reportName, const string& fileName, const string& argument) {
    ofstream file(fileName.c_str());

    if (!file.is_open()) {
        return false;
    }

    streambuf* oldBuffer = cout.rdbuf(file.rdbuf());

    if (reportName == "merit") {
        printMeritList();
    } else if (reportName == "attendance") {
        printAttendanceDefaulters();
    } else if (reportName == "fees") {
        printFeeDefaulters(argument);
    } else if (reportName == "semester") {
        printSemesterResult(argument);
    } else if (reportName == "department") {
        printDepartmentSummary();
    } else {
        cout.rdbuf(oldBuffer);
        file.close();
        return false;
    }

    cout.rdbuf(oldBuffer);
    file.close();
    return true;
}
