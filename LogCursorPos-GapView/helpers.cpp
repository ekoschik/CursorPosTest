
#include "helpers.h"
using namespace std;

class MouseSpeedBuffer
{
    std::list<POINT> ptBuffer;
    int _iAvgSpeed = 0;
    POINT _ptLast;
public:
    MouseSpeedBuffer() { ptBuffer.resize(POS_BUFFER_SIZE); }
    void AddPoint(POINT pt);
    void GetRenderData(WindowRenderData* pData);
};

__inline double dist(POINT pt1, POINT pt2)
{
    return sqrt(pow((double)pt2.x - (double)pt1.x, 2) +
        pow((double)pt2.y - (double)pt1.y, 2));
}

void MouseSpeedBuffer::AddPoint(POINT pt)
{
    // Make room in the buffer (if necessary) then add pt
    if (ptBuffer.size() >= POS_BUFFER_SIZE) {
        ptBuffer.pop_front();
    }
    ptBuffer.push_back(pt);
    _ptLast = pt;

    // Re claculate total distance (kinda wasteful but whatev's)
    auto it = ptBuffer.begin();
    POINT ptPrev = *it;
    double totalDistance = 0;
    while (it != ptBuffer.end()) {
        POINT ptCur = *it;
        totalDistance += dist(ptPrev, ptCur);
        ptPrev = ptCur;
        ++it;
    }

    // Update avg speed and return whether or not it changed
    int iAvgSpeedPrev = _iAvgSpeed;
    _iAvgSpeed = totalDistance / ptBuffer.size();
}

map<HWND, MouseSpeedBuffer*> WndMap;

MouseSpeedBuffer* GetDataForWindow(HWND hwnd)
{
    auto it = WndMap.find(hwnd);
    return it != WndMap.end() ? it->second : NULL;
}

void AddWindowToMap(HWND hwnd)
{
    if (GetDataForWindow(hwnd) == nullptr) {
        WndMap[hwnd] = new MouseSpeedBuffer;
    }
}

bool RemoveWindowFromMap(HWND hwnd)
{
    MouseSpeedBuffer* msd = GetDataForWindow(hwnd);
    if (msd != nullptr) {
        delete msd;
        WndMap.erase(hwnd);
    }

    return WndMap.size() == 0;
}

void AddPoint(HWND hwnd, POINT pt)
{
    MouseSpeedBuffer* msd = GetDataForWindow(hwnd);
    if (msd != nullptr) {
        msd->AddPoint(pt);
    }
}

COLORREF ColorFromPercentage(int percentage)
{
    COLORREF c1 = (percentage < 50) ? RGBMin : RGBMiddle;
    COLORREF c2 = (percentage < 50) ? RGBMiddle : RGBMax;
    double fraction = (percentage < 50) ?
        percentage / 50.0 : (percentage - 50) / 50.0;

#define CLRMATH(d1, d2, f) (int)(d1 + (d2 - d1) * (double)f)
    return RGB(CLRMATH((double)GetRValue(c1), (double)GetRValue(c2), fraction),
               CLRMATH((double)GetGValue(c1), (double)GetGValue(c2), fraction),
               CLRMATH((double)GetBValue(c1), (double)GetBValue(c2), fraction));
}

HBRUSH BrushFromPercentage(int p)
{
    p = max(0, min(99, p));
    static HBRUSH hbrCache[100] = {};
    static bool bCacheValid[100] = {};

    if (!bCacheValid[p]) {
        hbrCache[p] = CreateSolidBrush(ColorFromPercentage(p));
        bCacheValid[p] = true;
    }

    return hbrCache[p];
}

COLORREF TextBrushFromPercentage(int p)
{
    COLORREF color = ColorFromPercentage(p);

    if (1 - (0.299 * GetRValue(color) + 0.587 *
        GetGValue(color) + 0.114 * GetBValue(color)) / 255 < 0.5) {
        return RGB(0, 0, 0);
    }

    return RGB(255, 255, 255);
}

void MouseSpeedBuffer::GetRenderData(WindowRenderData* pData)
{
    pData->avgSpeed = _iAvgSpeed;
    pData->percentageOfGoal = (int)max(0, min(99, (double)_iAvgSpeed / GOAL));
    pData->hbrBackground = BrushFromPercentage(pData->percentageOfGoal);
    pData->rgbText = TextBrushFromPercentage(pData->percentageOfGoal);
    pData->ptLast = _ptLast;
}

bool GetRenderDataForWindow(HWND hwnd, WindowRenderData* pwrd)
{
    MouseSpeedBuffer* msd = GetDataForWindow(hwnd);
    if (msd != nullptr) {
        msd->GetRenderData(pwrd);
    }
    return msd != nullptr;
}
