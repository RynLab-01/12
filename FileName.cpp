#include <graphics.h>
#include <conio.h>
#include <cstring>
#include <string>
#include <iostream>
#include <vector>
#include <ctime>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <windows.h>
#include <cstdlib>

// ========== 个人标识 ==========
#define STU_ID L"20250612228"
#define STU_NAME L"赵文卉"
#define MAX_USER 50
#define MAX_INPUT_LEN 18
#define WIN_W 800
#define WIN_H 600

struct User
{
    wchar_t username[MAX_INPUT_LEN + 1];
    wchar_t password[MAX_INPUT_LEN + 1];
};
User userList[MAX_USER];
int userCount = 0;
std::wstring loginUser = L"";

struct Record
{
    double speed;
    double accuracy;
    int totalChar;
    int rightChar;
    long long costSec;
};

struct UserConfig
{
    COLORREF textColor;
    COLORREF bgColor;
    int fontSize;
    COLORREF cursorColor;
    int cursorDelay;
    UserConfig()
    {
        textColor = BLACK;
        bgColor = WHITE;
        fontSize = 22;
        cursorColor = BLUE;
        cursorDelay = 100;
    }
};
UserConfig curCfg;

const wchar_t* textPool[] = {
    L"Life is a journey not a destination.",
    L"Never give up on your dreams and goals.",
    L"Practice makes perfect every single day.",
    L"Coding requires patience and careful thinking.",
    L"Small efforts accumulate to great success."
};
const int poolSize = sizeof(textPool) / sizeof(textPool[0]);

void drawStr(int x, int y, const wchar_t* s, int size, COLORREF c);
void drawStrLeft(int x, int y, const wchar_t* s, int size, COLORREF c);
void drawTopBar();
void clearInputBuf();
void loadUsers();
void saveUsers();
std::wstring getCfgFileName();
void loadUserConfig();
void saveUserConfig();
std::wstring getRecordFileName();
void saveRecord(Record r);
std::vector<Record> loadAllRecord();
bool inputPlainText(int x, int y, wchar_t buf[], int maxLen);
bool inputPassword(int x, int y, wchar_t buf[], int maxLen);
bool isUserExist(const wchar_t* un);
bool registerUser(const wchar_t* un, const wchar_t* pw);
bool checkLogin(const wchar_t* un, const wchar_t* pw);
int menuMain();
void startTypePractice();
void openSetting();
void showRecord();
void menuLogin();
void showSystemHelp();

// 绘图函数（宽字符版本，完美匹配EasyX Unicode重载）
void drawStr(int x, int y, const wchar_t* s, int size, COLORREF c)
{
    settextstyle(size, 0, L"宋体");
    settextcolor(c);
    int w = textwidth(s);
    outtextxy(x - w / 2, y, s);
}
void drawStrLeft(int x, int y, const wchar_t* s, int size, COLORREF c)
{
    settextstyle(size, 0, L"宋体");
    settextcolor(c);
    outtextxy(x, y, s);
}

void drawTopBar()
{
    solidrectangle(0, 0, WIN_W, 40);
    wchar_t idBuf[128], nameBuf[128], loginBuf[128];
    swprintf_s(idBuf, L"学号：%s", STU_ID);
    swprintf_s(nameBuf, L"姓名：%s", STU_NAME);
    if (loginUser.empty())
        wcscpy_s(loginBuf, L"当前登录：无");
    else
        swprintf_s(loginBuf, L"当前登录：%s", loginUser.c_str());
    drawStrLeft(20, 8, idBuf, 20, BLACK);
    drawStrLeft(260, 8, nameBuf, 20, BLACK);
    drawStrLeft(520, 8, loginBuf, 20, WHITE);
}

void clearInputBuf()
{
    while (_kbhit()) _getch();
}

void loadUsers()
{
    std::wifstream fin(L"users.txt");
    if (!fin.is_open()) return;
    userCount = 0;
    wchar_t un[MAX_INPUT_LEN + 1] = { 0 }, pw[MAX_INPUT_LEN + 1] = { 0 };
    while (fin >> un >> pw && userCount < MAX_USER)
    {
        wcscpy_s(userList[userCount].username, un);
        wcscpy_s(userList[userCount].password, pw);
        userCount++;
    }
    fin.close();
}

void saveUsers()
{
    std::wofstream fout(L"users.txt");
    for (int i = 0; i < userCount; i++)
        fout << userList[i].username << L" " << userList[i].password << std::endl;
    fout.close();
}

