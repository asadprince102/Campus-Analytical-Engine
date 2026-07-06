#ifndef ATTENDANCE_H
#define ATTENDANCE_H

#include <string>
#include <vector>

bool markAttendance(const std::string& courseCode,
                    const std::string& semester,
                    const std::string& date);
double getAttendancePct(const std::string& roll, const std::string& courseCode);
std::vector<std::vector<std::string> > getShortageList();
std::vector<std::vector<std::string> > getShortageList(const std::string& courseCode);
bool undoLastSession();
void printDailySheet(const std::string& courseCode, const std::string& date);

#endif
