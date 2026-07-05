#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QCheckBox>
#include <QPair>
#include "taskmanager.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void addTask();
    void removeSelectedTask();
    void toggleSelectedTask();
    void applyTagFilter();
    void resetTagFilter();
    void clearCompletedTasks();
    void updateTaskList();

private:
    int selectedTaskIndex() const;
    QVector<QPair<int, Task>> visibleTasks() const;
    static QString priorityText(TaskPriority priority);
    static int priorityWeight(TaskPriority priority);

    TaskManager* m_taskManager;

    QListWidget* m_taskList;
    QLineEdit* m_titleInput;
    QLineEdit* m_descriptionInput;
    QLineEdit* m_tagsInput;
    QLineEdit* m_filterInput;
    QLineEdit* m_searchInput;
    QComboBox* m_priorityBox;
    QComboBox* m_sortBox;
    QPushButton* m_addButton;
    QPushButton* m_removeButton;
    QPushButton* m_toggleButton;
    QPushButton* m_filterButton;
    QPushButton* m_resetFilterButton;
    QPushButton* m_clearCompletedButton;
    QLabel* m_statusLabel;
    QCheckBox* m_hideCompletedBox;

    QString m_activeFilter;
};

#endif