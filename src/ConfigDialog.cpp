#include "ConfigDialog.h"

#include <commctrl.h>
#include <shellscalingapi.h>

#include <algorithm>
#include <string>

#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

namespace {

constexpr wchar_t kClass[] = L"StrayClawdConfig";

enum ControlId : int {
    kIdHelp = 100,
    kIdList,
    kIdLabel,
    kIdSlider,
    kIdValue,
    kIdEnableAll,
    kIdDisableAll,
    kIdReset,
    kIdTest,
};

enum Column : int { kColName, kColGroup, kColWeight, kColChance, kColNote };

struct State {
    Settings edit;
    HWND wnd = nullptr;
    HWND list = nullptr;
    HWND label = nullptr;
    HWND slider = nullptr;
    HWND value = nullptr;
    HWND test = nullptr;
    const std::function<void(int)>* play = nullptr;
    HFONT font = nullptr;
    int dpi = 96;
    bool filling = false;
    bool ok = false;
};

HWND g_open = nullptr;

int Px(const State& st, int v)
{
    return MulDiv(v, st.dpi, 96);
}

const wchar_t* GroupName(AnimGroup g)
{
    switch (g) {
    case AnimGroup::Activity: return L"Activity";
    case AnimGroup::Duo: return L"Duo";
    default: return L"Event";
    }
}

void SetCell(HWND list, int row, int col, const std::wstring& text)
{
    ListView_SetItemText(list, row, col, const_cast<wchar_t*>(text.c_str()));
}

void RefreshCells(State& st)
{
    int total[3] = {};
    for (int i = 0; i < kAnimCount; ++i) {
        total[static_cast<int>(kAnims[i].group)] += st.edit.Weight(i);
    }
    for (int i = 0; i < kAnimCount; ++i) {
        const AnimGroup g = kAnims[i].group;
        const int w = st.edit.Weight(i);
        SetCell(st.list, i, kColWeight, std::to_wstring(st.edit.weight[i]));
        std::wstring chance = L"-";
        if (g == AnimGroup::Event) {
            chance = std::to_wstring(w) + L" %";
        } else if (w > 0) {
            const int t = total[static_cast<int>(g)];
            chance = std::to_wstring((w * 1000 / t + 5) / 10) + L" %";
        }
        SetCell(st.list, i, kColChance, chance);
    }
}

void RefreshSlider(State& st)
{
    const int first = ListView_GetNextItem(st.list, -1, LVNI_SELECTED);
    const int count = ListView_GetSelectedCount(st.list);
    EnableWindow(st.slider, first >= 0);
    EnableWindow(st.test, count == 1);
    if (first < 0) {
        SetWindowTextW(st.label, L"Select one or more rows to set their weight.");
        SetWindowTextW(st.value, L"");
        return;
    }
    const int max = MaxWeight(kAnims[first].group);
    SendMessageW(st.slider, TBM_SETRANGE, TRUE, MAKELPARAM(0, max));
    SendMessageW(st.slider, TBM_SETTICFREQ, max > 20 ? 10 : 1, 0);
    SendMessageW(st.slider, TBM_SETPAGESIZE, 0, max > 20 ? 10 : 1);
    SendMessageW(st.slider, TBM_SETPOS, TRUE, st.edit.weight[first]);
    const std::wstring what = count > 1 ? std::to_wstring(count) + L" rows" : std::wstring(kAnims[first].label);
    const wchar_t* unit = kAnims[first].group == AnimGroup::Event ? L"Percent of " : L"Weight of ";
    SetWindowTextW(st.label, (unit + what + L":").c_str());
    SetWindowTextW(st.value, std::to_wstring(st.edit.weight[first]).c_str());
}

void FillChecks(State& st)
{
    st.filling = true;
    for (int i = 0; i < kAnimCount; ++i) {
        ListView_SetCheckState(st.list, i, st.edit.enabled[i]);
    }
    st.filling = false;
    RefreshCells(st);
    RefreshSlider(st);
}

HWND AddControl(State& st, const wchar_t* cls, const wchar_t* text, DWORD style, int id, int x, int y, int w, int h)
{
    HWND c = CreateWindowExW(0, cls, text, WS_CHILD | WS_VISIBLE | style, Px(st, x), Px(st, y), Px(st, w), Px(st, h),
                             st.wnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), nullptr, nullptr);
    SendMessageW(c, WM_SETFONT, reinterpret_cast<WPARAM>(st.font), FALSE);
    return c;
}

