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

void TaskManager::removeTask(int index)
{
    if (index < 0 || index >= m_tasks.size())
    {
        return;
    }

    m_tasks.removeAt(index);
    emit tasksChanged();
}

void TaskManager::toggleCompleted(int index)
{
    if (index < 0 || index >= m_tasks.size())
    {
        return;
    }

    m_tasks[index].setCompleted(!m_tasks[index].isCompleted());
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

int TaskManager::completedCount() const
{
    int count = 0;

    for (const Task& task : m_tasks)
    {
        if (task.isCompleted())
        {
            ++count;
        }
    }

    return count;
}