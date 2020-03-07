#include <windows.h>
#include <process.h>
#include <time.h>
#include "resource.h"

#define _BLOCK_T 0
#define _BLOCK_I 1
#define _BLOCK_Z 2
#define _BLOCK_L 3
#define _BLOCK_O 4
#define _BLOCK_RL 5
#define _BLOCK_RZ 6

#define ID_TIMER 1

#define WM_CREATEBLOCK (WM_USER+1)
#define WM_MOVEBLOCK (WM_USER+2)
#define WM_KILLBLOCK (WM_USER+3)
#define WM_GAMEOVER (WM_USER+4)

#define _PAINT_PAINT 0
#define _PAINT_ERASE 1

//��ɫ�ṹ��
typedef struct Color
{
	BOOL isWhite;
	BOOL isBlocked;
	int red;
	int green;
	int blue;
}Color;
//��׹ʱ��ķ���ṹ��
typedef struct FallBlock
{
	int id;
	int  position[4][2];
	Color color;
}FallBlock;

LRESULT CALLBACK WndProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam);
//��û�з���ʱ������һ������
FallBlock* CreateBlock(FallBlock* block, HWND hwnd);
//��ת����
FallBlock* SwapBlock(FallBlock* block);
//�ж��ܷ������ƶ�
BOOL JudgeBorder(FallBlock* block,int position);
//�ж��Ƿ񵽵�
BOOL JudgeBottom(FallBlock* block);
//�ж��ܷ���ת
BOOL JudgeSwap(FallBlock* block);
//������Ⱦ������ɫ
void PaintMap(FallBlock* block,int action);
//��������
void DestroyMap();
//����һ��
void RowClear();

int cxClient, cyClient, cxBlock, cyBlock,cxCount,cyCount;
int delta;
int cxMove, cyMove;  //����������ת
int cxNextMove = 7,cyNextMove = 8;   //������������һ������
unsigned int score = 0;   //������Ȼ��Ҫ�з���
TCHAR szBuffer[8];
const TCHAR szText[] = TEXT("Score:");
//������Ϸ�����������ʾ
Color* arrGame[10][20];

const int blockPoint[7][4][2] = {
	{{-1,0},{0,0},{1,0},{0,1}},		//T
	{{0,-1},{0,0},{0,1},{0,2}},		//I
	{{-1,0},{0,0},{0,1},{1,1}},		//Z
	{{0,0},{1,0},{-1,0},{-1,1}},	//L
	{{0,0},{1,0},{1,1},{0,1}},	//O
	{{-1,0},{0,0},{1,0},{1,1}},		//RL
	{{-1,1},{0,1},{0,0},{1,0}}		//RZ
};

void RowClear()
{
	BOOL beClear;
	for (int y = cyCount - 1; y > 0; y--) {
		beClear = TRUE;
		for (int x = 0; x < cxCount; x++) {
			if (!arrGame[x][y]->isBlocked) {
				beClear = FALSE;
				break;
			}
		}

		if (beClear) {
			for (int x = 0; x < cxCount; x++) {
				for (int ty = y; ty > 0; ty--) {
					arrGame[x][ty]->blue = arrGame[x][ty - 1]->blue;
					arrGame[x][ty]->green = arrGame[x][ty - 1]->green;
					arrGame[x][ty]->red = arrGame[x][ty - 1]->red;
					arrGame[x][ty]->isWhite = arrGame[x][ty - 1]->isWhite;
					arrGame[x][ty]->isBlocked = arrGame[x][ty - 1]->isBlocked;
				}
			}
			y++;
			//����ϵĴ������
			score += 100;
			if (score % 2000 == 0) {
				MessageBox(NULL, TEXT("...��Ȼ�ܸ�л�����㲻Ӧ�ð�ʱ���˷����������"), TEXT("THX"), MB_OK);
			}
		}
	}
	return beClear;
}