void CreateControls(State& st)
{
    AddControl(st, L"STATIC",
               L"Check a row to enable it. Select one or more rows, then move the slider to set their weight. "
               L"Activities and duos: the chance is the weight divided by the total of the enabled rows of the "
               L"group. Events: the value is a percent. Test (or a double click) plays the selected row now.",
               0, kIdHelp, 12, 10, 556, 48);
    st.list = AddControl(st, WC_LISTVIEWW, L"", WS_BORDER | WS_TABSTOP | LVS_REPORT | LVS_SHOWSELALWAYS, kIdList, 12, 62,
                         556, 400);
    ListView_SetExtendedListViewStyle(st.list, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
    const struct {
        const wchar_t* name;
        int width;
    } kCols[] = {{L"Animation", 200}, {L"Group", 70}, {L"Weight", 60}, {L"Chance", 65}, {L"Note", 138}};
    for (int c = 0; c < 5; ++c) {
        LVCOLUMNW col{};
        col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
        col.pszText = const_cast<wchar_t*>(kCols[c].name);
        col.cx = Px(st, kCols[c].width);
        col.iSubItem = c;
        ListView_InsertColumn(st.list, c, &col);
    }
    st.filling = true;
    for (int i = 0; i < kAnimCount; ++i) {
        LVITEMW it{};
        it.mask = LVIF_TEXT | LVIF_PARAM;
        it.iItem = i;
        it.pszText = const_cast<wchar_t*>(kAnims[i].label);
        it.lParam = i;
        ListView_InsertItem(st.list, &it);
        SetCell(st.list, i, kColGroup, GroupName(kAnims[i].group));
        SetCell(st.list, i, kColNote, kAnims[i].note ? kAnims[i].note : L"");
    }
    st.label = AddControl(st, L"STATIC", L"", 0, kIdLabel, 12, 474, 556, 20);
    st.slider = AddControl(st, TRACKBAR_CLASSW, L"", WS_TABSTOP | TBS_HORZ | TBS_AUTOTICKS, kIdSlider, 6, 494, 480, 32);
    st.value = AddControl(st, L"STATIC", L"", 0, kIdValue, 494, 500, 70, 20);
    AddControl(st, L"BUTTON", L"Enable all", WS_TABSTOP | BS_PUSHBUTTON, kIdEnableAll, 12, 544, 90, 28);
    AddControl(st, L"BUTTON", L"Disable all", WS_TABSTOP | BS_PUSHBUTTON, kIdDisableAll, 108, 544, 90, 28);
    AddControl(st, L"BUTTON", L"Reset defaults", WS_TABSTOP | BS_PUSHBUTTON, kIdReset, 204, 544, 110, 28);
    st.test = AddControl(st, L"BUTTON", L"Test", WS_TABSTOP | BS_PUSHBUTTON, kIdTest, 320, 544, 66, 28);
    AddControl(st, L"BUTTON", L"OK", WS_TABSTOP | BS_DEFPUSHBUTTON, IDOK, 392, 544, 85, 28);
    AddControl(st, L"BUTTON", L"Cancel", WS_TABSTOP | BS_PUSHBUTTON, IDCANCEL, 483, 544, 85, 28);
    FillChecks(st);
}

void OnSlider(State& st)
{
    const int v = static_cast<int>(SendMessageW(st.slider, TBM_GETPOS, 0, 0));
    for (int i = ListView_GetNextItem(st.list, -1, LVNI_SELECTED); i >= 0;
         i = ListView_GetNextItem(st.list, i, LVNI_SELECTED)) {
        st.edit.weight[i] = std::min(v, MaxWeight(kAnims[i].group));
    }
    SetWindowTextW(st.value, std::to_wstring(v).c_str());
    RefreshCells(st);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
    }
    auto* st = reinterpret_cast<State*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    if (!st) {
        return DefWindowProcW(hwnd, msg, wp, lp);
    }
    switch (msg) {
    case WM_CREATE:
        st->wnd = hwnd;
        CreateControls(*st);
        return 0;
    case WM_NOTIFY: {
        const auto* hdr = reinterpret_cast<const NMHDR*>(lp);
        if (hdr->idFrom == kIdList && hdr->code == NM_DBLCLK) {
            const auto* ia = reinterpret_cast<const NMITEMACTIVATE*>(lp);
            if (ia->iItem >= 0 && st->play && *st->play) {
                (*st->play)(ia->iItem);
            }
            return 0;
        }
        if (hdr->idFrom == kIdList && hdr->code == LVN_ITEMCHANGED) {
            const auto* nm = reinterpret_cast<const NMLISTVIEW*>(lp);
            if ((nm->uChanged & LVIF_STATE) == 0 || nm->iItem < 0) {
                return 0;
            }
            if (((nm->uNewState ^ nm->uOldState) & LVIS_STATEIMAGEMASK) && !st->filling) {
                st->edit.enabled[nm->iItem] = ListView_GetCheckState(st->list, nm->iItem) != FALSE;
                RefreshCells(*st);
            }
            if ((nm->uNewState ^ nm->uOldState) & LVIS_SELECTED) {
                RefreshSlider(*st);
            }
        }
        return 0;
    }
    case WM_HSCROLL:
        if (reinterpret_cast<HWND>(lp) == st->slider) {
            OnSlider(*st);
        }
        return 0;
    case WM_COMMAND:
        switch (LOWORD(wp)) {
        case IDOK:
            st->ok = true;
            DestroyWindow(hwnd);
            break;
        case IDCANCEL:
            DestroyWindow(hwnd);
            break;
        case kIdEnableAll:
        case kIdDisableAll:
            for (bool& e : st->edit.enabled) {
                e = LOWORD(wp) == kIdEnableAll;
            }
            FillChecks(*st);
            break;
        case kIdReset:
            st->edit.ResetAnimations();
            FillChecks(*st);
            break;
        case kIdTest: {
            const int row = ListView_GetNextItem(st->list, -1, LVNI_SELECTED);
            if (row >= 0 && st->play && *st->play) {
                (*st->play)(row);
            }
            SetFocus(st->list);
            break;
        }
        default:
            break;
        }
        return 0;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    default:
        break;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

}

bool RunConfigDialog(HINSTANCE instance, Settings& settings, const std::function<void(int)>& test)
{
    if (g_open) {
        SetForegroundWindow(g_open);
        return false;
    }
    static bool registered = false;
    if (!registered) {
        INITCOMMONCONTROLSEX icc{sizeof(icc), ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES};
        InitCommonControlsEx(&icc);
        WNDCLASSEXW wc{};
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = &WndProc;
        wc.hInstance = instance;
        wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
        wc.hIcon = static_cast<HICON>(LoadImageW(nullptr, IDI_APPLICATION, IMAGE_ICON, 0, 0, LR_SHARED));
        wc.lpszClassName = kClass;
        registered = RegisterClassExW(&wc) != 0;
        if (!registered) {
            return false;
        }
    }

    State st;
    st.edit = settings;
    st.play = &test;
    POINT cursor{};
    GetCursorPos(&cursor);
    HMONITOR mon = MonitorFromPoint(cursor, MONITOR_DEFAULTTONEAREST);
    UINT dx = 96;
    UINT dy = 96;
    if (SUCCEEDED(GetDpiForMonitor(mon, MDT_EFFECTIVE_DPI, &dx, &dy))) {
        st.dpi = static_cast<int>(dx);
    }
    NONCLIENTMETRICSW ncm{};
    ncm.cbSize = sizeof(ncm);
    SystemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0, st.dpi);
    st.font = CreateFontIndirectW(&ncm.lfMessageFont);

    const DWORD style = WS_POPUP | WS_CAPTION | WS_SYSMENU;
    const DWORD exStyle = WS_EX_DLGMODALFRAME;
    RECT rc{0, 0, Px(st, 580), Px(st, 584)};
    AdjustWindowRectExForDpi(&rc, style, FALSE, exStyle, st.dpi);
    MONITORINFO mi{};
    mi.cbSize = sizeof(mi);
    GetMonitorInfoW(mon, &mi);
    const int w = rc.right - rc.left;
    const int h = rc.bottom - rc.top;
    const int x = mi.rcWork.left + std::max(0, static_cast<int>((mi.rcWork.right - mi.rcWork.left - w) / 2));
    const int y = mi.rcWork.top + std::max(0, static_cast<int>((mi.rcWork.bottom - mi.rcWork.top - h) / 2));
    HWND wnd = CreateWindowExW(exStyle, kClass, L"StrayClawd - Configure the animations", style, x, y, w, h, nullptr,
                               nullptr, instance, &st);
    if (!wnd) {
        DeleteObject(st.font);
        return false;
    }
    g_open = wnd;
    ShowWindow(wnd, SW_SHOW);
    SetForegroundWindow(wnd);
    SetFocus(st.list);

    MSG msg{};
    while (IsWindow(wnd)) {
        const BOOL r = GetMessageW(&msg, nullptr, 0, 0);
        if (r == 0) {
            PostQuitMessage(static_cast<int>(msg.wParam));
            break;
        }
        if (r < 0) {
            break;
        }
        if (!IsDialogMessageW(wnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
    if (IsWindow(wnd)) {
        DestroyWindow(wnd);
    }
    g_open = nullptr;
    DeleteObject(st.font);
    if (!st.ok) {
        return false;
    }
    std::copy(std::begin(st.edit.weight), std::end(st.edit.weight), std::begin(settings.weight));
    std::copy(std::begin(st.edit.enabled), std::end(st.edit.enabled), std::begin(settings.enabled));
    return true;
}