std::wstring getCfgFileName() { return L"cfg_" + loginUser + L".txt"; }
void loadUserConfig()
{
    std::wifstream fin(getCfgFileName());
    if (!fin.is_open()) { curCfg = UserConfig(); return; }
    int t, b, fs, c, d;
    fin >> t >> b >> fs >> c >> d;
    curCfg.textColor = (COLORREF)t;
    curCfg.bgColor = (COLORREF)b;
    curCfg.fontSize = fs;
    curCfg.cursorColor = (COLORREF)c;
    curCfg.cursorDelay = d;
    fin.close();
}
void saveUserConfig()
{
    std::wofstream fout(getCfgFileName());
    fout << (int)curCfg.textColor << L" " << (int)curCfg.bgColor << L" "
        << curCfg.fontSize << L" " << (int)curCfg.cursorColor << L" " << curCfg.cursorDelay;
    fout.close();
}

std::wstring getRecordFileName() { return L"record_" + loginUser + L".txt"; }
void saveRecord(Record r)
{
    std::wofstream fout(getRecordFileName(), std::ios::app);
    fout << r.speed << L" " << r.accuracy << L" " << r.totalChar << L" " << r.rightChar << L" " << r.costSec << std::endl;
    fout.close();
}
std::vector<Record> loadAllRecord()
{
    std::vector<Record> res;
    std::wifstream fin(getRecordFileName());
    Record r;
    while (fin >> r.speed >> r.accuracy >> r.totalChar >> r.rightChar >> r.costSec)
        res.push_back(r);
    fin.close();
    return res;
}

bool inputPlainText(int x, int y, wchar_t buf[], int maxLen)
{
    wmemset(buf, 0, sizeof(wchar_t) * (maxLen + 1));
    int idx = 0;
    while (1)
    {
        cleardevice();
        drawTopBar();
        drawStr(WIN_W / 2, 120, L"用户登录", 36, RED);
        drawStrLeft(x, y - 40, L"请输入账号：", 24, BLACK);
        drawStrLeft(x, y, buf, 24, BLACK);
        char ch = _getch();
        if (ch == '\r') { if (idx == 0) return false; break; }
        if (ch == '\b') { if (idx > 0) buf[--idx] = L'\0'; }
        else if (idx < maxLen && ch >= 32 && ch <= 126) buf[idx++] = ch, buf[idx] = L'\0';
    }
    return true;
}

bool inputPassword(int x, int y, wchar_t buf[], int maxLen)
{
    wmemset(buf, 0, sizeof(wchar_t) * (maxLen + 1));
    int idx = 0;
    while (1)
    {
        cleardevice();
        drawTopBar();
        drawStr(WIN_W / 2, 120, L"用户登录", 36, RED);
        drawStrLeft(x, y - 40, L"请输入密码：", 24, BLACK);
        wchar_t star[MAX_INPUT_LEN + 1] = { 0 };
        for (int i = 0; i < idx; i++) star[i] = L'*';
        drawStrLeft(x, y, star, 24, BLACK);
        char ch = _getch();
        if (ch == '\r') { if (idx == 0) return false; break; }
        if (ch == '\b') { if (idx > 0) buf[--idx] = L'\0'; }
        else if (idx < maxLen && ch >= 32 && ch <= 126) buf[idx++] = ch, buf[idx] = L'\0';
    }
    return true;
}

bool isUserExist(const wchar_t* un)
{
    for (int i = 0; i < userCount; i++)
        if (wcscmp(userList[i].username, un) == 0) return true;
    return false;
}
bool registerUser(const wchar_t* un, const wchar_t* pw)
{
    if (isUserExist(un)) return false;
    wcscpy_s(userList[userCount].username, un);
    wcscpy_s(userList[userCount].password, pw);
    userCount++; saveUsers(); return true;
}
bool checkLogin(const wchar_t* un, const wchar_t* pw)
{
    for (int i = 0; i < userCount; i++)
        if (wcscmp(userList[i].username, un) == 0 && wcscmp(userList[i].password, pw) == 0)
            return true;
    return false;
}

