#include <windows.h>
#include <cstdio>
#include <cmath>

#define TIME(h, m, s) (((int)h << 16) | ((int)m << 8) | (int)s)

UINT_PTR timer;
FLASHWINFO flash_inf;
HANDLE mutex_lock;

const int h2o_goal = 2120;
const int shot_ml = 66;
const char h_start = 6;
const char m_start = 0;
const char h_end = 20;
const char m_end = 0;

bool hybernation;

int h2o_consumed;
int shots_need;
char int_min, int_sec, init_int_min, init_int_sec; // Interval

int calcInterval(int start, int end); // OUT: raw seconds IN: TIME MACRO
void flash(bool on_off);
void prnt(const char* t);
void pc(char c);
void pi(long long int i);
void pshots();
void pbar();
char getch();
void hideCursor();
void tproc(HWND p1, UINT p2, UINT_PTR p3, DWORD p4);
DWORD WINAPI input_thread(LPVOID lp);

int  APIENTRY WinMain(HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPSTR lpCmdLine,
                      int nCmdShow)
{ 	
	mutex_lock = CreateMutex(NULL, FALSE, NULL);
	HANDLE thread = CreateThread(NULL, 0, input_thread, NULL, 0, NULL);
	
	COORD buf_s;
	buf_s.X = 79;
	buf_s.Y = 20;
	SMALL_RECT wnd_pos;
	wnd_pos.Top = 0;
	wnd_pos.Left = 0;
	wnd_pos.Bottom = 19;
	wnd_pos.Right = 78;
	HANDLE wh = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleWindowInfo(wh, TRUE, &wnd_pos);
	SetConsoleScreenBufferSize(wh, buf_s);
	
	SetConsoleTitle("H2O");
	hideCursor();
	
	HWND wnd = GetConsoleWindow();
	
	flash_inf.cbSize = sizeof(FLASHWINFO);
	flash_inf.hwnd = wnd;
	//flash.dwFlags = FLASHW_TRAY | FLASHW_TIMER | 1; // Flash ALL
	flash_inf.uCount = 0;
	flash_inf.dwTimeout = 500;
	
	SYSTEMTIME lt;
	GetLocalTime(&lt);
	
	int itot = calcInterval(TIME(h_start, m_start, 0), TIME(h_end, m_end, 0));
	int ielasp = calcInterval(TIME(h_start, m_start, 0), TIME(lt.wHour, lt.wMinute, lt.wSecond));
	
	int interval = (int)floor((float)itot/floor((float)h2o_goal/(float)shot_ml));
	int_min = char(interval/60);
	int_sec = char(interval % 60);
	
	shots_need = ielasp/interval + 1;
	int int_interval = interval - ielasp % interval;
	init_int_min = char(int_interval/60);
	init_int_sec = char(int_interval % 60);
	
	if((lt.wHour >= h_end || lt.wHour < h_start) && (lt.wMinute >= m_end || lt.wMinute < m_start))
	{
		hybernation = true;
	}
	else
	{
		flash(1);
	}
	
	timer = SetTimer(NULL, NULL, 1000, tproc);
	
	pbar();
	
	MSG msg;
	int count = 0;
	while(GetMessage(&msg, NULL, 0, 0))
	{
		/* ++count;
		if(msg.message == WM_TIMER)
		{
			prnt("TIMERMSG!!!");
		}
		
		char txt[100];
		sprintf(txt, "c:%d msg:%d\n", (long long int)count, msg.message);
		prnt(txt);*/
		
		//hideCursor();
		
		DispatchMessage(&msg);
	}
	
	/* while(1)
	{
		FlashWindow(wnd, 1);
		Sleep(500);
		FlashWindow(wnd, 1);
		Sleep(500);
	} */
	KillTimer(NULL, timer);
	return 0;
}

int calcInterval(int start, int end)
{
	char h, m, s;
	char hs = start >> 16;
	char ms = (start & 0xFF00) >> 8;
	char ss = start & 0xFF;
	char he = end >> 16;
	char me = (end & 0xFF00) >> 8;
	char se =  end & 0xFF;
	
	if(he <= hs)
	{
		h = 24 - hs + he;
	}
	else
	{
		h = he - hs;
	}
	m = me - ms;
	s = se - ss;
	
	if(s < 0)
	{
		--m;
		s = 60 + s;
	}
	
	if(m < 0)
	{
		--h;
		m += 60;
	}
	else if(h == 24)
	{
		h = 0;
	}
	
	return h * 3600 + m * 60 + s;
}

