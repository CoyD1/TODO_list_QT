#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <QObject>
#include <QVector>
#include "task.h"

class TaskManager : public QObject
{
    Q_OBJECT

public:
    explicit TaskManager(QObject* parent = nullptr);

    void addTask(const Task& task);
    QVector<Task> tasks() const;
    QVector<Task> filterByTag(const QString& tag) const;

signals:
    void tasksChanged();

private:
    QVector<Task> m_tasks;
};

#endif