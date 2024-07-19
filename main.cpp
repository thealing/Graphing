#include <stdio.h>
#include <windows.h>
#include <math.h>

HINSTANCE mainhinstance;
HBITMAP hbm;

FILE *f;
char function[1000];

int length = 0;
int error = 0;

double parse(int start, int end, double x)
{
	int charpos = start;

	double y = 0;
	double a = 0;
	int op = 0;

	double m = 1;

	while (charpos < end)
	{
		if (function[charpos] == ')')
		{
			break;
		}
		if (function[charpos] == ' ')
		{
			charpos++;
			continue;
		}

		if (function[charpos] == 'x' || function[charpos] >= '0' && function[charpos] <= '9' || function[charpos] == '(' || function[charpos] >= 'a' && function[charpos] <= 'z')
		{
			double z;

			if (function[charpos] == '(')
			{
				z = parse(charpos + 1, end, x);

				int c = 1;

				while (c > 0)
				{
					charpos++;

					if (charpos == end)
					{
						error = 1;
						return 0;
					}
					
					if (function[charpos] == '(') c++;
					if (function[charpos] == ')') c--;
				}
			}
			else if (function[charpos] == 'x')
			{
				z = x;
			}
			else if (function[charpos] >= 'a' && function[charpos] <= 'z')
			{
				int namestart = charpos;
				
				while (function[charpos] != '(')
				{
					charpos++;
					
					if (charpos == end)
					{
						error = 1;
						return 0;
					}
				}
				
				z = parse(charpos + 1, end, x);
				
				if (!memcmp("sqrt", function + namestart, charpos - namestart))
				{
					z = sqrt(z);
				}
				else if (!memcmp("sin", function + namestart, charpos - namestart))
				{
					z = sin(z);
				}
				else if (!memcmp("cos", function + namestart, charpos - namestart))
				{
					z = cos(z);
				}
				else if (!memcmp("tan", function + namestart, charpos - namestart))
				{
					z = tan(z);
				}
				else if (!memcmp("abs", function + namestart, charpos - namestart))
				{
					z = abs(z);
				}
				else if (!memcmp("floor", function + namestart, charpos - namestart))
				{
					z = floor(z);
				}
				else if (!memcmp("ceil", function + namestart, charpos - namestart))
				{
					z = ceil(z);
				}
				else if (!memcmp("round", function + namestart, charpos - namestart))
				{
					z = round(z);
				}
				else
				{
					error = 1;
					return 0;
				}
				
				int c = 1;

				while (c > 0)
				{
					charpos++;

					if (charpos == end)
					{
						error = 1;
						return 0;
					}
					
					if (function[charpos] == '(') c++;
					if (function[charpos] == ')') c--;
				}
			}
			else
			{
				int d = 0;
				sscanf(function + charpos, "%lg%n", &z, &d);

				charpos += d - 1;
			}

			if (op == 0) a = z;
			else if (op == 1)
			{
				y += a * m;
				a = z;
				m = 1;
			}
			else if (op == 2)
			{
				y += a * m;
				a = z;
				m = -1;
			}
			else if (op == 3)
			{
				a *= z;
			}
			else if (op == 4)
			{
				a /= z;
			}
			else if (op == 5)
			{
				a = fmod(a, z);
			}
		}
		else if (function[charpos] == '+')
		{
			op = 1;
		}
		else if (function[charpos] == '-')
		{
			op = 2;
		}
		else if (function[charpos] == '*')
		{
			op = 3;
		}
		else if (function[charpos] == '/')
		{
			op = 4;
		}
		else if (function[charpos] == '%')
		{
			op = 5;
		}

		charpos++;
	}

	y += a * m;

	return y;
}

double xtoy(double x)
{
	error = 0;
	
	double y = parse(0, length, x);

	if (error) return 0;
	else return y;
}

