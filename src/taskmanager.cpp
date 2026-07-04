#include "taskmanager.h"

TaskManager::TaskManager(QObject* parent)
    : QObject(parent)
{
}

void TaskManager::addTask(const Task& task)
{
    m_tasks.append(task);
    emit tasksChanged();
}

QVector<Task> TaskManager::tasks() const
{
    return m_tasks;
}

QVector<Task> TaskManager::filterByTag(const QString& tag) const
{
    QVector<Task> result;

    for (const Task& task : m_tasks)
    {
        if (task.tags().contains(tag, Qt::CaseInsensitive))
        {
            result.append(task);
        }
    }

    return result;
}