void flash(bool on_off)
{
	if(hybernation)
	{
		return;
	}
	
	if(on_off)
	{
		flash_inf.dwFlags = FLASHW_TRAY | FLASHW_TIMER;
		FlashWindowEx(&flash_inf);
	}
	else
	{
		flash_inf.dwFlags = 0;
		FlashWindowEx(&flash_inf);
	}
}

void prnt(const char* t)
{
	long unsigned int wrt = 0;
	HANDLE stdo = GetStdHandle(STD_OUTPUT_HANDLE);
	WriteConsole(stdo, t, strlen(t), &wrt, NULL);
}

void pc(char c)
{
	long unsigned int wrt = 0;
	HANDLE stdo = GetStdHandle(STD_OUTPUT_HANDLE);
	WriteConsole(stdo, &c, 1, &wrt, NULL);
}

void pi(long long int i)
{
	char txt[100];
	sprintf(txt, "%d", i);
	prnt(txt);
}

void pshots()
{
	COORD shotz;
	shotz.X = 29;
	shotz.Y = 2;
	HANDLE oh = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(oh, shotz);
	pi(shots_need);
	prnt("  ");
}

void pbar()
{
	HANDLE oh = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD comp;
	comp.X = 0;
	comp.Y = 0;
	COORD xy;
	xy.X = 27;
	xy.Y = 1;
	
	SetConsoleCursorPosition(oh, comp);
	prnt("[");                                                           
	for(int j = 0; j < 60; ++j)
	{							
		pc((unsigned char)176);
	}	
	prnt("] 0% 0/");
	pi(h2o_goal);
	prnt("      ");
	SetConsoleCursorPosition(oh, xy);
	char t[6];
	sprintf(t, "%02d:%02d", init_int_min, init_int_sec);
	prnt(t);
	pshots();
}

char getch()
{
    DWORD mode, cc;
    HANDLE h = GetStdHandle(STD_INPUT_HANDLE);

    GetConsoleMode( h, &mode );
    SetConsoleMode( h, mode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT));
    char c = 0;
    ReadConsole( h, &c, 1, &cc, NULL );
    SetConsoleMode( h, mode );
	
	//pi(mode);
	
    return c;
}

void hideCursor()
{
	HANDLE chand = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO inf;
	inf.dwSize = 100;
	inf.bVisible = FALSE;
	SetConsoleCursorInfo(chand, &inf);
}

void tproc(HWND p1, UINT p2, UINT_PTR p3, DWORD p4)
{
	hideCursor();
	
	SetTimer(NULL, timer, 1000, tproc);
	
	static char min = init_int_min;
	static char sec = init_int_sec;
	
	--sec;
	if(sec < 0)
	{
		sec = 59;
		--min;
	}
	
	static SYSTEMTIME lt;
	GetLocalTime(&lt);
	
	if(lt.wHour == h_end && lt.wMinute == m_end)
	{
		flash(0);
		hybernation = true;
	}
	else if(hybernation && lt.wHour == h_start && lt.wMinute == m_start)
	{
		hybernation = false;
		flash(1);
		min = int_min;
		sec = int_sec;
		shots_need = 1;
		h2o_consumed = 0;
		pbar();
		return;
	}
	
	char t[6];
	sprintf(t, "%02d:%02d", min, sec);
	
	COORD xy;
	xy.X = 27;
	xy.Y = 1;
	
	HANDLE oh = GetStdHandle(STD_OUTPUT_HANDLE);
	
	// CRITICAL SECTION
	WaitForSingleObject(mutex_lock, INFINITE);
	
	SetConsoleCursorPosition(oh, xy);
	prnt(t);
	
	ReleaseMutex(mutex_lock);
	// CRITICAL SECTION
	
	if(!min && !sec)
	{
		if(h2o_consumed + shots_need * shot_ml < h2o_goal)
		{
			++shots_need;
		}
		
		min = int_min;
		sec = int_sec;
		pshots();
		
		if(shots_need > 0)
		{
			flash(1);
		}
	}
	
	hideCursor();
}

