#ifndef SCHEDULETOOLS_H
#define SCHEDULETOOLS_H

#include <QString>
#include <QList>
#include <QStandardItemModel>

enum ScheduleErrorType {
    SE_INCORRECT_ORDER = -2,
    SE_END_BEFORE_BEGIN,
    SE_CORRECT,
    SE_NO_BEGIN_AND_END
};

struct ScheduleError {
    ScheduleErrorType error_type;
    int error_row;
};

struct ScheduleEntry {
    unsigned char ls_hour, ls_minute;
    unsigned char le_hour, le_minute;
    unsigned char rings;
    unsigned char is_incorrect;
};

struct CalcModeScheduleEntry {
    unsigned char ls_hour, ls_minute;
    signed char lesson_delay, break_delay;
    unsigned char rings;
};

typedef QList<ScheduleEntry> Schedule;
typedef QList<CalcModeScheduleEntry> CalcModeSchedule;

ScheduleEntry cmse_to_se(CalcModeScheduleEntry cmse);
CalcModeScheduleEntry se_to_cmse(ScheduleEntry se);
CalcModeScheduleEntry se_to_cmse(ScheduleEntry se1, ScheduleEntry se2);

Schedule cmschedule_to_schedule(CalcModeSchedule cmsch);
CalcModeSchedule schedule_to_cmschedule(Schedule sch);

QString se_to_csv(ScheduleEntry se);
QString cmse_to_csv(CalcModeScheduleEntry cmse);

QString schedule_to_csv(Schedule sch);
QString cmschedule_to_csv(CalcModeSchedule cmsch);

Schedule csv_to_schedule(QString csv);
CalcModeSchedule csv_to_cmschedule(QString csv);

Schedule schedule_from_qstdim(QStandardItemModel* model);
CalcModeSchedule cmschedule_from_qstdim(QStandardItemModel* model);

void schedule_to_qstdim(QStandardItemModel** model, Schedule sch);
void cmschedule_to_qstdim(QStandardItemModel** model, CalcModeSchedule cmsch);

ScheduleError check_schedule(Schedule sch);

#endif // SCHEDULETOOLS_H
