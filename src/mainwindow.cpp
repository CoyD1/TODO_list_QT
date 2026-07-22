#include "mainwindow.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QListWidgetItem>
#include <QFont>
#include <QGroupBox>
#include <QDialog>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QDateEdit>
#include <QDateTime>
#include <QSettings>
#include <QHeaderView>
#include <QListWidget>
#include <QMenu>
#include <QPalette>
#include <QColor>
#include <QSizePolicy>
#include <memory>
#include <algorithm>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
    m_taskManager(std::make_unique<TaskManager>()),
    m_activeStatusFilter(-1)
{
    setWindowTitle("Командный TODO-лист");
    resize(1060, 760);
    setMinimumSize(760, 600);
    applyAppStyle();

    QWidget* centralWidget = new QWidget(this);
    centralWidget->setObjectName("centralRoot");
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(16, 12, 16, 12);
    mainLayout->setSpacing(10);

    QLabel* appTitle = new QLabel("Командный TODO-лист", this);
    appTitle->setObjectName("appTitle");
    QLabel* appSubtitle = new QLabel(
        "Задачи команды · фильтры · сохранение в файл", this);
    appSubtitle->setObjectName("appSubtitle");
    QHBoxLayout* headerLayout = new QHBoxLayout();
    headerLayout->addWidget(appTitle);
    headerLayout->addSpacing(14);
    headerLayout->addWidget(appSubtitle);
    headerLayout->addStretch();
    mainLayout->addLayout(headerLayout);

    // ── Поиск и фильтры ──
    QGroupBox* filterGroup = new QGroupBox("Поиск и управление", this);
    filterGroup->setObjectName("card");
    filterGroup->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    QGridLayout* filterGroupLayout = new QGridLayout(filterGroup);
    filterGroupLayout->setContentsMargins(12, 22, 12, 12);
    filterGroupLayout->setSpacing(8);
    filterGroupLayout->setColumnStretch(0, 1);
    filterGroupLayout->setColumnStretch(1, 1);

    m_searchInput = new QLineEdit(this);
    m_searchInput->setPlaceholderText("Поиск по названию или исполнителю...");
    m_searchInput->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    m_addButton = new QPushButton("Новая задача", this);
    m_addButton->setObjectName("primaryButton");
    m_manageMembersButton = new QPushButton("Участники", this);
    QPushButton* moreButton = new QPushButton("Ещё", this);
    QMenu* moreMenu = new QMenu(moreButton);
    QAction* saveAction = moreMenu->addAction("Сохранить в файл...");
    QAction* loadAction = moreMenu->addAction("Загрузить из файла...");
    moreMenu->addSeparator();
    QAction* clearCompletedAction = moreMenu->addAction("Очистить выполненные");
    moreMenu->addSeparator();
    m_hideCompletedAction = moreMenu->addAction("Скрыть выполненные");
    m_hideCompletedAction->setCheckable(true);
    m_showOverdueOnlyAction = moreMenu->addAction("Только просроченные");
    m_showOverdueOnlyAction->setCheckable(true);
    QAction* resetFiltersAction = moreMenu->addAction("Сбросить фильтры");
    moreButton->setMenu(moreMenu);
    QHBoxLayout* searchLayout = new QHBoxLayout();
    searchLayout->setContentsMargins(0, 0, 0, 0);
    searchLayout->addWidget(m_searchInput, 1);
    searchLayout->addWidget(m_addButton);
    searchLayout->addWidget(m_manageMembersButton);
    searchLayout->addWidget(moreButton);
    filterGroupLayout->addLayout(searchLayout, 0, 0, 1, 2);

    QHBoxLayout* tagFilterLayout = new QHBoxLayout();
    m_filterInput = new QLineEdit(this);
    m_filterInput->setPlaceholderText("Фильтр по тегу");
    m_filterInput->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    tagFilterLayout->addWidget(m_filterInput, 1);
    m_assigneeFilterInput = new QLineEdit(this);
    m_assigneeFilterInput->setPlaceholderText("Фильтр по исполнителю");
    m_assigneeFilterInput->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    tagFilterLayout->addWidget(m_assigneeFilterInput, 1);
    m_statusFilterBox = new QComboBox(this);
    m_statusFilterBox->addItem("Все статусы");
    m_statusFilterBox->addItem("Запланированные");
    m_statusFilterBox->addItem("В работе");
    m_statusFilterBox->addItem("Выполненные");
    m_statusFilterBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_statusFilterBox->setMinimumContentsLength(8);
    m_statusFilterBox->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    m_sortBox = new QComboBox(this);
    m_sortBox->addItem("Порядок добавления");
    m_sortBox->addItem("Сначала важные");
    m_sortBox->addItem("Сначала простые");
    m_sortBox->addItem("По сроку");
    m_sortBox->addItem("Сначала новые");
    m_sortBox->addItem("Сначала старые");
    m_sortBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_sortBox->setMinimumContentsLength(8);
    m_sortBox->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    QGridLayout* selectorsLayout = new QGridLayout();
    selectorsLayout->setContentsMargins(0, 0, 0, 0);
    selectorsLayout->addWidget(new QLabel("Статус:", this), 0, 0);
    selectorsLayout->addWidget(m_statusFilterBox, 0, 1);
    selectorsLayout->addWidget(new QLabel("Сортировка:", this), 0, 2);
    selectorsLayout->addWidget(m_sortBox, 0, 3);
    selectorsLayout->setColumnStretch(1, 1);
    selectorsLayout->setColumnStretch(3, 1);
    filterGroupLayout->addLayout(selectorsLayout, 2, 0, 1, 2);
    filterGroupLayout->addLayout(tagFilterLayout, 1, 0, 1, 2);

    QGridLayout* actionRow = new QGridLayout();
    actionRow->setContentsMargins(0, 0, 0, 0);
    actionRow->setHorizontalSpacing(8);
    m_toggleButton = new QPushButton("Отметить", this);
    actionRow->addWidget(m_toggleButton, 0, 0);
    m_editButton = new QPushButton("Редактировать", this);
    actionRow->addWidget(m_editButton, 0, 1);
    m_duplicateButton = new QPushButton("Дублировать", this);
    actionRow->addWidget(m_duplicateButton, 0, 2);
    m_removeButton = new QPushButton("Удалить", this);
    m_removeButton->setObjectName("dangerButton");
    actionRow->addWidget(m_removeButton, 0, 3);
    for (int column = 0; column < 4; ++column)
    {
        actionRow->setColumnStretch(column, 1);
    }
    filterGroupLayout->addLayout(actionRow, 3, 0, 1, 2);

    mainLayout->addWidget(filterGroup);

    // ── Список задач ──
    QGroupBox* listGroup = new QGroupBox("Задачи", this);
    listGroup->setObjectName("card");
    listGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QVBoxLayout* listLayout = new QVBoxLayout(listGroup);
    listLayout->setContentsMargins(10, 22, 10, 10);
    m_taskTable = new QTableWidget(this);
    m_taskTable->setObjectName("taskTable");
    m_taskTable->setColumnCount(7);
    m_taskTable->setHorizontalHeaderLabels({
        "Статус", "Задача", "Исполнитель", "Приоритет",
        "Срок", "Теги", "Создана"
    });
    m_taskTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_taskTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_taskTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_taskTable->setAlternatingRowColors(true);
    m_taskTable->setShowGrid(false);
    m_taskTable->setFocusPolicy(Qt::StrongFocus);
    m_taskTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_taskTable->verticalHeader()->setVisible(false);
    m_taskTable->verticalHeader()->setDefaultSectionSize(34);
    const int fiveRowsHeight = m_taskTable->horizontalHeader()->sizeHint().height()
                               + 5 * m_taskTable->verticalHeader()->defaultSectionSize()
                               + 24;
    m_taskTable->setMinimumHeight(fiveRowsHeight);
    m_taskTable->horizontalHeader()->setHighlightSections(false);
    m_taskTable->horizontalHeader()->setStretchLastSection(false);
    m_taskTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_taskTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_taskTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
    m_taskTable->setColumnWidth(0, 100);
    m_taskTable->setColumnWidth(2, 105);
    m_taskTable->setColumnWidth(3, 85);
    m_taskTable->setColumnWidth(4, 135);
    m_taskTable->setColumnWidth(6, 125);
    listLayout->addWidget(m_taskTable);
    mainLayout->addWidget(listGroup, 1);

    // ── Статус строка ──
    m_statusLabel = new QLabel(this);
    m_statusLabel->setObjectName("footerStatus");
    mainLayout->addWidget(m_statusLabel);

    setCentralWidget(centralWidget);

    connect(m_addButton, &QPushButton::clicked,
            this, &MainWindow::addTask);
    connect(m_editButton, &QPushButton::clicked,
            this, &MainWindow::editSelectedTask);
    connect(m_duplicateButton, &QPushButton::clicked,
            this, &MainWindow::duplicateSelectedTask);
    connect(m_manageMembersButton, &QPushButton::clicked,
            this, &MainWindow::manageTeamMembers);
    connect(m_removeButton, &QPushButton::clicked,
            this, &MainWindow::removeSelectedTask);
    connect(m_toggleButton, &QPushButton::clicked,
            this, &MainWindow::toggleSelectedTask);
    connect(resetFiltersAction, &QAction::triggered,
            this, &MainWindow::resetFilters);
    connect(saveAction, &QAction::triggered,
            this, &MainWindow::saveTasksToFile);
    connect(loadAction, &QAction::triggered,
            this, &MainWindow::loadTasksFromFile);
    connect(clearCompletedAction, &QAction::triggered,
            this, &MainWindow::clearCompletedTasks);
    connect(m_taskTable, &QTableWidget::cellDoubleClicked,
            this, &MainWindow::editSelectedTask);
    connect(m_searchInput, &QLineEdit::textChanged,
            this, &MainWindow::updateTaskList);
    connect(m_filterInput, &QLineEdit::textChanged,
            this, &MainWindow::applyFilters);
    connect(m_assigneeFilterInput, &QLineEdit::textChanged,
            this, &MainWindow::applyFilters);
    connect(m_statusFilterBox, &QComboBox::currentIndexChanged,
            this, &MainWindow::applyFilters);
    connect(m_sortBox, &QComboBox::currentIndexChanged,
            this, &MainWindow::updateTaskList);
    connect(m_hideCompletedAction, &QAction::toggled,
            this, &MainWindow::updateTaskList);
    connect(m_showOverdueOnlyAction, &QAction::toggled,
            this, &MainWindow::updateTaskList);
    connect(m_taskManager.get(), &TaskManager::tasksChanged,
            this, &MainWindow::updateTaskList);
    connect(m_taskManager.get(), &TaskManager::tasksChanged,
            this, &MainWindow::autoSaveTasks);

    loadTeamMembers();
    m_tasksFilePath = defaultTasksFilePath();
    loadTasksFromPath(m_tasksFilePath, false);
    refreshTeamMembersFromTasks();

    updateTaskList();
}