DWORD WINAPI input_thread(LPVOID lp)
{
	DWORD mode, cc;
	HANDLE ih = GetStdHandle(STD_INPUT_HANDLE);
	HANDLE oh = GetStdHandle(STD_OUTPUT_HANDLE);
	HWND wnd = GetConsoleWindow();
	COORD comp;
	comp.X = 0;
	comp.Y = 0;

    GetConsoleMode( ih, &mode );
    SetConsoleMode( ih, ENABLE_EXTENDED_FLAGS | ENABLE_MOUSE_INPUT | ENABLE_PROCESSED_INPUT);
    char c = 0;
	char t[50];
	
	CONSOLE_SCREEN_BUFFER_INFO scr_inf;
	GetConsoleScreenBufferInfo(oh, &scr_inf);
	WORD dtext = scr_inf.wAttributes;
	
	INPUT_RECORD ir[128];
	DWORD nread;
	while(1)
	{
		//ReadConsole( h, &c, 1, &cc, NULL );
		//prnt("IT FUKIN WORKZ*******");
		ReadConsoleInput(ih, ir, 128, &nread);
		for(int i = 0; i < nread; ++i)
		{
			if(ir[i].EventType == MOUSE_EVENT)
			{
				const MOUSE_EVENT_RECORD* m = &ir[i].Event.MouseEvent;
				if(m->dwButtonState == 0x1)
				{
					if(shots_need > -5)
					{
						--shots_need;
						h2o_consumed += shot_ml;
						
						if(h2o_consumed >= h2o_goal)
						{
							h2o_consumed = h2o_goal;
						}
						
						if(shots_need <= 0)
						{
							flash(0);
						}
						
						float pctg = (float)h2o_consumed/(float)h2o_goal * 60.0f;
						
						// CRITICAL SECTION
						WaitForSingleObject(mutex_lock, INFINITE);
						
						SetConsoleCursorPosition(oh, comp);
						prnt("[");
						for(int j = 0; j < 60; ++j)
						{							
							if(j < (int)pctg)
							{
								SetConsoleTextAttribute(oh, FOREGROUND_BLUE);
								pc((unsigned char)219);
								SetConsoleTextAttribute(oh, dtext);
							}
							else
							{
								pc((unsigned char)176);
							}
						}
						prnt("] ");
						
						sprintf(t, "%d%% %d/%d", (int)(pctg/60.0f * 100.0f), h2o_consumed, h2o_goal);
						prnt(t);
						
						pshots();
						
						ReleaseMutex(mutex_lock);
						// CRITICAL SECTION
					}
					
					//prnt("PRESSSEEEEEED______LMB");
					//SetConsoleCursorPosition(oh, m->dwMousePosition);
					//pc('#');
				}
				else if(m->dwButtonState == 0x2)
				{
					hideCursor();
					ShowWindow(wnd, SW_MINIMIZE);
					hideCursor();
					//SetConsoleCursorPosition(oh, m->dwMousePosition);
					//pc(' ');
					//prnt("PRESSSEEEEEED______RMB");
				}
			}
		}
	}
}

/* char txt[200];
	sprintf(txt, "", );
	MessageBox(wnd, texte, "FUK", MB_OK); */
	
// ADVANCED BLINKING
		/* flash_inf.cbSize = sizeof(FLASHWINFO);
	flash_inf.hwnd = wnd;
	flash_inf.dwFlags = FLASHW_TRAY | FLASHW_TIMER; // Flash ALL
	flash_inf.uCount = 0;
	flash_inf.dwTimeout = 500;
	
	while(1)
	{
		//FlashWindow(wnd, TRUE);
		flash_inf.dwFlags = FLASHW_TRAY | FLASHW_TIMER;
		flash_inf.dwTimeout = 300;
		FlashWindowEx(&flash_inf);
		for(int i = 0; i < 100; ++i)
		{
			flash_inf.dwTimeout += i * 10;
			FlashWindowEx(&flash_inf);
			Sleep(30);
		}
		Sleep(800);
		flash_inf.dwFlags = 0;
		FlashWindowEx(&flash_inf);
		Sleep(200);
		flash_inf.dwFlags = FLASHW_TRAY | FLASHW_TIMER;
		FlashWindowEx(&flash_inf);
		Sleep(200);
		//flash_inf.dwFlags = 0;
		//flash_inf.dwTimeout = 200;
		//FlashWindowEx(&flash_inf);
		//FlashWindow(wnd, TRUE);
		//Sleep(3000);
	} */