void copyrect(int* pd, int* ps, int sw, int dw, int sx, int sy, int ex, int ey, int dx, int dy) 
{
	int ax = 0, ay = 0;
	while (ay < dy)
	{
		if (*(ps + (sy + ay) * sw + (sx + ax)) != 0) *(pd + (ey + ay) * dw + (ex + ax)) = *(ps + (sy + ay) * sw + (sx + ax));
		
		ax++;
		if (ax == dx)
		{
			ax = 0;
			ay++;
		}
	}
}

void drawline(int* p, int dw, int sx, int sy, int ex, int ey, int c)
{
	double ax = 0, ay = 0;
	double incx = ( sx < ex ? sqrt( (double)((ex - sx) * (ex - sx)) / (double)((ex - sx) * (ex - sx) + (ey - sy) * (ey - sy)) ) : 0 - sqrt( (double)((ex - sx) * (ex - sx)) / (double)((ex - sx) * (ex - sx) + (ey - sy) * (ey - sy)) ) );
	double incy = ( sy < ey ? sqrt( (double)((ey - sy) * (ey - sy)) / (double)((ex - sx) * (ex - sx) + (ey - sy) * (ey - sy)) ) : 0 - sqrt( (double)((ey - sy) * (ey - sy)) / (double)((ex - sx) * (ex - sx) + (ey - sy) * (ey - sy)) ) );

	while (fabs(ax) <= fabs(ex - sx) && fabs(ay) <= fabs(ey - sy))
	{
		*(p + (sy + (int)ay) * dw + (sx + (int)ax)) = c;

		ax += incx;
		ay += incy;
	}
}