MainWindow::~MainWindow() = default;

void MainWindow::applyAppStyle()
{
    // Принудительно светлая палитра: на macOS Dark Mode иначе текст
    // остаётся светлым на белом фоне из stylesheet.
    QPalette lightPalette = palette();
    lightPalette.setColor(QPalette::Window, QColor("#eef2f6"));
    lightPalette.setColor(QPalette::WindowText, QColor("#1f2933"));
    lightPalette.setColor(QPalette::Base, QColor("#ffffff"));
    lightPalette.setColor(QPalette::AlternateBase, QColor("#f7f9fc"));
    lightPalette.setColor(QPalette::Text, QColor("#1f2933"));
    lightPalette.setColor(QPalette::Button, QColor("#f4f7fb"));
    lightPalette.setColor(QPalette::ButtonText, QColor("#1f2933"));
    lightPalette.setColor(QPalette::PlaceholderText, QColor("#7a8794"));
    lightPalette.setColor(QPalette::Highlight, QColor("#2f6fed"));
    lightPalette.setColor(QPalette::HighlightedText, QColor("#ffffff"));
    lightPalette.setColor(QPalette::ToolTipBase, QColor("#ffffff"));
    lightPalette.setColor(QPalette::ToolTipText, QColor("#1f2933"));
    setPalette(lightPalette);

    setStyleSheet(R"(
        QMainWindow, QWidget#centralRoot {
            background: #eef2f6;
            color: #1f2933;
            font-size: 13px;
        }
        QLabel {
            color: #1f2933;
            background: transparent;
        }
        QLabel#appTitle {
            font-size: 22px;
            font-weight: 700;
            color: #16324f;
        }
        QLabel#appSubtitle {
            color: #526273;
            margin-bottom: 4px;
        }
        QGroupBox#card {
            background: #ffffff;
            border: 1px solid #b8c4d2;
            border-radius: 10px;
            margin-top: 14px;
            padding-top: 8px;
            font-weight: 600;
            color: #1f2933;
        }
        QGroupBox#card::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            left: 12px;
            padding: 0 6px;
            color: #1f2933;
            background: #ffffff;
        }
        QLineEdit, QComboBox, QDateEdit, QSpinBox {
            color: #1f2933;
            background: #ffffff;
            border: 1px solid #9aabbc;
            border-radius: 6px;
            padding: 6px 8px;
            min-height: 18px;
            selection-background-color: #2f6fed;
            selection-color: #ffffff;
        }
        QLineEdit:focus, QComboBox:focus, QDateEdit:focus {
            border: 1px solid #2f6fed;
            background: #ffffff;
            color: #1f2933;
        }
        QComboBox QAbstractItemView {
            color: #1f2933;
            background: #ffffff;
            selection-background-color: #dce9ff;
            selection-color: #1f2933;
        }
        QPushButton {
            color: #1f2933;
            background: #e8eef6;
            border: 1px solid #8fa0b3;
            border-radius: 7px;
            padding: 6px 12px;
            min-height: 24px;
        }
        QPushButton:hover {
            background: #d9e4f2;
            border-color: #6f8399;
            color: #1f2933;
        }
        QPushButton:pressed {
            background: #c9d7e8;
            color: #1f2933;
        }
        QPushButton#primaryButton {
            background: #2f6fed;
            color: #ffffff;
            border: 1px solid #255fd0;
            font-weight: 600;
        }
        QPushButton#primaryButton:hover {
            background: #2762d8;
            color: #ffffff;
        }
        QPushButton#dangerButton {
            background: #fff1ef;
            color: #a83228;
            border: 1px solid #d98c84;
            font-weight: 600;
        }
        QPushButton#dangerButton:hover {
            background: #ffe3df;
            color: #8f261e;
        }
        QCheckBox {
            color: #1f2933;
            spacing: 6px;
        }
        QTableWidget#taskTable {
            color: #1f2933;
            background: #ffffff;
            border: 1px solid #b8c4d2;
            border-radius: 8px;
            gridline-color: #e3e9f0;
            selection-background-color: #dce9ff;
            selection-color: #1f2933;
            alternate-background-color: #f4f7fb;
        }
        QTableWidget#taskTable::item {
            color: #1f2933;
            padding: 4px;
        }
        QTableWidget#taskTable::item:selected {
            color: #1f2933;
            background: #dce9ff;
        }
        QHeaderView::section {
            background: #e8eef6;
            color: #1f2933;
            border: none;
            border-bottom: 1px solid #b8c4d2;
            padding: 8px 6px;
            font-weight: 600;
        }
        QLabel#footerStatus {
            color: #334155;
            padding: 2px 2px;
        }
        QScrollBar:vertical {
            background: transparent;
            width: 10px;
            margin: 2px;
        }
        QScrollBar::handle:vertical {
            background: #9aabbc;
            border-radius: 5px;
            min-height: 24px;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0;
        }
    )");
}

