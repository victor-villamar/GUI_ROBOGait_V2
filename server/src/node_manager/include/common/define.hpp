#pragma once

#define EN_CASA 0
#define ROBOT 1
#define NAV 0

#if EN_CASA && !ROBOT
#define PATH "/home/andri/Desktop/gui_nuevo/gui_/" // casa

#elif !ROBOT
#define PATH "/home/robogait/Desktop/gui_andri/GUI_ROBOGait/" // uni
#else
#define PATH "/home/robogait/GUI_ROBOGait/" // robot
#endif