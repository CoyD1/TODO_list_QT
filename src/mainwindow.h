#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QCheckBox>
#include <QDateEdit>
#include <QPair>
#include <memory>
#include "taskmanager.h"
#include "serializer.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void addTask();
    void editSelectedTask();
    void removeSelectedTask();
    void duplicateSelectedTask();
    void manageTeamMembers();
    void toggleSelectedTask();
    void applyFilters();
    void resetFilters();
    void clearCompletedTasks();
    void saveTasksToFile();
    void loadTasksFromFile();
    void autoSaveTasks();
    void updateTaskList();

private:
    void applyAppStyle();
    int selectedTaskIndex() const;
    QVector<QPair<int, Task>> visibleTasks() const;
    static QString priorityText(TaskPriority priority);
    static QString taskStatusText(TaskStatus status);
    static int priorityWeight(TaskPriority priority);
    void addTeamMember(const QString& name);
    void refreshTeamMembersFromTasks();
    void loadTeamMembers();
    void saveTeamMembers() const;
    QString defaultTasksFilePath() const;
    bool loadTasksFromPath(const QString& filePath, bool showMessage);

    std::unique_ptr<TaskManager> m_taskManager;

    QTableWidget* m_taskTable;
    QLineEdit* m_titleInput;
    QLineEdit* m_descriptionInput;
    QComboBox* m_assigneeInput;
    QPushButton* m_manageMembersButton;
    QLineEdit* m_tagsInput;
    QLineEdit* m_filterInput;
    QLineEdit* m_assigneeFilterInput;
    QLineEdit* m_searchInput;
    QComboBox* m_priorityBox;
    QComboBox* m_statusBox;
    QComboBox* m_statusFilterBox;
    QDateEdit* m_dueDateInput;
    QCheckBox* m_noDueDateBox;
    QComboBox* m_sortBox;
    QPushButton* m_addButton;
    QPushButton* m_editButton;
    QPushButton* m_duplicateButton;
    QPushButton* m_removeButton;
    QPushButton* m_toggleButton;
    QPushButton* m_filterButton;
    QPushButton* m_resetFilterButton;
    QPushButton* m_clearCompletedButton;
    QPushButton* m_saveButton;
    QPushButton* m_loadButton;
    QLabel* m_statusLabel;
    QCheckBox* m_hideCompletedBox;
    QCheckBox* m_showOverdueOnlyBox;

    QStringList m_teamMembers;
    QString m_activeFilter;
    QString m_activeAssigneeFilter;
    int m_activeStatusFilter;
    QString m_tasksFilePath;
};

#endif
