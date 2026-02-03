cmake_minimum_required(VERSION 3.16)
project(StudyOS LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SQLite3 REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(studyos
  src/main.cpp
  src/models.hpp
  src/storage/Database.cpp
  src/storage/Database.hpp
  src/services/TaskService.cpp
  src/services/TaskService.hpp
  src/services/SessionService.cpp
  src/services/SessionService.hpp
  src/services/PreferenceService.cpp
  src/services/PreferenceService.hpp
  src/services/BusyBlockService.cpp
  src/services/BusyBlockService.hpp
  src/scheduler/Scheduler.cpp
  src/scheduler/Scheduler.hpp
  src/gui/AppState.hpp
  src/gui/MainWindow.hpp
  src/gui/MainWindow.cpp
  src/gui/TimerWidget.hpp
  src/gui/TimerWidget.cpp
  src/gui/TasksWidget.hpp
  src/gui/TasksWidget.cpp
  src/gui/StatsWidget.hpp
  src/gui/StatsWidget.cpp
  src/gui/ScheduleWidget.hpp
  src/gui/ScheduleWidget.cpp
  src/gui/models/ScheduleTableModel.hpp
  src/gui/models/ScheduleTableModel.cpp
  src/gui/dialogs/BusyBlockDialog.hpp
  src/gui/dialogs/BusyBlockDialog.cpp
  src/gui/dialogs/PreferenceDialog.hpp
  src/gui/dialogs/PreferenceDialog.cpp
)

target_include_directories(studyos PRIVATE src)

target_link_libraries(studyos PRIVATE SQLite::SQLite3 Qt6::Widgets)

set_target_properties(studyos PROPERTIES
  AUTOMOC ON
  AUTOUIC ON
  AUTORCC ON
)