void PaintMap(FallBlock* block,int action)
{
	if (action == _PAINT_PAINT) {
		for (int i = 0; i < 4; i++) {
			arrGame[block->position[i][0]][block->position[i][1]]->blue = block->color.blue;
			arrGame[block->position[i][0]][block->position[i][1]]->green = block->color.green;
			arrGame[block->position[i][0]][block->position[i][1]]->red = block->color.red;
			arrGame[block->position[i][0]][block->position[i][1]]->isWhite = FALSE;
		}
	}
	else if (action == _PAINT_ERASE){
		for (int i = 0; i < 4; i++) {
			arrGame[block->position[i][0]][block->position[i][1]]->isWhite = TRUE;
		}
	}
}

BOOL JudgeBorder(FallBlock* block, int position)
{
	for (int i = 0; i < 4; i++) {
		if (position == VK_LEFT) {
			if ((block->position[i][0] - 1 < 0) ||   //�������ᳬ���߽�
				arrGame[block->position[i][0] - 1][block->position[i][1]]->isBlocked) {  //���߻�����ש��
				return FALSE;
			}
		}
		if (position == VK_RIGHT ){
			if ((block->position[i][0] + 1 >= cxCount) ||   //������һᳬ���߽�
				arrGame[block->position[i][0] + 1][block->position[i][1]]->isBlocked) {  //���߻�����ש��
				return FALSE;
			}
		}
	}
	return TRUE;
}

BOOL JudgeBottom(FallBlock* block)
{
	for (int i = 0; i < 4; i++) {
		if ((block->position[i][1] + 1 >= cyCount) ||
			arrGame[block->position[i][0]][block->position[i][1]+ 1]->isBlocked) {
			return FALSE;
		}
	}
	return TRUE;
}

