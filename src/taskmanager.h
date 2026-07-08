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
    void removeTask(int index);
    void toggleCompleted(int index);
    void clearCompleted();
    void updateTask(int index, const Task& task);
    void setTasks(const QVector<Task>& tasks);
    QVector<Task> tasks() const;
    QVector<Task> filterByTag(const QString& tag) const;
    int completedCount() const;
    int taskIndexById(int id) const;

signals:
    void tasksChanged();

private:
    int assignId();

    QVector<Task> m_tasks;
    int m_nextId;
};

#endif