void MainWindow::addTask()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Новая задача");
    dialog.resize(500, 380);

    QFormLayout* form = new QFormLayout(&dialog);
    form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    QLineEdit* titleEdit = new QLineEdit(&dialog);
    titleEdit->setPlaceholderText("Название задачи");
    QLineEdit* descriptionEdit = new QLineEdit(&dialog);
    descriptionEdit->setPlaceholderText("Описание задачи");
    QComboBox* assigneeEdit = new QComboBox(&dialog);
    assigneeEdit->setEditable(true);
    assigneeEdit->setInsertPolicy(QComboBox::NoInsert);
    assigneeEdit->addItems(m_teamMembers);
    assigneeEdit->lineEdit()->setPlaceholderText("Выберите или введите имя");
    QLineEdit* tagsEdit = new QLineEdit(&dialog);
    tagsEdit->setPlaceholderText("учеба, срочно, работа");

    QComboBox* priorityEdit = new QComboBox(&dialog);
    priorityEdit->addItems({"Низкий", "Средний", "Высокий"});
    priorityEdit->setCurrentIndex(1);
    QComboBox* statusEdit = new QComboBox(&dialog);
    statusEdit->addItems({"Запланирована", "В работе", "Выполнена"});

    QDateEdit* dueDateEdit = new QDateEdit(QDate::currentDate(), &dialog);
    dueDateEdit->setCalendarPopup(true);
    dueDateEdit->setEnabled(false);
    QCheckBox* noDueDateBox = new QCheckBox("Без срока", &dialog);
    noDueDateBox->setChecked(true);
    connect(noDueDateBox, &QCheckBox::toggled,
            dueDateEdit, &QWidget::setDisabled);

    QHBoxLayout* dueDateLayout = new QHBoxLayout();
    dueDateLayout->addWidget(dueDateEdit);
    dueDateLayout->addWidget(noDueDateBox);
    dueDateLayout->addStretch();

    form->addRow("Название:", titleEdit);
    form->addRow("Описание:", descriptionEdit);
    form->addRow("Исполнитель:", assigneeEdit);
    form->addRow("Теги:", tagsEdit);
    form->addRow("Приоритет:", priorityEdit);
    form->addRow("Статус:", statusEdit);
    form->addRow("Срок:", dueDateLayout);

    QDialogButtonBox* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, [&]()
    {
        if (titleEdit->text().trimmed().isEmpty())
        {
            QMessageBox::warning(&dialog, "Новая задача", "Введите название задачи");
            return;
        }
        dialog.accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted)
    {
        return;
    }

    const QString title = titleEdit->text().trimmed();
    const QString description = descriptionEdit->text().trimmed();
    const QString assignee = assigneeEdit->currentText().trimmed();
    QStringList tags = tagsEdit->text().split(",", Qt::SkipEmptyParts);

    for (QString& tag : tags)
    {
        tag = tag.trimmed();
    }

    const TaskPriority priority = static_cast<TaskPriority>(priorityEdit->currentIndex());
    const TaskStatus status = static_cast<TaskStatus>(statusEdit->currentIndex());
    Task task(title, description, tags, assignee, priority, status);
    addTeamMember(assignee);

    if (!noDueDateBox->isChecked() && dueDateEdit->date().isValid())
    {
        task.setDueDate(dueDateEdit->date());
    }

    m_taskManager->addTask(task);
}

