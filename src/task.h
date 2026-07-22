#ifndef TASK_H
#define TASK_H

#include <QString>
#include <QStringList>
#include <QDate>
#include <QDateTime>
#include <QJsonObject>

enum class TaskPriority
{
    Low,
    Medium,
    High
};

enum class TaskStatus
{
    Planned,
    InProgress,
    Completed
};

class Task
{
public:
    Task();
    Task(const QString& title,
         const QString& description,
         const QStringList& tags,
         const QString& assignee,
         TaskPriority priority,
         TaskStatus status = TaskStatus::Planned);

    int id() const;
    QString title() const;
    QString description() const;
    QStringList tags() const;
    QString assignee() const;
    TaskPriority priority() const;
    TaskStatus status() const;
    QDate dueDate() const;
    QDateTime createdAt() const;
    bool isCompleted() const;
    bool isOverdue() const;
    bool isDueSoon(int days = 3) const;

    void setId(int id);
    void setTitle(const QString& title);
    void setDescription(const QString& description);
    void setTags(const QStringList& tags);
    void setAssignee(const QString& assignee);
    void setPriority(TaskPriority priority);
    void setStatus(TaskStatus status);
    void setDueDate(const QDate& date);
    void setCreatedAt(const QDateTime& dateTime);
    void setCompleted(bool completed);

    QJsonObject toJson() const;
    static Task fromJson(const QJsonObject& json);

private:
    int m_id;
    QString m_title;
    QString m_description;
    QStringList m_tags;
    QString m_assignee;
    TaskPriority m_priority;
    TaskStatus m_status;
    QDate m_dueDate;
    QDateTime m_createdAt;
};

#endif