BOOL JudgeSwap(FallBlock* block)
{
	FallBlock tempBlock;
	//����ʱ���鸳��ֵ
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 2; j++) {
			tempBlock.position[i][j] = block->position[i][j];
		}
	}

	SwapBlock(&tempBlock); //��ת��ʱ����

	for (int i = 0; i < 4; i++) {
		if (tempBlock.position[i][0] < 0 ||   //����������
			tempBlock.position[i][0] >= cxCount ||  //���������ұ�
			tempBlock.position[i][1] >= cyCount||  //���������ײ�
			arrGame[tempBlock.position[i][0]][tempBlock.position[i][1]]->isBlocked) {
			return FALSE;
		}
	}
	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam) 
{
	static RECT rectPaint, rectMenu;
	static FallBlock* block = NULL, *nextBlock = NULL;
	BOOL isGameOver = FALSE;

	switch (umsg)
	{
	case WM_CREATE:
	{
		//��ȡ�����ͻ���
		RECT rect;
		GetClientRect(hwnd, &rect);
		cxClient = rect.right;
		cyClient = rect.bottom;

		//��ȡ��Ϸ��
		rectPaint.top = rect.top;
		rectPaint.bottom = rect.bottom;
		rectPaint.left = rect.left;
		rectPaint.right = rect.right * 2 / 3;

		//��ȡ��������Ĵ�С
		cxBlock = rectPaint.right / 10;
		cyBlock = cxBlock;

		//�Է���Ϊ��λ����ȡ��Ϸ���Ĵ�С
		cxCount = (rectPaint.right - rectPaint.left) / cxBlock;
		cyCount = (rectPaint.bottom - rectPaint.top) / cyBlock;

		//ȷ�����Ϊ����
		rectPaint.right = rectPaint.left + cxBlock * 10;
		delta = rectPaint.bottom - rectPaint.top - (cyBlock * cyCount);
		
		//�Ӷ���ȡ�Ʒ���
		rectMenu.left = rectPaint.right + 1;
		rectMenu.top = rectPaint.top;
		rectMenu.right = rect.right;
		rectMenu.bottom = rectPaint.bottom;

		SetTimer(hwnd, ID_TIMER, 1000, NULL);

		SendMessage(hwnd, WM_CREATEBLOCK, 0, 0);

		return 0;
	}
	case WM_TIMER:
	{
		if (block == NULL) {   //���û�з���
			SendMessage(hwnd, WM_CREATEBLOCK, 0, 0);
		}
		else if (JudgeBottom(block)) {   //����з��飬�ҷ�������½�
			SendMessage(hwnd, WM_MOVEBLOCK, 0, 0);
		}
		else {     //����з��飬�ҷ��鲻���½�
			SendMessage(hwnd, WM_KILLBLOCK, 0, 0);
		}
		return 0;
	}
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case ID_40001:
		{
			MessageBox(hwnd, TEXT("����˹������淨,���������аٶ�"), TEXT("���ܵ���Ϸ˵��"), MB_OK);
			break;
		}
		case ID_40002:
		{
			MessageBox(hwnd, TEXT("�¶��˾͸�����"), TEXT("ƭ���"), MB_OK);
			break;
		}
		case ID_40003:
		{
			MessageBox(hwnd, TEXT("��û��ʵװ���ֹ��ܣ�������Ҳ����ΪʲôҪд�˵�"), TEXT("��Ǹ"), MB_OK);
			break;
		}
		case ID_40004:
		{
			MessageBox(hwnd, TEXT("���������ʵװŶ"), TEXT("��Ȼûʲô����"), MB_OK);
			SendMessage(hwnd, WM_GAMEOVER, 0, 0);
			break;
		}
		}
		return 0;
	}
	case WM_CREATEBLOCK:
	{
		//���û�з��飬�ʹ���һ������
		if (block == NULL) {
			if (nextBlock == NULL) {
				nextBlock = CreateBlock(nextBlock, hwnd);
				block = CreateBlock(block, hwnd);
			}
			else
			{
				block = nextBlock;
				nextBlock = CreateBlock(nextBlock, hwnd);
			}
		}
		PaintMap(block, _PAINT_PAINT);

		InvalidateRect(hwnd, NULL, FALSE);

		return 0;
	}
	case WM_MOVEBLOCK:
	{
		PaintMap(block, _PAINT_ERASE);
		for (int i = 0; i < 4; i++) {
			block->position[i][1]++;
		}
		cyMove++;
		PaintMap(block, _PAINT_PAINT);

		InvalidateRect(hwnd, NULL, FALSE);

		return 0;
	}
	case WM_KILLBLOCK:
	{
		for (int i = 0; i < 4; i++) {
			arrGame[block->position[i][0]][block->position[i][1]]->isBlocked = TRUE;
		}
		//�ж���Ϸ�Ƿ����
		for (int i = 0; i < cxCount; i++) {
			if (arrGame[i][1]->isBlocked) {
				isGameOver = TRUE;
				break;
			}
		}

		free(block);
		block = NULL;

		if (isGameOver) {
			SendMessage(hwnd, WM_GAMEOVER, 0, 0);
		}
		else {
			RowClear();
		}

		InvalidateRect(hwnd, NULL, FALSE);

		return 0;
	}
	case WM_GAMEOVER:
	{
		KillTimer(hwnd, ID_TIMER);
		if (block != NULL) {
			free(block);
			block = NULL;
		}
		if (nextBlock != NULL) {
			free(nextBlock);
			nextBlock = NULL;
		}

		for (int i = 0; i < cxCount; i++) {
			for (int j = 0; j < cyCount; j++) {
				arrGame[i][j]->isWhite = TRUE;
				arrGame[i][j]->isBlocked = FALSE;
			}
		}

		MessageBox(NULL, TEXT("��Ϸ����"), TEXT("GAME OVER"), MB_ICONERROR);

		SendMessage(hwnd, WM_DESTROY, 0, 0);
		return 0;
	}
	case WM_DESTROY:
	{
		DestroyMap();

		PostQuitMessage(0);

		return 0;
	}
	case WM_KEYDOWN:
	{
		switch (wParam) {
		case VK_RIGHT:
		{
			if (block != NULL && JudgeBorder(block, wParam)) {
				PaintMap(block, _PAINT_ERASE);
				for (int i = 0; i < 4; i++) {
					block->position[i][0]++;
				}
				cxMove++;
				PaintMap(block, _PAINT_PAINT);
			}
			break;
		}
		case VK_LEFT:
		{
			if (block != NULL && JudgeBorder(block, wParam)) {
				PaintMap(block, _PAINT_ERASE);
				for (int i = 0; i < 4; i++) {
					block->position[i][0]--;
				}
				cxMove--;
				PaintMap(block, _PAINT_PAINT);
			}
			break;
		}
		case VK_DOWN:
		{
			if (block != NULL && JudgeBottom(block)) {
				PaintMap(block, _PAINT_ERASE);
				for (int i = 0; i < 4; i++) {
					block->position[i][1]++;
				}
				cyMove++;
				PaintMap(block, _PAINT_PAINT);
			}
			break;
		}
		case VK_UP:
		{
			if (block != NULL && JudgeSwap(block)) {
				if (block->id == _BLOCK_O) {
					break;
				}
				PaintMap(block, _PAINT_ERASE);
				SwapBlock(block);
				PaintMap(block, _PAINT_PAINT);
			}
			break;
		}
		}
		InvalidateRect(hwnd, NULL, TRUE);

		return 0;
	}
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdcPaint = BeginPaint(hwnd, &ps);

		SetBkMode(hdcPaint, TRANSPARENT);
		SelectObject(hdcPaint, GetStockObject(DKGRAY_BRUSH));
		SelectObject(hdcPaint, GetStockObject(NULL_PEN));
		//���Ҳ�ķ�����
		Rectangle(hdcPaint, rectMenu.left, rectMenu.top, rectMenu.right, rectMenu.bottom);
		TextOut(hdcPaint, rectMenu.left, rectMenu.top, szText, wcslen(szText));
		wsprintf(szBuffer, TEXT("%08d"), score);
		TextOut(hdcPaint, rectMenu.left, rectMenu.top + 30, szBuffer, wcslen(szBuffer));

		HBRUSH hBrush = GetStockObject(WHITE_BRUSH);

		SelectObject(hdcPaint, hBrush);
		SelectObject(hdcPaint, GetStockObject(BLACK_PEN));
		Rectangle(hdcPaint, rectPaint.left, rectPaint.top, rectPaint.right, rectPaint.bottom);

		//������
		for (int i = 0; i < cxCount; i++) {
			MoveToEx(hdcPaint, rectPaint.left + i * cxBlock, rectPaint.top, NULL);
			LineTo(hdcPaint, rectPaint.left + i * cxBlock, rectPaint.bottom);
		}
		//������
		for (int i = 0; i <= cyCount; i++) {
			MoveToEx(hdcPaint, rectPaint.left, rectPaint.bottom - i * cyBlock, NULL);
			LineTo(hdcPaint, rectPaint.right, rectPaint.bottom - i * cyBlock);
		}
		SelectObject(hdcPaint, GetStockObject(NULL_PEN));
		//���������ɫ
		for (int i = 0; i < cxCount; i++) {
			for (int j = 0; j < cyCount; j++) {
				if (!arrGame[i][j]->isWhite) {
					hBrush = CreateSolidBrush(RGB(arrGame[i][j]->red, arrGame[i][j]->green, arrGame[i][j]->blue));
					SelectObject(hdcPaint, hBrush);
					Rectangle(hdcPaint, i*cxBlock, j*cyBlock + delta, (i + 1)*cxBlock, (j + 1)*cyBlock + delta);
					DeleteObject(hBrush);
					hBrush = GetStockObject(WHITE_BRUSH);
				}
			}
		}

		//����ɫ��
		SelectObject(hdcPaint, GetStockObject(WHITE_BRUSH));
		SelectObject(hdcPaint, GetStockObject(NULL_PEN));
		Rectangle(hdcPaint,
			(cxNextMove +4)*cxBlock-15,
			(cyNextMove)*cyBlock-10,
			(cxNextMove +7)*cyBlock+20,
			(cyNextMove +4)*cyBlock)+20;

		hBrush = CreateSolidBrush(RGB(nextBlock->color.red, nextBlock->color.green, nextBlock->color.blue));
		SelectObject(hdcPaint, hBrush);
		SelectObject(hdcPaint, GetStockObject(NULL_PEN));

		//����һ������
		for (int i = 0; i < 4; i++) {
			Rectangle(hdcPaint,
				(cxNextMove + nextBlock->position[i][0])*cxBlock + 5,
				(cyNextMove + nextBlock->position[i][1])*cyBlock,
				(cxNextMove + nextBlock->position[i][0] + 1)*cxBlock + 5,
				(cyNextMove + nextBlock->position[i][1] + 1)*cyBlock);
		}

		DeleteObject(hBrush);
		hBrush = GetStockObject(WHITE_BRUSH);

		EndPaint(hwnd, &ps);

		return 0;
	}
	default:
	{
		return DefWindowProc(hwnd, umsg, wParam, lParam);
	}
	}
}