void MainWindow::editSelectedTask()
{
    int index = selectedTaskIndex();

    if (index < 0)
    {
        return;
    }

    const QVector<Task> allTasks = m_taskManager->tasks();
    if (index >= allTasks.size())
    {
        return;
    }
    const Task& task = allTasks[index];

    QDialog dialog(this);
    dialog.setWindowTitle("Редактирование задачи");

    QFormLayout* form = new QFormLayout(&dialog);
    form->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
    form->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    QLineEdit* titleEdit = new QLineEdit(task.title(), &dialog);
    QLineEdit* descEdit = new QLineEdit(task.description(), &dialog);
    QComboBox* assigneeEdit = new QComboBox(&dialog);
    assigneeEdit->setEditable(true);
    assigneeEdit->addItems(m_teamMembers);
    assigneeEdit->setCurrentText(task.assignee());
    QLineEdit* tagsEdit = new QLineEdit(task.tags().join(", "), &dialog);
    QComboBox* priorityEdit = new QComboBox(&dialog);
    priorityEdit->addItem("Низкий");
    priorityEdit->addItem("Средний");
    priorityEdit->addItem("Высокий");
    priorityEdit->setCurrentIndex(static_cast<int>(task.priority()));

    QComboBox* statusEdit = new QComboBox(&dialog);
    statusEdit->addItem("Запланирована");
    statusEdit->addItem("В работе");
    statusEdit->addItem("Выполнена");
    statusEdit->setCurrentIndex(static_cast<int>(task.status()));

    QDateEdit* dueDateEdit = new QDateEdit(&dialog);
    dueDateEdit->setCalendarPopup(true);
    QCheckBox* noDueDateBox = new QCheckBox("Без срока", &dialog);

    if (task.dueDate().isValid())
    {
        dueDateEdit->setDate(task.dueDate());
    }
    else
    {
        dueDateEdit->setDate(QDate::currentDate());
    }
    noDueDateBox->setChecked(!task.dueDate().isValid());
    dueDateEdit->setEnabled(task.dueDate().isValid());
    connect(noDueDateBox, &QCheckBox::toggled,
            dueDateEdit, &QWidget::setDisabled);

    QHBoxLayout* dueDateLayout = new QHBoxLayout();
    dueDateLayout->addWidget(dueDateEdit);
    dueDateLayout->addWidget(noDueDateBox);
    dueDateLayout->addStretch();

    form->addRow("Название:", titleEdit);
    form->addRow("Описание:", descEdit);
    form->addRow("Исполнитель:", assigneeEdit);
    form->addRow("Теги:", tagsEdit);
    form->addRow("Приоритет:", priorityEdit);
    form->addRow("Статус:", statusEdit);
    form->addRow("Срок:", dueDateLayout);

    QDialogButtonBox* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form->addRow(buttons);

    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted)
    {
        return;
    }

    QString title = titleEdit->text().trimmed();
    if (title.isEmpty())
    {
        return;
    }

    QStringList tags;
    for (const QString& tag : tagsEdit->text().split(",", Qt::SkipEmptyParts))
    {
        tags.append(tag.trimmed());
    }

    TaskPriority priority = static_cast<TaskPriority>(priorityEdit->currentIndex());
    const QString assignee = assigneeEdit->currentText().trimmed();
    addTeamMember(assignee);
    Task updatedTask(title,
                     descEdit->text().trimmed(),
                     tags,
                     assignee,
                     priority,
                     static_cast<TaskStatus>(statusEdit->currentIndex()));
    updatedTask.setCreatedAt(task.createdAt());

    if (!noDueDateBox->isChecked() && dueDateEdit->date().isValid())
    {
        updatedTask.setDueDate(dueDateEdit->date());
    }

    m_taskManager->updateTask(index, updatedTask);
}

