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
#include <QUdpSocket>
#include "taskmanager.h"
#include "serializer.h"
#include "taskserver.h"
#include "networkclient.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

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

    void startAutomaticSync();
    void finishAutomaticSyncSearch();
    void onDiscoveryResponse();
    void onDiscoveryRequest();
    void onConnected();
    void onDisconnected();
    void onConnectionError(const QString& error);
    void onTasksReceived(const QVector<Task>& tasks);
    void onServerClientConnected();
    void onServerClientDisconnected();

private:
    int selectedTaskIndex() const;
    QVector<QPair<int, Task>> visibleTasks() const;
    static QString priorityText(TaskPriority priority);
    static QString taskStatusText(TaskStatus status);
    static int priorityWeight(TaskPriority priority);
    bool startLocalServer(quint16 port);
    void stopLocalServer();
    void startDiscoveryResponder();
    void stopDiscoverySearch();
    void updateSyncStatus();
    void addTeamMember(const QString& name);
    void refreshTeamMembersFromTasks();
    void loadTeamMembers();
    void saveTeamMembers() const;
    void setClientModeEnabled(bool enabled);
    QString defaultTasksFilePath() const;
    bool loadTasksFromPath(const QString& filePath, bool showMessage);

    TaskManager* m_taskManager;
    TaskServer* m_server;
    NetworkClient* m_client;

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

    QLabel* m_syncStatusLabel;
    QUdpSocket* m_discoveryClient;
    QUdpSocket* m_discoveryServer;

    QString m_syncHost;
    quint16 m_syncPort;
    bool m_autoSyncInProgress;
    QStringList m_teamMembers;
    QString m_activeFilter;
    QString m_activeAssigneeFilter;
    int m_activeStatusFilter;
    QString m_tasksFilePath;
    bool m_clientMode;
    int m_connectedClients;
};

#endif
