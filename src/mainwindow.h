#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
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
    void updateTaskList();

private:
    int selectedTaskIndex() const;

    TaskManager* m_taskManager;

    QListWidget* m_taskList;
    QLineEdit* m_titleInput;
    QLineEdit* m_descriptionInput;
    QLineEdit* m_tagsInput;
    QLineEdit* m_filterInput;
    QComboBox* m_priorityBox;
    QPushButton* m_addButton;
    QPushButton* m_removeButton;
    QPushButton* m_toggleButton;
    QPushButton* m_filterButton;
    QPushButton* m_resetFilterButton;
    QLabel* m_statusLabel;

    QString m_activeFilter;
};

#endif