LRESULT CALLBACK windowcb(HWND mainhwnd, unsigned int message, WPARAM wparam, LPARAM lparam)
{
	static int* asciibmp = (int *)calloc(996*18, 4);
	
	static const HDC mainhdc = GetDC(mainhwnd);
	
	static int wwidth;
	static int wheight;
	
	static int init = 1;
	static int mapwidth = 0, mapheight = 0;
	static double scale = 0;
	
	static double offsetx = 0, offsety = 0;
	int prvMousex = 0, prvMousey = 0;
	
	static tagBITMAPINFO visualmapbmi;
	
	static int* visualmap;
	static int* visualmapWithLayer;
	
	static int showpointercoor = 1;
	
	static int multiplr;
	
	int redraw = 1;
	
	static int touching = 0;
	
	if (message == WM_SIZE)
	{
		wwidth = LOWORD(lparam);
		wheight = HIWORD(lparam);
	}
	else if (message == WM_COMMAND)
	{
		;
	}
	else if (message == WM_CLOSE)
	{
		exit(0);
	}
	else if (message == WM_PAINT)
	{
		redraw = 1;
	}
	else if (message == WM_TIMER)
	{
		char newfunction[1000];
		
		f = fopen("function.txt", "r");
		int newlength = fread(newfunction, 1, sizeof(newfunction) - 1, f);
		fclose(f);
		
		if (newlength == length && !memcmp(function, newfunction, length)) redraw = 0;
		else
		{
			length = newlength;
			memcpy(function, newfunction, newlength);
		}
	}
	else if (message == WM_KEYDOWN)
	{
		if (wparam == 'R') 
		{
			scale = 50.0;
			offsetx = 0;
			offsety = 0;
			
			redraw = 1;
		}
	}
	else if (message == WM_MOUSEMOVE)
	{
		redraw = 1;
	}
	else if (message == WM_LBUTTONDOWN)
	{
		offsetx += 0.000001;
		
		prvMousex = LOWORD(lparam);
		prvMousey = HIWORD(lparam);	
		
		redraw = 1;
		
		touching = 1;
	}
	else if (message == WM_MOUSEWHEEL)
	{
		offsetx = (double)offsetx / (double)scale;
		offsety = (double)offsety / (double)scale;
		
		if ((signed short)HIWORD(wparam) > 0) 
		{
			scale = scale * 1.3;
		}
		else if ((signed short)HIWORD(wparam) < 0) 
		{
			scale = scale / 1.3;
		}
		
		offsetx = offsetx * (double)scale;
		offsety = offsety * (double)scale;
		
		redraw = 1;
	}
	else if (init) 
	{
		memset(function, 0, sizeof(function));

		f = fopen("function.txt", "r");
		length = fread(function, 1, sizeof(function) - 1, f);
		fclose(f);
		
		RECT recct;
		GetClientRect(mainhwnd, &recct);
		
		wwidth = recct.right;
		wheight = recct.bottom;
		
		init = 0;
		mapwidth = 600, mapheight = 300;
		scale = 80.0;
		
		visualmap = (int *)calloc(mapwidth*mapheight * 3, 4);
		visualmap += mapwidth*mapheight;
		visualmapWithLayer = (int *)calloc(mapwidth*mapheight, 4);
	
		visualmapbmi.bmiHeader.biSize = sizeof(visualmapbmi.bmiHeader);
		visualmapbmi.bmiHeader.biWidth = mapwidth;
		visualmapbmi.bmiHeader.biHeight = mapheight;
		visualmapbmi.bmiHeader.biPlanes = 1;
		visualmapbmi.bmiHeader.biBitCount = 32;
		visualmapbmi.bmiHeader.biCompression = BI_RGB;
		
		hbm = LoadBitmap(mainhinstance, "ascii");
		GetBitmapBits(hbm, 996*18 * 4, asciibmp);
	}
	
	if (redraw == 1 && offsetx == 0 && offsety == 0)
	{
		ZeroMemory(visualmap, mapwidth*mapheight * 4);
		
		double dresolution = (double)mapwidth / 2.0 / scale;
		
		multiplr = log10(dresolution) >= 0 ? (int)log10(dresolution) : (int)log10(dresolution) - 1;
		
		if (dresolution >= 1) while (dresolution >= 10) dresolution /= 10;
		else while (dresolution < 1) dresolution *= 10;
		
		int resolution = (int)((double)mapwidth / 2.0 / dresolution);
		
		drawline(visualmap, mapwidth, 0, mapheight / 2, mapwidth, mapheight / 2, 0xFF0000);
		
		int asdx = 0;
		while (1) 
		{
			while ((asdx - mapwidth / 2) % resolution != 0) asdx++;
			
			if (asdx > mapwidth - 15) break;
		
			drawline(visualmap, mapwidth, asdx, (int)((double)mapheight * 0.45), asdx, (int)((double)mapheight * 0.55), 0xFF0000);
				
			int n2d = (asdx - mapwidth / 2) / resolution;
			int NextNumberPos = asdx;
			
			if (n2d < 0) 
			{
				if (n2d % 2 == 0)
				{
					copyrect(visualmap, asciibmp, 996, mapwidth, 60, 0, NextNumberPos, mapheight / 2 - 18, 12, 18);
					NextNumberPos += 12;
				}
				
				n2d = 0 - n2d;
			}
			
			while (n2d > 9) n2d /= 10;
			
			if (n2d % 2 == 0)
			{
				copyrect(visualmap, asciibmp, 996, mapwidth, 96 + n2d * 12, 0, NextNumberPos, mapheight / 2 - 18, 12, 18);
			}
			
			asdx++;
		}
		
		drawline(visualmap, mapwidth, mapwidth / 2, 0, mapwidth / 2, mapheight, 0xFF0000);
		
		
		int asdy = 15;
		while (1) 
		{
			while ((asdy - mapheight / 2) % resolution != 0) asdy++;
			
			if (asdy > mapheight) break;
		
			drawline(visualmap, mapwidth, (int)((double)mapwidth * 0.45), asdy, (int)((double)mapwidth * 0.55), asdy, 0xFF0000);
				
			int n2d = (mapheight / 2 - asdy) / resolution;
			int NextNumberPos = mapwidth / 2;
			
			if (n2d < 0) 
			{
				if (n2d % 2 == 0)
				{
					copyrect(visualmap, asciibmp, 996, mapwidth, 60, 0, NextNumberPos, asdy - 18, 12, 18);
					NextNumberPos += 12;
				}
				
				n2d = 0 - n2d;
			}
			
			while (n2d > 9) n2d /= 10;
			
			if (n2d % 2 == 0)
			{
				copyrect(visualmap, asciibmp, 996, mapwidth, 96 + n2d * 12, 0, NextNumberPos, asdy - 18, 12, 18);
			}
			
			asdy++;
		}
		
		double x = 0 - (double)mapwidth / 2.0 / scale;
		while (x < (double)mapwidth / 2.0 / scale)
		{
			int sx = (int)(mapwidth / 2 + x * scale), sy = (int)(mapheight / 2 - ( xtoy(x) ) * scale);
			x += 1.0 / scale;
			int ex = (int)(mapwidth / 2 + x * scale), ey = (int)(mapheight / 2 - ( xtoy(x) ) * scale);
			
			if (sy > 0	&& sy < mapheight && ey > 0 && ey < mapheight) ;
			else if (sy > 0	&& sy < mapheight) 
			{
				if (ey < 0) ey = 0;
				else if (ey > mapheight) ey = mapheight;
			}
			else if (ey > 0	&& ey < mapheight) 
			{
				if (sy < 0) sy = 0;
				else if (sy > mapheight) sy = mapheight;
			}
			else if (sy < 0 && ey > mapheight)
			{
				sy = 0;
				ey = mapheight;
			}
			else if (ey < 0 && sy > mapheight)
			{
				ey = 0;
				sy = mapheight;
			}
			else goto skip1;
			
			drawline(visualmap, mapwidth, sx, sy, ex, ey, 0x0000FF);
			skip1:;
		}
		
		CopyMemory(visualmapWithLayer, visualmap, mapwidth*mapheight * 4);
		
		if (multiplr != 0) 
		{
			int ScaleIndBmp[(32*12)*18];
			char ScaleIndText[32];
			
			POINT mp;
			GetCursorPos(&mp);
			ScreenToClient(mainhwnd, &mp);
			
			sprintf(ScaleIndText, "%d\0", multiplr);
			
			ZeroMemory(ScaleIndBmp, (32*12)*18 * 4);
			
			int NextCharPos = 0;
			while (ScaleIndText[NextCharPos] != 0)
			{
				if (ScaleIndText[NextCharPos] > 40) copyrect(ScaleIndBmp, asciibmp, 996, 32*12, (ScaleIndText[NextCharPos] - 40) * 12, 0, NextCharPos * 12, 0, 12, 18);
				
				NextCharPos++;
			}
			
			copyrect(visualmapWithLayer, ScaleIndBmp, 32*12, mapwidth, 0, 0, 5, 5, 32*12, 18);
		}
	}
	else if (redraw == 1)
	{
		redrawOC:;
		
		ZeroMemory(visualmap, mapwidth*mapheight * 4);
		
		double dresolution = (double)mapwidth / 2.0 / scale;
		
		if (fabs(offsety) <= mapheight / 2) drawline(visualmap, mapwidth, 0, mapheight / 2 + offsety, mapwidth, mapheight / 2 + offsety, 0xFF0000);
		if (fabs(offsetx) <= mapwidth / 2) drawline(visualmap, mapwidth, mapwidth / 2 + offsetx, 0, mapwidth / 2 + offsetx, mapheight, 0xFF0000);
	
		int multiplr = log10(dresolution) >= 0 ? (int)log10(dresolution) : (int)log10(dresolution) - 1;
		
		double x = 0 - ((double)mapwidth / 2.0 + offsetx) / scale;
		while (x < ((double)mapwidth / 2.0 - offsetx) / scale)
		{
			int sx = (mapwidth / 2 + (int)offsetx) + (int)(x * scale), sy = (mapheight / 2 + (int)offsety) - (int)(( xtoy(x) ) * scale);
			x += 1.0 / scale;
			int ex = (mapwidth / 2 + (int)offsetx) + (int)(x * scale), ey = (mapheight / 2 + (int)offsety) - (int)(( xtoy(x) ) * scale);
			
			if (sy > 0	&& sy < mapheight && ey > 0 && ey < mapheight) ;
			else if (sy > 0	&& sy < mapheight) 
			{
				if (ey < 0) ey = 0;
				else if (ey > mapheight) ey = mapheight;
			}
			else if (ey > 0	&& ey < mapheight) 
			{
				if (sy < 0) sy = 0;
				else if (sy > mapheight) sy = mapheight;
			}
			else if (sy < 0 && ey > mapheight)
			{
				sy = 0;
				ey = mapheight;
			}
			else if (ey < 0 && sy > mapheight)
			{
				ey = 0;
				sy = mapheight;
			}
			else goto skip2;
			
			drawline(visualmap, mapwidth, sx, sy, ex, ey, 0x0000FF);
			
			skip2:;
		}
		
		CopyMemory(visualmapWithLayer, visualmap, mapwidth*mapheight * 4);
		
		StretchDIBits(mainhdc, 0, wheight, wwidth, -wheight, 0, 0, mapwidth, mapheight, visualmapWithLayer, &visualmapbmi, DIB_RGB_COLORS, SRCCOPY);
	}
		
	CopyMemory(visualmapWithLayer, visualmap, mapwidth*mapheight * 4);
		
	int ScaleIndBmp[(32*12)*18];
	char ScaleIndText[32];
		
	POINT mp;
	GetCursorPos(&mp);
	ScreenToClient(mainhwnd, &mp);
	
	sprintf(ScaleIndText, "x %.3g y %.3g\0", ((double)(mp.x - wwidth / 2) * (double)mapwidth / (double)wwidth - (double)offsetx) / scale	, ((double)(wheight / 2 - mp.y) * (double)mapheight / (double)wheight + (double)offsety) / scale);
	
	ZeroMemory(ScaleIndBmp, (32*12)*18 * 4);
	
	int NextCharPos = 0;
	while (ScaleIndText[NextCharPos] != 0)
	{
		if (ScaleIndText[NextCharPos] > 40) copyrect(ScaleIndBmp, asciibmp, 996, 32*12, (ScaleIndText[NextCharPos] - 40) * 12, 0, NextCharPos * 12, 0, 12, 18);
		
		NextCharPos++;
	}
	
	copyrect(visualmapWithLayer, ScaleIndBmp, 32*12, mapwidth, 0, 0, 5, 5, 32*12, 18);
	
	StretchDIBits(mainhdc, 0, wheight, wwidth, -wheight, 0, 0, mapwidth, mapheight, visualmapWithLayer, &visualmapbmi, DIB_RGB_COLORS, SRCCOPY);

	if (GetAsyncKeyState(VK_LBUTTON) == 0) touching = 0;

	while (touching)
	{
		if (mp.x != prvMousex || mp.y != prvMousey)
		{
			offsetx += (double)(mp.x - prvMousex) * (double)mapwidth / (double)wwidth;
			offsety += (double)(mp.y - prvMousey) * (double)mapheight / (double)wheight;
			
			prvMousex = mp.x;
			prvMousey = mp.y;
			redraw = 1;
			goto redrawOC;
		}
		
		GetCursorPos(&mp);
		ScreenToClient(mainhwnd, &mp);
	}
	
	return DefWindowProc(mainhwnd, message, wparam, lparam);
}

int WINAPI WinMain(HINSTANCE hinstance1, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{	
	mainhinstance = hinstance1;
	
	HWND mainhwnd = CreateWindow("STATIC", "Graphing", 0, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, 0, 0, 0, 0);
	
	SetWindowLongPtr(mainhwnd, GWLP_WNDPROC, (LONG_PTR)windowcb);
	SetWindowLongPtr(mainhwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
	UpdateWindow(mainhwnd);
	
	SetActiveWindow(mainhwnd);
	
	SetTimer(mainhwnd, 1, 500, 0);
	
	MSG msg;
	while(GetMessage(&msg, 0, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}