void MainWindow::duplicateSelectedTask()
{
    int index = selectedTaskIndex();

    if (index < 0)
    {
        QMessageBox::information(this, "Дублирование", "Выберите задачу для копирования");
        return;
    }

    const QVector<Task> allTasks = m_taskManager->tasks();
    if (index >= allTasks.size())
    {
        QMessageBox::warning(this, "Дублирование", "Не удалось получить выбранную задачу");
        return;
    }

    m_taskManager->duplicateTask(index);
}

void MainWindow::manageTeamMembers()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Участники команды");
    dialog.resize(380, 360);

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    QListWidget* memberList = new QListWidget(&dialog);
    memberList->addItems(m_teamMembers);
    layout->addWidget(memberList, 1);

    QHBoxLayout* addLayout = new QHBoxLayout();
    QLineEdit* nameInput = new QLineEdit(&dialog);
    nameInput->setPlaceholderText("Имя участника");
    QPushButton* addButton = new QPushButton("Добавить", &dialog);
    addLayout->addWidget(nameInput, 1);
    addLayout->addWidget(addButton);
    layout->addLayout(addLayout);

    QPushButton* removeButton = new QPushButton("Удалить выбранного", &dialog);
    layout->addWidget(removeButton);

    QDialogButtonBox* buttons = new QDialogButtonBox(
        QDialogButtonBox::Close, &dialog);
    layout->addWidget(buttons);

    auto addMemberToDialog = [memberList, nameInput]()
    {
        const QString name = nameInput->text().trimmed();
        if (name.isEmpty())
        {
            return;
        }

        for (int i = 0; i < memberList->count(); ++i)
        {
            if (memberList->item(i)->text().compare(name, Qt::CaseInsensitive) == 0)
            {
                memberList->setCurrentRow(i);
                return;
            }
        }

        memberList->addItem(name);
        nameInput->clear();
    };

    connect(addButton, &QPushButton::clicked, &dialog, addMemberToDialog);
    connect(nameInput, &QLineEdit::returnPressed, &dialog, addMemberToDialog);
    connect(removeButton, &QPushButton::clicked, &dialog,
            [this, memberList, &dialog]()
            {
                QListWidgetItem* item = memberList->currentItem();
                if (!item)
                {
                    return;
                }

                const QString name = item->text();
                for (const Task& task : m_taskManager->tasks())
                {
                    if (task.assignee().compare(name, Qt::CaseInsensitive) == 0)
                    {
                        QMessageBox::information(
                            &dialog,
                            "Участники команды",
                            "Нельзя удалить участника, пока ему назначены задачи");
                        return;
                    }
                }

                delete memberList->takeItem(memberList->currentRow());
            });
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    dialog.exec();

    m_teamMembers.clear();
    for (int i = 0; i < memberList->count(); ++i)
    {
        m_teamMembers.append(memberList->item(i)->text());
    }

    m_teamMembers.sort(Qt::CaseInsensitive);
    saveTeamMembers();
}