void showSystemHelp()
{
    cleardevice();
    setbkcolor(WHITE);
    cleardevice();
    drawTopBar();
    drawStr(WIN_W / 2, 60, L"系统功能说明", 32, BLUE);
    drawStrLeft(80, 120, L"1.用户注册：创建专属账号，保存个人练习数据", 22, BLACK);
    drawStrLeft(80, 160, L"2.用户登录：登录后使用打字练习、设置、成绩记录", 22, BLACK);
    drawStrLeft(80, 200, L"3.打字练习：随机英文段落，自动计算速度与正确率", 22, BLACK);
    drawStrLeft(80, 240, L"4.练习设置：自定义文字颜色、字体、光标闪烁效果", 22, BLACK);
    drawStrLeft(80, 280, L"5.历史记录：查看每一次打字练习完整成绩", 22, BLACK);
    drawStrLeft(80, 320, L"操作按键：上下键选择菜单、回车确认、ESC退出页面", 22, BLACK);
    drawStr(WIN_W / 2, 450, L"按任意键返回登录首页", 24, BLACK);
    _getch();
}

int menuMain()
{
    int sel = 1;
    while (1)
    {
        cleardevice(); setbkcolor(curCfg.bgColor); cleardevice(); drawTopBar();
        drawStr(WIN_W / 2, 100, L"打字练习系统主菜单", 34, curCfg.textColor);
        COLORREF c1 = sel == 1 ? YELLOW : curCfg.textColor;
        COLORREF c2 = sel == 2 ? YELLOW : curCfg.textColor;
        COLORREF c3 = sel == 3 ? YELLOW : curCfg.textColor;
        COLORREF c4 = sel == 4 ? YELLOW : curCfg.textColor;
        drawStr(WIN_W / 2, 170, L"1. 开始打字练习", 26, c1);
        drawStr(WIN_W / 2, 220, L"2. 练习设置", 26, c2);
        drawStr(WIN_W / 2, 270, L"3. 查看历史成绩", 26, c3);
        drawStr(WIN_W / 2, 320, L"4. 退出登录", 26, c4);
        drawStr(WIN_W / 2, 400, L"上下键切换选项，回车确认", 22, BLACK);
        char ch = _getch();
        if (ch == 72 || ch == 'w' || ch == 'W') sel = sel == 1 ? 4 : sel - 1;
        else if (ch == 80 || ch == 's' || ch == 'S') sel = sel == 4 ? 1 : sel + 1;
        else if (ch == '\r') return sel;
    }
}

void startTypePractice()
{
    srand((unsigned)time(NULL));
    std::wstring target = textPool[rand() % poolSize];
    std::wstring input;
    clock_t st = clock();
    bool showCur = true;
    int flashCnt = 0;
    bool errFlash = false;
    while (1)
    {
        cleardevice(); setbkcolor(curCfg.bgColor); cleardevice(); drawTopBar();
        drawStrLeft(60, 80, L"待输入文本：", 24, curCfg.textColor);
        drawStrLeft(60, 120, target.c_str(), curCfg.fontSize, curCfg.textColor);
        drawStrLeft(60, 180, L"你的输入：", 24, curCfg.textColor);
        int x = 60;
        bool hasErr = false;
        for (int i = 0; i < (int)target.size(); i++)
        {
            wchar_t tmp[2] = { target[i], L'\0' };
            if (i < (int)input.size())
            {
                if (input[i] != target[i])
                {
                    hasErr = true;
                    settextcolor(RED);
                }
                else settextcolor(GREEN);
            }
            else settextcolor(curCfg.textColor);
            settextstyle(curCfg.fontSize, 0, L"宋体");
            outtextxy(x, 180, tmp);
            x += textwidth(tmp);
        }
        if (hasErr) { flashCnt++; if (flashCnt >= 5) { errFlash = !errFlash; flashCnt = 0; } }
        if (showCur)
        {
            settextcolor(curCfg.cursorColor);
            settextstyle(curCfg.fontSize, 0, L"宋体");
            outtextxy(x, 180, L"|");
        }
        showCur = !showCur;

        long long ms = clock() - st;
        long long sec = ms / 1000;
        int total = (int)input.size(), right = 0;
        for (int i = 0; i < total; i++) if (input[i] == target[i]) right++;
        double acc = total == 0 ? 0 : 100.0 * right / total;
        double wpm = sec == 0 ? 0 : (double)total / 5.0 / (sec / 60.0);
        wchar_t infoBuf[256];
        swprintf_s(infoBuf, L"耗时：%llds | 总字符：%d | 正确：%d | 正确率：%.2f%% | WPM速度：%.2f", sec, total, right, acc, wpm);
        drawStrLeft(40, 480, infoBuf, 20, BLACK);
        drawStrLeft(40, 520, L"ESC退出练习，全部输入完成自动保存成绩", 20, BLACK);

        if ((int)input.size() == (int)target.size())
        {
            Record r = { wpm, acc, total, right, sec };
            saveRecord(r);
            cleardevice(); setbkcolor(curCfg.bgColor); cleardevice(); drawTopBar();
            drawStr(WIN_W / 2, 220, L"练习完成！成绩已自动保存", 32, GREEN);
            drawStr(WIN_W / 2, 280, infoBuf, 22, curCfg.textColor);
            drawStr(WIN_W / 2, 360, L"按任意键返回主菜单", 24, BLACK);
            _getch(); return;
        }
        if (_kbhit())
        {
            char ch = _getch();
            if (ch == 27) return;
            if (ch == '\b') { if (!input.empty()) input.pop_back(); }
            else if ((int)input.size() < (int)target.size() && ch >= 32 && ch <= 126) input += ch;
        }
        Sleep(curCfg.cursorDelay);
    }
}