FallBlock* CreateBlock(FallBlock* block, HWND hwnd)
{
	static BOOL fExist[7];
	static int blockCount = 0;

	block = malloc(sizeof(FallBlock));

	//����һ��
	if (blockCount % 14 == 0) {
		memset(fExist, 0, sizeof(BOOL) * 7);
	}
	int iThis = rand() % 7;
	while (fExist[iThis] >= 2) {
		iThis = rand() % 7;
	}
	fExist[iThis]++;
	blockCount++;
	if (blockCount > 60000 && blockCount % 14 == 0) {
		blockCount = 0;
	}

	//ȷ������ĳ�ʼλ��
	cxMove = cxCount / 2;
	cyMove = 1;
	//�趨�������ɫ���ǲ�ɫ����
	block->id = iThis;
	block->color.blue = rand()%256;
	block->color.green = rand()%256;
	block->color.red = rand()%256;

	for (int i = 0; i < 4; i++) {
		block->position[i][0] = blockPoint[iThis][i][0] + cxMove;
		block->position[i][1] = blockPoint[iThis][i][1] + cyMove;
	}

	return block;
}

FallBlock* SwapBlock(FallBlock* block)
{
	int tempArray[4][2];
	for (int i = 0; i < 4; i++) {
		block->position[i][0] -= cxMove;
		block->position[i][1] -= cyMove;
	}
	for (int i = 0; i < 4; i++) {
		tempArray[i][0] = block->position[i][1] * (-1);
		tempArray[i][1] = block->position[i][0] * 1;
	}
	for (int i = 0; i < 4; i++) {
		block->position[i][0] = tempArray[i][0] + cxMove;
		block->position[i][1] = tempArray[i][1] + cyMove;
	}

	return block;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInstance, LPSTR szCmdLine, int iCmdShow)
{
	srand(time(0));
	wsprintf(szBuffer, TEXT("%08d"), score);

	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 20; j++) {
			arrGame[i][j] = malloc(sizeof(Color));
			arrGame[i][j]->isWhite = TRUE;
			arrGame[i][j]->isBlocked = FALSE;
		}
	}

	TCHAR szAppName[] = TEXT("����˹����");
	WNDCLASS wndClass;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hbrBackground = (HBRUSH)GetStockObject(DKGRAY_BRUSH);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hIcon = LoadIcon(hInstance, IDI_ICON);
	wndClass.hInstance = hInstance;
	wndClass.lpfnWndProc = WndProc;
	wndClass.lpszClassName = szAppName;
	wndClass.lpszMenuName = MAKEINTRESOURCE(IDM_MENU);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;

	if (!RegisterClass(&wndClass)) {
		MessageBox(NULL, TEXT("��ʼ��ʧ����"), TEXT("�Բ���"), MB_ICONERROR);
		return 0;
	}

	HWND hwnd = CreateWindow(szAppName, TEXT("����˹����"), WS_OVERLAPPEDWINDOW^WS_THICKFRAME,
		CW_USEDEFAULT, CW_USEDEFAULT, 640, 720,
		NULL, NULL, hInstance, NULL);

	ShowWindow(hwnd, iCmdShow);
	UpdateWindow(hwnd);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

void DestroyMap()
{
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 20; j++) {
			free(arrGame[i][j]);
			arrGame[i][j] = NULL;
		}
	}
}