void MainWindow::addTeamMember(const QString& name)
{
    const QString trimmedName = name.trimmed();
    if (trimmedName.isEmpty())
    {
        return;
    }

    for (const QString& member : m_teamMembers)
    {
        if (member.compare(trimmedName, Qt::CaseInsensitive) == 0)
        {
            return;
        }
    }

    m_teamMembers.append(trimmedName);
    m_teamMembers.sort(Qt::CaseInsensitive);
    saveTeamMembers();
}

void MainWindow::refreshTeamMembersFromTasks()
{
    for (const Task& task : m_taskManager->tasks())
    {
        addTeamMember(task.assignee());
    }
}

void MainWindow::loadTeamMembers()
{
    QSettings settings("TODO_lists_QT", "TODO_lists_QT");
    m_teamMembers = settings.value("team/members").toStringList();
    m_teamMembers.removeDuplicates();
    m_teamMembers.sort(Qt::CaseInsensitive);
}

void MainWindow::saveTeamMembers() const
{
    QSettings settings("TODO_lists_QT", "TODO_lists_QT");
    settings.setValue("team/members", m_teamMembers);
}

void MainWindow::removeSelectedTask()
{
    int index = selectedTaskIndex();

    if (index < 0)
    {
        return;
    }

    QString taskTitle = m_taskManager->tasks().value(index).title();
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Подтверждение",
        QString("Удалить задачу «%1»?").arg(taskTitle),
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes)
    {
        return;
    }

    m_taskManager->removeTask(index);
}

void MainWindow::toggleSelectedTask()
{
    int index = selectedTaskIndex();

    if (index < 0)
    {
        return;
    }

    m_taskManager->toggleCompleted(index);
}

void MainWindow::applyFilters()
{
    m_activeFilter = m_filterInput->text().trimmed();
    m_activeAssigneeFilter = m_assigneeFilterInput->text().trimmed();
    m_activeStatusFilter = m_statusFilterBox->currentIndex() - 1;
    updateTaskList();
}

void MainWindow::resetFilters()
{
    m_activeFilter.clear();
    m_activeAssigneeFilter.clear();
    m_activeStatusFilter = -1;
    m_filterInput->clear();
    m_assigneeFilterInput->clear();
    m_statusFilterBox->setCurrentIndex(0);
    m_hideCompletedAction->setChecked(false);
    m_showOverdueOnlyAction->setChecked(false);
    updateTaskList();
}

void MainWindow::clearCompletedTasks()
{
    m_taskManager->clearCompleted();
}

QString MainWindow::defaultTasksFilePath() const
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir + "/tasks.json";
}

bool MainWindow::loadTasksFromPath(const QString& filePath, bool showMessage)
{
    if (!QFile::exists(filePath))
    {
        return false;
    }

    JsonSerializer serializer;
    QVector<Task> tasks = serializer.load(filePath);

    if (tasks.isEmpty() && showMessage)
    {
        QMessageBox::warning(this, "Загрузка", "Файл пуст или имеет неверный формат");
        return false;
    }

    m_taskManager->setTasks(tasks);
    refreshTeamMembersFromTasks();
    m_tasksFilePath = filePath;

    if (showMessage)
    {
        QMessageBox::information(this, "Загрузка",
                                 QString("Загружено задач: %1").arg(tasks.size()));
    }

    return true;
}

