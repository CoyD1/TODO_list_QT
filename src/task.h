#ifndef TASK_H
#define TASK_H

#include <QString>
#include <QStringList>
#include <QDate>
#include <QJsonObject>

enum class TaskPriority
{
    Low,
    Medium,
    High
};

class Task
{
public:
    Task();
    Task(const QString& title,
         const QString& description,
         const QStringList& tags,
         TaskPriority priority);

    int id() const;
    QString title() const;
    QString description() const;
    QStringList tags() const;
    TaskPriority priority() const;
    QDate dueDate() const;
    bool isCompleted() const;

    void setId(int id);
    void setTitle(const QString& title);
    void setDescription(const QString& description);
    void setTags(const QStringList& tags);
    void setPriority(TaskPriority priority);
    void setDueDate(const QDate& date);
    void setCompleted(bool completed);

    QJsonObject toJson() const;
    static Task fromJson(const QJsonObject& json);

private:
    int m_id;
    QString m_title;
    QString m_description;
    QStringList m_tags;
    TaskPriority m_priority;
    QDate m_dueDate;
    bool m_completed;
};

#endif