void openSetting()
{
    int sel = 1;
    while (1)
    {
        cleardevice(); setbkcolor(curCfg.bgColor); cleardevice(); drawTopBar();
        drawStr(WIN_W / 2, 80, L"个性化界面设置", 32, BLUE);
        COLORREF c1 = sel == 1 ? YELLOW : curCfg.textColor;
        COLORREF c2 = sel == 2 ? YELLOW : curCfg.textColor;
        COLORREF c3 = sel == 3 ? YELLOW : curCfg.textColor;
        COLORREF c4 = sel == 4 ? YELLOW : curCfg.textColor;
        COLORREF c5 = sel == 5 ? YELLOW : curCfg.textColor;
        COLORREF c6 = sel == 6 ? YELLOW : curCfg.textColor;
        drawStr(WIN_W / 2, 140, L"1.切换文字颜色", 24, c1);
        drawStr(WIN_W / 2, 180, L"2.切换背景底色", 24, c2);
        drawStr(WIN_W / 2, 220, L"3.修改字体大小", 24, c3);
        drawStr(WIN_W / 2, 260, L"4.修改光标颜色", 24, c4);
        drawStr(WIN_W / 2, 300, L"5.修改光标闪烁速度", 24, c5);
        drawStr(WIN_W / 2, 340, L"6.保存配置返回", 24, c6);
        wchar_t cfgBuf[128];
        swprintf_s(cfgBuf, L"当前字体：%d 光标间隔：%dms", curCfg.fontSize, curCfg.cursorDelay);
        drawStr(WIN_W / 2, 420, cfgBuf, 22, BLACK);
        char op = _getch();
        if (op == 72 || op == 'w') sel = sel == 1 ? 6 : sel - 1;
        else if (op == 80 || op == 's') sel = sel == 6 ? 1 : sel + 1;
        else if (op == '\r')
        {
            if (sel == 1) curCfg.textColor = curCfg.textColor == BLACK ? BLUE : BLACK;
            else if (sel == 2) curCfg.bgColor = curCfg.bgColor == WHITE ? CYAN : WHITE;
            else if (sel == 3) curCfg.fontSize = curCfg.fontSize == 22 ? 28 : 22;
            else if (sel == 4) curCfg.cursorColor = curCfg.cursorColor == BLUE ? RED : BLUE;
            else if (sel == 5) curCfg.cursorDelay = curCfg.cursorDelay == 100 ? 200 : 100;
            else if (sel == 6)
            {
                saveUserConfig();
                cleardevice(); drawTopBar();
                drawStr(WIN_W / 2, 240, L"配置保存成功", 30, GREEN);
                drawStr(WIN_W / 2, 320, L"按任意键返回", 24, BLACK);
                _getch(); break;
            }
        }
    }
}

void showRecord()
{
    cleardevice(); setbkcolor(curCfg.bgColor); cleardevice(); drawTopBar();
    drawStr(WIN_W / 2, 70, L"个人历史练习记录", 30, BLUE);
    auto list = loadAllRecord();
    int y = 120;
    if (list.empty())
    {
        drawStr(WIN_W / 2, 220, L"暂无练习记录，请先完成打字练习", 24, BLACK);
    }
    else
    {
        drawStrLeft(30, 120, L"WPM速度 | 正确率 | 总字符 | 正确数 | 耗时(s)", 22, curCfg.textColor);
        y += 40;
        for (auto& r : list)
        {
            wchar_t lineBuf[256];
            swprintf_s(lineBuf, L"%.2f | %.2f%% | %d | %d | %lld", r.speed, r.accuracy, r.totalChar, r.rightChar, r.costSec);
            drawStrLeft(30, y, lineBuf, 20, BLACK);
            y += 30;
        }
    }
    drawStr(WIN_W / 2, 500, L"按任意键返回主菜单", 24, BLACK);
    _getch();
}

