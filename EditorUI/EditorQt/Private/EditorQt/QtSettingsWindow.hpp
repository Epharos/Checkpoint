#pragma once

#include "EditorQtCommon.hpp"

#include <QDialog>
#include <QFrame>
#include <QLabel>
#include <QListWidget>
#include <QStackedWidget>
#include <QTableWidget>
#include <QFont>

namespace cp::editorqt
{
    class QtKeyCaptureDialog final : public QDialog
    {
    public:
        explicit QtKeyCaptureDialog(QWidget* _parent = nullptr)
            : QDialog(_parent)
        {
            setWindowTitle("Set Keybind");
            setModal(true);
            setMinimumSize(320, 110);
            setFocusPolicy(Qt::StrongFocus);

            auto* layout = new QVBoxLayout(this);

            label = new QLabel("Press any key combination...", this);
            label->setAlignment(Qt::AlignCenter);
            layout->addWidget(label);

            auto* buttonRow = new QHBoxLayout();
            clearButton  = new QPushButton("Clear",  this);
            cancelButton = new QPushButton("Cancel", this);
            buttonRow->addWidget(clearButton);
            buttonRow->addStretch();
            buttonRow->addWidget(cancelButton);
            layout->addLayout(buttonRow);

            connect(clearButton,  &QPushButton::clicked, [this]() { capturedChord.clear(); accept(); });
            connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
        }

        std::string capturedChord;

    protected:
        void keyPressEvent(QKeyEvent* _event) override
        {
            if (_event->key() == Qt::Key_Control || _event->key() == Qt::Key_Shift ||
                _event->key() == Qt::Key_Alt || _event->key() == Qt::Key_Meta)
            {
                return;
            }

            capturedChord = QKeySequence(_event->key() | _event->modifiers())
                .toString(QKeySequence::PortableText)
                .toStdString();
            label->setText(QString::fromStdString(capturedChord));
            accept();
        }

    private:
        QLabel* label = nullptr;
        QPushButton* clearButton = nullptr;
        QPushButton* cancelButton = nullptr;
    };

    class QtSettingsWindow final : public cp::editorui::ISettingsWindow
    {
    public:
        explicit QtSettingsWindow(QWidget* _parent = nullptr)
            : dialog(new QDialog(_parent))
        {
            dialog->setWindowTitle("Editor Settings");
            dialog->setModal(true);
            dialog->setMinimumSize(820, 520);

            auto* root = new QHBoxLayout(dialog.get());
            root->setContentsMargins(0, 0, 0, 0);
            root->setSpacing(0);

            sectionList = new QListWidget(dialog.get());
            sectionList->setMaximumWidth(160);
            sectionList->setMinimumWidth(140);
            sectionList->addItem("Keybinds");
            root->addWidget(sectionList);

            auto* divider = new QFrame(dialog.get());
            divider->setFrameShape(QFrame::VLine);
            divider->setFrameShadow(QFrame::Sunken);
            root->addWidget(divider);

            auto* right = new QVBoxLayout();
            right->setContentsMargins(8, 8, 8, 8);
            right->setSpacing(6);

            stack = new QStackedWidget(dialog.get());
            stack->addWidget(BuildKeybindsPage());
            right->addWidget(stack);

            auto* btnRow = new QHBoxLayout();
            resetBtn = new QPushButton("Reset All", dialog.get());
            applyBtn = new QPushButton("Apply", dialog.get());
            okBtn = new QPushButton("OK", dialog.get());
            cancelBtn = new QPushButton("Cancel", dialog.get());
            btnRow->addWidget(resetBtn);
            btnRow->addStretch();
            btnRow->addWidget(applyBtn);
            btnRow->addWidget(okBtn);
            btnRow->addWidget(cancelBtn);
            right->addLayout(btnRow);

            root->addLayout(right);

            QObject::connect(sectionList, &QListWidget::currentRowChanged, stack, &QStackedWidget::setCurrentIndex);
            sectionList->setCurrentRow(0);

            QObject::connect(applyBtn, &QPushButton::clicked, [this]() { ApplyChanges(); });
            QObject::connect(okBtn, &QPushButton::clicked, [this]() { ApplyChanges(); dialog->accept(); });
            QObject::connect(cancelBtn, &QPushButton::clicked, dialog.get(), &QDialog::reject);
            QObject::connect(resetBtn, &QPushButton::clicked, [this]() { ResetAllToDefaults(); });
        }

        void SetKeybindEntries(std::vector<cp::editorui::KeybindSettingsEntry> _entries) override
        {
            entries = std::move(_entries);
            pendingChords.clear();
            RefreshTable();
        }