void MainWindow::saveTasksToFile()
{
    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Сохранить задачи",
        m_tasksFilePath,
        "JSON (*.json)");

    if (filePath.isEmpty())
    {
        return;
    }

    JsonSerializer serializer;
    serializer.save(m_taskManager->tasks(), filePath);
    m_tasksFilePath = filePath;

    QMessageBox::information(this, "Сохранение", "Задачи сохранены");
}

void MainWindow::loadTasksFromFile()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Загрузить задачи",
        m_tasksFilePath,
        "JSON (*.json)");

    if (filePath.isEmpty())
    {
        return;
    }

    loadTasksFromPath(filePath, true);
}

void MainWindow::autoSaveTasks()
{
    JsonSerializer serializer;
    serializer.save(m_taskManager->tasks(), m_tasksFilePath);
}

int MainWindow::selectedTaskIndex() const
{
    const int row = m_taskTable->currentRow();

    if (row < 0)
    {
        return -1;
    }

    QTableWidgetItem* item = m_taskTable->item(row, 0);
    return item ? item->data(Qt::UserRole).toInt() : -1;
}

QString MainWindow::priorityText(TaskPriority priority)
{
    switch (priority)
    {
    case TaskPriority::Low:
        return "Низкий";
    case TaskPriority::Medium:
        return "Средний";
    case TaskPriority::High:
        return "Высокий";
    }

    return "Средний";
}

QString MainWindow::taskStatusText(TaskStatus status)
{
    switch (status)
    {
    case TaskStatus::Planned:
        return "Запланирована";
    case TaskStatus::InProgress:
        return "В работе";
    case TaskStatus::Completed:
        return "Выполнена";
    }

    return "Запланирована";
}

int MainWindow::priorityWeight(TaskPriority priority)
{
    switch (priority)
    {
    case TaskPriority::Low:
        return 0;
    case TaskPriority::Medium:
        return 1;
    case TaskPriority::High:
        return 2;
    }

    return 1;
}

QVector<QPair<int, Task>> MainWindow::visibleTasks() const
{
    QVector<QPair<int, Task>> tasks;
    const QVector<Task> allTasks = m_taskManager->tasks();
    const QString searchText = m_searchInput->text().trimmed();

    for (int i = 0; i < allTasks.size(); ++i)
    {
        const Task& task = allTasks[i];

        if (!searchText.isEmpty()
            && !task.title().contains(searchText, Qt::CaseInsensitive)
            && !task.assignee().contains(searchText, Qt::CaseInsensitive))
        {
            continue;
        }

        if (!m_activeFilter.isEmpty()
            && !task.tags().contains(m_activeFilter, Qt::CaseInsensitive))
        {
            continue;
        }

        if (!m_activeAssigneeFilter.isEmpty()
            && !task.assignee().contains(m_activeAssigneeFilter, Qt::CaseInsensitive))
        {
            continue;
        }

        if (m_activeStatusFilter >= 0
            && static_cast<int>(task.status()) != m_activeStatusFilter)
        {
            continue;
        }

        if (m_hideCompletedAction->isChecked() && task.isCompleted())
        {
            continue;
        }

        if (m_showOverdueOnlyAction->isChecked() && !task.isOverdue())
        {
            continue;
        }

        tasks.append(qMakePair(i, task));
    }

    if (m_sortBox->currentIndex() == 1)
    {
        std::sort(tasks.begin(), tasks.end(),
                  [](const QPair<int, Task>& left, const QPair<int, Task>& right)
                  {
                      return priorityWeight(left.second.priority()) > priorityWeight(right.second.priority());
                  });
    }
    else if (m_sortBox->currentIndex() == 2)
    {
        std::sort(tasks.begin(), tasks.end(),
                  [](const QPair<int, Task>& left, const QPair<int, Task>& right)
                  {
                      return priorityWeight(left.second.priority()) < priorityWeight(right.second.priority());
                  });
    }
    else if (m_sortBox->currentIndex() == 3)
    {
        std::sort(tasks.begin(), tasks.end(),
                  [](const QPair<int, Task>& left, const QPair<int, Task>& right)
                  {
                      bool leftValid = left.second.dueDate().isValid();
                      bool rightValid = right.second.dueDate().isValid();

                      if (!leftValid && !rightValid) return false;
                      if (!leftValid) return false;
                      if (!rightValid) return true;

                      return left.second.dueDate() < right.second.dueDate();
                  });
    }
    else if (m_sortBox->currentIndex() == 4)
    {
        std::sort(tasks.begin(), tasks.end(),
                  [](const QPair<int, Task>& left, const QPair<int, Task>& right)
                  {
                      return left.second.createdAt() > right.second.createdAt();
                  });
    }
    else if (m_sortBox->currentIndex() == 5)
    {
        std::sort(tasks.begin(), tasks.end(),
                  [](const QPair<int, Task>& left, const QPair<int, Task>& right)
                  {
                      return left.second.createdAt() < right.second.createdAt();
                  });
    }

    return tasks;
}

