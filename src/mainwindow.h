#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include "taskmanager.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void addTask();
    void updateTaskList();

private:
    TaskManager* m_taskManager;

    QListWidget* m_taskList;
    QLineEdit* m_titleInput;
    QLineEdit* m_descriptionInput;
    QLineEdit* m_tagsInput;
    QComboBox* m_priorityBox;
    QPushButton* m_addButton;
};

#endif