        void SetApplyKeybindsHandler(ApplyKeybindsHandler _handler) override
        {
            applyHandler = std::move(_handler);
        }

        void Show() override
        {
            dialog->exec();
        }

    private:
        QWidget* BuildKeybindsPage()
        {
            auto* page = new QWidget();
            auto* layout = new QVBoxLayout(page);
            layout->setContentsMargins(0, 0, 0, 0);

            table = new QTableWidget(page);
            table->setColumnCount(4);
            table->setHorizontalHeaderLabels({"Category", "Action", "Shortcut", "Scope"});
            table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
            table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
            table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
            table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
            table->setSelectionBehavior(QAbstractItemView::SelectRows);
            table->setEditTriggers(QAbstractItemView::NoEditTriggers);
            table->setAlternatingRowColors(true);

            auto* hint = new QLabel("Double-click the Shortcut column to remap a keybind.", page);
            hint->setEnabled(false);
            layout->addWidget(hint);
            layout->addWidget(table);

            QObject::connect(table, &QTableWidget::cellDoubleClicked, [this](int _row, int _col)
            {
                if (_col == 2)
                {
                    StartRemap(_row);
                }
            });

            return page;
        }

        void RefreshTable() const
        {
            table->setRowCount(static_cast<int>(entries.size()));

            std::unordered_map<std::string, std::vector<int>> chordRows;
            for (int i = 0; i < static_cast<int>(entries.size()); ++i)
            {
                const std::string chord = CurrentChord(i);
                if (!chord.empty())
                    chordRows[chord].push_back(i);
            }

            for (int i = 0; i < static_cast<int>(entries.size()); ++i)
            {
                const auto& e = entries[i];
                const std::string chord = CurrentChord(i);
                const bool conflict = !chord.empty() && chordRows[chord].size() > 1;
                const bool modified = HasPendingChange(e.actionId) && chord != e.defaultKey;

                auto makeItem = [](const QString& text) -> QTableWidgetItem*
                {
                    auto* item = new QTableWidgetItem(text);
                    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                    return item;
                };

                auto* chordItem = makeItem(QString::fromStdString(chord));

                if (conflict)
                {
                    chordItem->setForeground(QColor(220, 60, 60));
                }

                if (modified)
                {
                    QFont f = chordItem->font();
                    f.setBold(true);
                    chordItem->setFont(f);
                }

                table->setItem(i, 0, makeItem(QString::fromStdString(e.category)));
                table->setItem(i, 1, makeItem(QString::fromStdString(e.displayName)));
                table->setItem(i, 2, chordItem);
                QString scopeDisplay = QString::fromStdString(e.scope);
                if (!scopeDisplay.isEmpty())
                    scopeDisplay[0] = scopeDisplay[0].toUpper();
                table->setItem(i, 3, makeItem(scopeDisplay));
            }
        }

        std::string CurrentChord(int _row) const
        {
            const auto it = pendingChords.find(entries[_row].actionId);
            return it != pendingChords.end() ? it->second : entries[_row].currentKey;
        }

        bool HasPendingChange(const std::string& _actionId) const
        {
            return pendingChords.count(_actionId) > 0;
        }

        void StartRemap(int _row)
        {
            QtKeyCaptureDialog capture(dialog.get());
            if (capture.exec() == QDialog::Accepted)
            {
                pendingChords[entries[_row].actionId] = capture.capturedChord;
                RefreshTable();
            }
        }

        void ApplyChanges()
        {
            if (!applyHandler || pendingChords.empty())
                return;

            std::vector<std::pair<std::string, std::string>> changes(
                pendingChords.begin(), pendingChords.end()
            );

            applyHandler(std::move(changes));

            for (auto& e : entries)
            {
                const auto it = pendingChords.find(e.actionId);

                if (it != pendingChords.end())
                {
                    e.currentKey = it->second;
                }
            }

            pendingChords.clear();
            RefreshTable();
        }

        void ResetAllToDefaults()
        {
            pendingChords.clear();

            for (const auto& e : entries)
            {
                pendingChords[e.actionId] = e.defaultKey;
            }

            RefreshTable();
        }

        std::unique_ptr<QDialog> dialog;
        QListWidget* sectionList = nullptr;
        QStackedWidget* stack = nullptr;
        QTableWidget* table = nullptr;
        QPushButton* resetBtn = nullptr;
        QPushButton* applyBtn = nullptr;
        QPushButton* okBtn = nullptr;
        QPushButton* cancelBtn = nullptr;

        std::vector<cp::editorui::KeybindSettingsEntry> entries;
        std::unordered_map<std::string, std::string> pendingChords;
        ApplyKeybindsHandler applyHandler;
    };
}
