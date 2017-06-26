
#pragma once
#include <Windows.h>
#include <Windowsx.h>
#include <cmath>
#include <float.h>
#include <map>
#include <list>

// Each window stores a buffer of cursor positions it gets back from GetCursorPos,
// which are used to get an average mouse speed over the last five seconds.
#define TIMER_DELAY 50                      // milisecs each window waits before calling GetCursorPos
#define POS_BUFFER_SIZE 5000 / TIMER_DELAY  // store 5 seconds of position data per window

// To translate speed into color, an arbitrary average distance is chosen,
// and the percentage the current average distance is of the goal is shown
// by a color gradient from blue to yellow to red (ColorFromPercentage).
#define GOAL        20                      // 100% speed
#define RGBMin      RGB(0, 0, 255)          // blue   (0% of goal)
#define RGBMiddle   RGB(255, 255, 0)        // yellow (50%)
#define RGBMax      RGB(255, 0, 0)          // red    (100% of goal)

void AddWindowToMap(HWND hwnd);
bool RemoveWindowFromMap(HWND hwnd);
void AddPoint(HWND hwnd, POINT pt);

struct WindowRenderData {
    int avgSpeed;
    int percentageOfGoal;
    HBRUSH hbrBackground;
    COLORREF rgbText;
    POINT ptLast;
};

bool GetRenderDataForWindow(HWND hwnd, WindowRenderData* pwrd);

