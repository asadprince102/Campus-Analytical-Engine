#include "student_ops.h"

#include "filehandler.h"

#include <cctype>
#include <cstdlib>
#include <iomanip>
#include <sstream>

using namespace std;

const string STUDENTS_FILE = "students.txt";

static vector<string> studentHeader() {
    vector<string> header;
    header.push_back("roll");
    header.push_back("name");
    header.push_back("dept");
    header.push_back("cgpa");
    header.push_back("status");
    return header;
}

static string formatDouble(double value) {
    stringstream out;
    out << fixed << setprecision(2) << value;
    return out.str();
}

static string lowerText(const string& text) {
    string lowered = text;

    for (int i = 0; i < (int)lowered.length(); i++) {
        lowered[i] = (char)tolower((unsigned char)lowered[i]);
    }

    return lowered;
}

static bool containsText(const string& source, const string& target) {
    string a = lowerText(source);
    string b = lowerText(target);

    if (b.length() == 0) {
        return true;
    }

    if (b.length() > a.length()) {
        return false;
    }

    for (int i = 0; i <= (int)a.length() - (int)b.length(); i++) {
        bool same = true;

        for (int j = 0; j < (int)b.length(); j++) {
            if (a[i + j] != b[j]) {
                same = false;
                break;
            }
        }

        if (same) {
            return true;
        }
    }

    return false;
}

bool isValidRoll(const string& roll) {
    if (roll.length() != 11) {
        return false;
    }

    if (roll.substr(0, 4) != "BSAI") {
        return false;
    }

    if (roll[4] != '-' || roll[7] != '-') {
        return false;
    }

    for (int i = 5; i <= 6; i++) {
        if (!isdigit((unsigned char)roll[i])) {
            return false;
        }
    }

    for (int i = 8; i <= 10; i++) {
        if (!isdigit((unsigned char)roll[i])) {
            return false;
        }
    }

    return true;
}

bool nameHasNoDigits(const string& name) {
    if (name.length() == 0) {
        return false;
    }

    for (int i = 0; i < (int)name.length(); i++) {
        if (isdigit((unsigned char)name[i])) {
            return false;
        }
    }

    return true;
}

bool addStudent(const string& roll, const string& name, const string& dept, double cgpa) {
    if (!isValidRoll(roll) || !nameHasNoDigits(name) || dept.length() == 0) {
        return false;
    }

    if (cgpa < 0.0 || cgpa > 4.0) {
        return false;
    }

    if (rowExists(STUDENTS_FILE, 0, roll)) {
        return false;
    }

    vector<string> row;
    row.push_back(roll);
    row.push_back(name);
    row.push_back(dept);
    row.push_back(formatDouble(cgpa));
    row.push_back("active");

    return appendTXT(STUDENTS_FILE, row);
}

vector<string> searchByRoll(const string& roll) {
    return findRow(STUDENTS_FILE, 0, roll);
}

vector<vector<string> > searchByName(const string& namePart) {
    vector<vector<string> > rows = readTXT(STUDENTS_FILE);
    vector<vector<string> > matches;

    for (int i = 0; i < (int)rows.size(); i++) {
        if ((int)rows[i].size() >= 2 && containsText(rows[i][1], namePart)) {
            matches.push_back(rows[i]);
        }
    }

    return matches;
}

bool updateStudent(const string& roll, int fieldIndex, const string& newValue) {
    if (fieldIndex <= 0 || fieldIndex > 4) {
        return false;
    }

    if (fieldIndex == 1 && !nameHasNoDigits(newValue)) {
        return false;
    }

    if (fieldIndex == 3) {
        double cgpa = atof(newValue.c_str());
        if (cgpa < 0.0 || cgpa > 4.0) {
            return false;
        }
    }

    if (fieldIndex == 4 && newValue != "active" && newValue != "inactive") {
        return false;
    }

    vector<vector<string> > rows = readTXT(STUDENTS_FILE);

    for (int i = 0; i < (int)rows.size(); i++) {
        if ((int)rows[i].size() > fieldIndex && rows[i][0] == roll) {
            rows[i][fieldIndex] = newValue;
            writeTXT(STUDENTS_FILE, studentHeader(), rows);
            return true;
        }
    }

    return false;
}

bool softDelete(const string& roll) {
    return updateStudent(roll, 4, "inactive");
}

vector<vector<string> > listActiveStudents() {
    vector<vector<string> > rows = readTXT(STUDENTS_FILE);
    vector<vector<string> > active;

    for (int i = 0; i < (int)rows.size(); i++) {
        if ((int)rows[i].size() >= 5 && rows[i][4] == "active") {
            active.push_back(rows[i]);
        }
    }

    for (int i = 0; i < (int)active.size(); i++) {
        int smallest = i;

        for (int j = i + 1; j < (int)active.size(); j++) {
            if (active[j][0] < active[smallest][0]) {
                smallest = j;
            }
        }

        if (smallest != i) {
            vector<string> temp = active[i];
            active[i] = active[smallest];
            active[smallest] = temp;
        }
    }

    return active;
}

bool isActiveStudent(const string& roll) {
    vector<string> row = searchByRoll(roll);
    return row.size() >= 5 && row[4] == "active";
}
