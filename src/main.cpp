#include "gui/MainWindow.hpp"
#include "services/BusyBlockService.hpp"
#include "services/PreferenceService.hpp"
#include "storage/Database.hpp"

#include <QApplication>

int main(int argc, char** argv) {
  QApplication app(argc, argv);

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
