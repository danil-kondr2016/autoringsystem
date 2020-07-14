#include "scheduletools.h"
#include <QString>
#include <QStandardItemModel>
#include <QTime>

ScheduleEntry cmse_to_se(CalcModeScheduleEntry cmse)
{
    ScheduleEntry se;
    int ls, le;

    se.ls_hour = cmse.ls_hour;
    se.ls_minute = cmse.ls_minute;

    ls = static_cast<int>(se.ls_hour) * 60 + se.ls_minute;
    le = ls + cmse.lesson_delay;

    se.le_hour = static_cast<unsigned char>(le / 60);
    se.le_minute = le % 60;

    se.rings = cmse.rings;

    return se;
}

CalcModeScheduleEntry se_to_cmse(ScheduleEntry se)
{
    CalcModeScheduleEntry cmse;
    int ls, le;

    ls = static_cast<int>(se.ls_hour) * 60 + se.ls_minute;
    le = static_cast<int>(se.le_hour) * 60 + se.le_minute;

    cmse.ls_hour = se.ls_hour;
    cmse.ls_minute = se.ls_minute;

    cmse.lesson_delay = static_cast<signed char>(le - ls);
    cmse.break_delay = 0;

    cmse.rings = se.rings;

    return cmse;
}

CalcModeScheduleEntry se_to_cmse(ScheduleEntry se1, ScheduleEntry se2)
{
    CalcModeScheduleEntry cmse;
    int ls, le, ls1;

    ls = static_cast<signed char>(se1.ls_hour * 60 + se1.ls_minute);
    le = static_cast<signed char>(se1.le_hour * 60 + se1.le_minute);
    ls1 = static_cast<signed char>(se2.ls_hour * 60 + se2.ls_minute);

    cmse.ls_hour = se1.ls_hour;
    cmse.ls_minute = se1.ls_minute;

    cmse.lesson_delay = static_cast<signed char>(le - ls);
    cmse.break_delay = static_cast<signed char>(ls1 - le);

    cmse.rings = se1.rings;

    return cmse;
}

Schedule cmschedule_to_schedule(CalcModeSchedule cmsch)
{
    Schedule sch;

    for (int i = 0; i < cmsch.length(); i++) {
        sch.append(cmse_to_se(cmsch[i]));
    }

    return sch;
}

CalcModeSchedule schedule_to_cmschedule(Schedule sch) {
    CalcModeSchedule cmsch;

    for (int i = 0; i < sch.length(); i++) {
        if ((i+1) < sch.length())
            cmsch.append(se_to_cmse(sch[i], sch[i+1]));
        else
            cmsch.append(se_to_cmse(sch[i]));
    }

    return cmsch;
}

QString se_to_csv(ScheduleEntry se)
{
    QString result;
    result.sprintf("%02hhu:%02hhu,%02hhu:%02hhu,%d",
                   se.ls_hour, se.ls_minute,
                   se.le_hour, se.le_minute, se.rings);

    return result;
}

QString cmse_to_csv(CalcModeScheduleEntry cmse)
{
    return se_to_csv(cmse_to_se(cmse));
}

QString schedule_to_csv(Schedule sch)
{
    QString result;

    for (int i = 0; i < sch.length(); i++) {
        result += se_to_csv(sch[i]);
        result += "\n";
    }

    return result;
}

QString cmschedule_to_csv(CalcModeSchedule cmsch)
{
    QString result;

    for (int i = 0; i < cmsch.length(); i++) {
        result += cmse_to_csv(cmsch[i]);
        result += "\n";
    }

    return result;
}

Schedule csv_to_schedule(QString csv)
{
    QStringList rows = csv.split("\n");
    Schedule sch;

    for (QString row : rows) {
        if (row == QString())
            break;

        ScheduleEntry se;
        sscanf(row.toUtf8().data(), "%02hhu:%02hhu,%02hhu:%02hhu,%hhu",
               &se.ls_hour, &se.ls_minute,
               &se.le_hour, &se.le_minute, &se.rings);

        sch.append(se);
    }

    return sch;
}

CalcModeSchedule csv_to_cmschedule(QString csv)
{
    return schedule_to_cmschedule(csv_to_schedule(csv));
}

Schedule schedule_from_qstdim(QStandardItemModel* model)
{
    Schedule sch;

    int rc = model->rowCount();

    QStringList lesson_start, lesson_end;
    ScheduleEntry se;
    for (int row = 0; row < rc; row++) {
        lesson_start = model->index(row, 0).data().toString().split(":");
        lesson_end = model->index(row, 1).data().toString().split(":");

        se.ls_hour = static_cast<unsigned char>(lesson_start[0].toInt());
        se.ls_minute = static_cast<unsigned char>(lesson_start[1].toInt());

        se.le_hour = static_cast<unsigned char>(lesson_end[0].toInt());
        se.le_minute =  static_cast<unsigned char>(lesson_end[1].toInt());

        se.rings = static_cast<unsigned char>(model->index(row, 2).data().toInt());

        sch.append(se);
    }

    return sch;
}

CalcModeSchedule cmschedule_from_qstdim(QStandardItemModel* model)
{
    CalcModeSchedule cmsch;

    int rc = model->rowCount();

    QStringList lesson_start;
    CalcModeScheduleEntry cmse;
    for (int row = 0; row < rc; row++) {
        lesson_start = model->index(row, 0).data().toString().split(":");

        cmse.ls_hour = static_cast<unsigned char>(lesson_start[0].toInt());
        cmse.ls_minute = static_cast<unsigned char>(lesson_start[1].toInt());

        cmse.lesson_delay = static_cast<signed char>(model->index(row, 1).data().toInt());
        cmse.break_delay = static_cast<signed char>(model->index(row, 2).data().toInt());
        cmse.rings = static_cast<unsigned char>(model->index(row, 3).data().toInt());

        cmsch.append(cmse);
    }

    return cmsch;
}

void schedule_to_qstdim(QStandardItemModel** model, Schedule sch)
{
    (*model)->clear();
    (*model)->setColumnCount(3);
    (*model)->setHorizontalHeaderLabels(QStringList() << "Начало" << "Конец" << "Особые звонки");

    QList<QStandardItem*> row;
    for (int i = 0; i < sch.length(); i++) {
        row.append(new QStandardItem(QString().sprintf("%02hhu:%02hhu", sch[i].ls_hour, sch[i].ls_minute)));
        row.append(new QStandardItem(QString().sprintf("%02hhu:%02hhu", sch[i].le_hour, sch[i].le_minute)));
        row.append(new QStandardItem(QString::number(sch[i].rings)));

        (*model)->appendRow(row);
        row.clear();
    }
}

void cmschedule_to_qstdim(QStandardItemModel** model, CalcModeSchedule cmsch)
{
    (*model)->clear();
    (*model)->setColumnCount(4);
    (*model)->setHorizontalHeaderLabels(QStringList() << "Начало" << "Длина урока (мин)" << "Длина перемены (мин)" << "Особые звонки");

    QList<QStandardItem*> row;
    for (int i = 0; i < cmsch.length(); i++) {
        row.append(new QStandardItem(QString().sprintf("%02hhu:%02hhu", cmsch[i].ls_hour, cmsch[i].ls_minute)));
        row.append(new QStandardItem(QString::number(cmsch[i].lesson_delay)));
        row.append(new QStandardItem(QString::number(cmsch[i].break_delay)));
        row.append(new QStandardItem(QString::number(cmsch[i].rings)));

        (*model)->appendRow(row);
        row.clear();
    }
}
