include(${CMAKE_BINARY_DIR}/_deps/xcmake_tools/Common.cmake)

# Fetch dependencies
FetchAndPopulate("https://github.com/LIONant-depot/xcontainer.git" "main")

DefineInterfaceComponent(xscheduler "dependencies/xcore"
  "source/xscheduler.h"
  "source/xscheduler_jobs.h"
  "source/xscheduler_system.h"
  "source/xscheduler_task_group.h"
  "source/xscheduler_triggers.h"
  "Readme.md"
  "**Implementation"
  "source/Implementation/xscheduler_jobs_inline.h"
  "source/Implementation/xscheduler_system_inline.h"
  "source/Implementation/xscheduler_task_group_inline.h"
  "source/Implementation/xscheduler_triggers_inline.h"
)