void MainWindow::updateTaskList()
{
    QVector<QPair<int, Task>> tasks = visibleTasks();
    m_taskTable->setUpdatesEnabled(false);
    m_taskTable->clearContents();
    m_taskTable->setRowCount(tasks.size());

    for (int row = 0; row < tasks.size(); ++row)
    {
        const QPair<int, Task>& entry = tasks[row];
        const Task& task = entry.second;
        const int sourceIndex = entry.first;

        QString dueDateText = "—";
        if (task.dueDate().isValid())
        {
            dueDateText = task.dueDate().toString("dd.MM.yyyy");
            if (task.isOverdue())
            {
                dueDateText += " · просрочена";
            }
            else if (task.isDueSoon())
            {
                dueDateText += " · скоро";
            }
        }

        const QString createdText = task.createdAt().isValid()
                                        ? task.createdAt().toString("dd.MM.yyyy HH:mm")
                                        : "—";
        const QString assigneeText = task.assignee().isEmpty() ? "Не назначен" : task.assignee();

        QTableWidgetItem* statusItem = new QTableWidgetItem(taskStatusText(task.status()));
        QTableWidgetItem* titleItem = new QTableWidgetItem(task.title());
        QTableWidgetItem* assigneeItem = new QTableWidgetItem(assigneeText);
        QTableWidgetItem* priorityItem = new QTableWidgetItem(priorityText(task.priority()));
        QTableWidgetItem* dueDateItem = new QTableWidgetItem(dueDateText);
        QTableWidgetItem* tagsItem = new QTableWidgetItem(task.tags().join(", "));
        QTableWidgetItem* createdItem = new QTableWidgetItem(createdText);

        statusItem->setData(Qt::UserRole, sourceIndex);
        titleItem->setToolTip(task.description());

        const QColor defaultTextColor("#1f2933");
        statusItem->setForeground(defaultTextColor);
        titleItem->setForeground(defaultTextColor);
        assigneeItem->setForeground(defaultTextColor);
        priorityItem->setForeground(defaultTextColor);
        dueDateItem->setForeground(defaultTextColor);
        tagsItem->setForeground(defaultTextColor);
        createdItem->setForeground(defaultTextColor);

        m_taskTable->setItem(row, 0, statusItem);
        m_taskTable->setItem(row, 1, titleItem);
        m_taskTable->setItem(row, 2, assigneeItem);
        m_taskTable->setItem(row, 3, priorityItem);
        m_taskTable->setItem(row, 4, dueDateItem);
        m_taskTable->setItem(row, 5, tagsItem);
        m_taskTable->setItem(row, 6, createdItem);

        if (task.isCompleted())
        {
            QFont font = titleItem->font();
            font.setStrikeOut(true);
            titleItem->setFont(font);

            for (int column = 0; column < m_taskTable->columnCount(); ++column)
            {
                m_taskTable->item(row, column)->setForeground(QColor("#808080"));
            }
        }
        else if (task.isOverdue())
        {
            dueDateItem->setForeground(QColor("#C0392B"));
            titleItem->setForeground(QColor("#C0392B"));
        }
        else if (task.isDueSoon())
        {
            for (int column = 0; column < m_taskTable->columnCount(); ++column)
            {
                m_taskTable->item(row, column)->setBackground(QColor("#FFF4CE"));
            }
            dueDateItem->setForeground(QColor("#A15C00"));
            titleItem->setForeground(QColor("#A15C00"));
        }
        else
        {
            switch (task.priority())
            {
            case TaskPriority::High:
                priorityItem->setForeground(QColor("#C0392B"));
                break;
            case TaskPriority::Low:
                priorityItem->setForeground(QColor("#27864A"));
                break;
            default:
                break;
            }
        }
    }

    m_taskTable->setUpdatesEnabled(true);

    int total = m_taskManager->tasks().size();
    int completed = m_taskManager->completedCount();
    int overdue = m_taskManager->overdueCount();
    QString statusText = QString("Всего: %1 | Выполнено: %2 | Просрочено: %3 | Показано: %4")
                             .arg(total)
                             .arg(completed)
                             .arg(overdue)
                             .arg(tasks.size());

    if (!m_activeFilter.isEmpty())
    {
        statusText += QString(" | Тег: %1").arg(m_activeFilter);
    }

    if (!m_activeAssigneeFilter.isEmpty())
    {
        statusText += QString(" | Исполнитель: %1").arg(m_activeAssigneeFilter);
    }

    if (m_activeStatusFilter >= 0)
    {
        statusText += " | Статус: "
                      + taskStatusText(static_cast<TaskStatus>(m_activeStatusFilter));
    }

    if (!m_searchInput->text().trimmed().isEmpty())
    {
        statusText += QString(" | Поиск: %1").arg(m_searchInput->text().trimmed());
    }

    m_statusLabel->setText(statusText);
}