void menuLogin()
{
    while (1)
    {
        cleardevice();
        setbkcolor(WHITE);
        cleardevice();
        drawTopBar();
        drawStr(WIN_W / 2, 80, L"英文打字练习系统", 40, BLUE);
        drawStr(WIN_W / 2, 140, L"===== 功能选择菜单 =====", 26, BLACK);
        drawStr(WIN_W / 2, 190, L"1. 登录已有账号", 28, BLACK);
        drawStr(WIN_W / 2, 240, L"2. 新用户注册", 28, BLACK);
        drawStr(WIN_W / 2, 290, L"3. 查看系统使用说明", 28, BLACK);
        drawStr(WIN_W / 2, 340, L"4. 直接退出程序", 28, BLACK);
        drawStr(WIN_W / 2, 400, L"请按下数字键 1/2/3/4 选择对应功能", 22, BLACK);
        drawStr(WIN_W / 2, 450, L"20250612228赵文卉", 20, BLUE);

        char op = _getch();
        wchar_t un[MAX_INPUT_LEN + 1] = { 0 }, pw[MAX_INPUT_LEN + 1] = { 0 };
        if (op == '1')
        {
            if (!inputPlainText(300, 220, un, MAX_INPUT_LEN))
            {
                cleardevice(); drawTopBar(); drawStr(WIN_W / 2, 240, L"账号不能为空", 32, RED); _getch(); continue;
            }
            if (!inputPassword(300, 220, pw, MAX_INPUT_LEN))
            {
                cleardevice(); drawTopBar(); drawStr(WIN_W / 2, 240, L"密码不能为空", 32, RED); _getch(); continue;
            }
            if (checkLogin(un, pw))
            {
                loginUser = un; loadUserConfig();
                while (1)
                {
                    int sel = menuMain();
                    if (sel == 1) startTypePractice();
                    else if (sel == 2) openSetting();
                    else if (sel == 3) showRecord();
                    else if (sel == 4) { loginUser = L""; curCfg = UserConfig(); break; }
                }
            }
            else
            {
                cleardevice(); drawTopBar(); drawStr(WIN_W / 2, 240, L"账号或密码错误", 32, RED); drawStr(WIN_W / 2, 300, L"按任意键返回", 24, BLACK); _getch();
            }
        }
        else if (op == '2')
        {
            if (!inputPlainText(300, 220, un, MAX_INPUT_LEN))
            {
                cleardevice(); drawTopBar(); drawStr(WIN_W / 2, 240, L"账号不能为空", 32, RED); _getch(); continue;
            }
            if (!inputPassword(300, 220, pw, MAX_INPUT_LEN))
            {
                cleardevice(); drawTopBar(); drawStr(WIN_W / 2, 240, L"密码不能为空", 32, RED); _getch(); continue;
            }
            if (registerUser(un, pw))
            {
                cleardevice(); drawTopBar(); drawStr(WIN_W / 2, 240, L"注册成功，请登录", 32, GREEN); drawStr(WIN_W / 2, 300, L"按任意键返回", 24, BLACK); _getch();
            }
            else
            {
                cleardevice(); drawTopBar(); drawStr(WIN_W / 2, 240, L"账号已存在，禁止重复注册", 32, RED); drawStr(WIN_W / 2, 300, L"按任意键返回", 24, BLACK); _getch();
            }
        }
        else if (op == '3')
        {
            showSystemHelp();
        }
        else if (op == '4')
        {
            return;
        }
    }
}

int main()
{
    initgraph(WIN_W, WIN_H);
    setbkcolor(WHITE);
    cleardevice();
    loadUsers();
    menuLogin();
    wchar_t exitBuf[128];
    swprintf_s(exitBuf, L"程序退出，按任意键关闭窗口");
    drawStr(WIN_W / 2, 250, exitBuf, 28, BLACK);
    _getch();
    closegraph();
    return 0;
}