#include "gui/MainWindow.hpp"
#include "services/BusyBlockService.hpp"
#include "services/PreferenceService.hpp"
#include "storage/Database.hpp"

#include <QApplication>

namespace {
const char* kStyleSheet = R"(
QMainWindow, QWidget        { background: #1e1e2e; color: #cdd6f4; font-size: 13px; }
QTabWidget::pane            { border: 1px solid #313244; background: #1e1e2e; }
QTabBar::tab                { background: #181825; color: #a6adc8; padding: 8px 20px;
                              border: 1px solid #313244; border-bottom: none; margin-right: 2px; }
QTabBar::tab:selected       { background: #1e1e2e; color: #cba6f7; }
QTabBar::tab:hover:!selected{ background: #232334; color: #cdd6f4; }
QPushButton                 { background: #313244; color: #cdd6f4; border: none;
                              padding: 6px 16px; border-radius: 4px; }
QPushButton:hover           { background: #45475a; }
QPushButton:pressed         { background: #585b70; }
QPushButton:disabled        { background: #181825; color: #585b70; }
QLineEdit, QSpinBox, QComboBox, QDateEdit, QTimeEdit
                            { background: #181825; color: #cdd6f4; border: 1px solid #313244;
                              padding: 4px 8px; border-radius: 4px; }
QLineEdit:focus, QSpinBox:focus, QComboBox:focus { border-color: #cba6f7; }
QComboBox::drop-down        { border: none; }
QComboBox QAbstractItemView { background: #181825; color: #cdd6f4; border: 1px solid #313244;
                              selection-background-color: #313244; }
QListWidget                 { background: #181825; border: 1px solid #313244;
                              border-radius: 4px; outline: none; }
QListWidget::item           { padding: 5px 8px; border-bottom: 1px solid #232334; }
QListWidget::item:selected  { background: #313244; color: #cba6f7; }
QListWidget::item:hover:!selected { background: #232334; }
QTableView                  { background: #181825; gridline-color: #313244;
                              border: 1px solid #313244; selection-background-color: #313244; }
QHeaderView::section        { background: #181825; color: #a6adc8; padding: 6px 8px;
                              border: 1px solid #313244; font-size: 12px; }
QStatusBar                  { background: #181825; color: #6c7086; border-top: 1px solid #313244; }
QStatusBar::item            { border: none; }
QProgressBar                { background: #313244; border: none; border-radius: 4px; text-align: center; }
QProgressBar::chunk         { background: #cba6f7; border-radius: 4px; }
QScrollBar:vertical         { background: #181825; width: 6px; border: none; }
QScrollBar::handle:vertical { background: #45475a; border-radius: 3px; min-height: 20px; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
QScrollBar:horizontal       { background: #181825; height: 6px; border: none; }
QScrollBar::handle:horizontal { background: #45475a; border-radius: 3px; min-width: 20px; }
QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }
QDialog                     { background: #1e1e2e; }
QCheckBox                   { color: #cdd6f4; }
QCheckBox::indicator        { width: 16px; height: 16px; background: #181825;
                              border: 1px solid #313244; border-radius: 3px; }
QCheckBox::indicator:checked{ background: #cba6f7; border-color: #cba6f7; }
QDialogButtonBox QPushButton{ min-width: 80px; }
)";
}  // namespace

int main(int argc, char** argv) {
  QApplication app(argc, argv);
  app.setStyleSheet(kStyleSheet);

  try {
    Database db("studyos.db");
    db.initSchema();

    TaskService task_service(db);
    SessionService session_service(db);
    PreferenceService preference_service(db);
    BusyBlockService busy_block_service(db);

    // TODO(Phase3): Add settings/preferences UI.

    MainWindow window(task_service, session_service, preference_service, busy_block_service);
    window.resize(900, 600);
    window.show();

    return app.exec();
  } catch (const std::exception& ex) {
    // Basic fallback error path for init failures.
    fprintf(stderr, "Fatal: %s\n", ex.what());
    return 1;
  }
}
