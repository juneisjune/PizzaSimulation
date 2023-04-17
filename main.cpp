#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <process.h>
#include <tchar.h>
#include<iostream>
#include <fstream>
using namespace std;

#define NUM_OF_CUSTOMER 190
#define RANGE_MIN 10
#define RANGE_MAX (50 - RANGE_MIN)
#define TABLE_CNT 30

HANDLE hSemaphore[2];
DWORD randTimeArr[210];
CRITICAL_SECTION hCriticalSection;
time_t StartEat, FinishEat;

struct CustomerInform //파일 구현을 위한 구조체
{
	int CustomerId;
	int EnterHour;
	int EnterMin;
	int EatTime;
	int OutHour;
	int OutMin;
};

CustomerInform People[250] = {};
int chair = 30;
int RandomCustomer = rand() % 20;
int CurrentTime = (11 * 3600) + (30 * 60); //11시 30분의 초시간
int FileNumber =0;
void  Enter()
{
	WaitForSingleObject(hSemaphore[1], INFINITE);
	EnterCriticalSection(&hCriticalSection);
	int EnterTime = CurrentTime;
	
	_tprintf(_T("Enter Customer %d~ "), GetCurrentThreadId());
	time(&StartEat);
	
	if (0 < chair && chair<= 30&& EnterTime < 50420)
	{
		cout << "바로 식사 가능" << endl;
	
		ReleaseSemaphore(hSemaphore[0], 1, NULL);
		ReleaseSemaphore(hSemaphore[1], 1, NULL);

	}
	else if(EnterTime > 50420)
	{
		cout << "BREAK 타임 입니다(나가주세여)" << endl;
		ofstream fout;
		fout.open("CustomerInform.txt");
		for (int i = 0; i < NUM_OF_CUSTOMER + RandomCustomer; i++)
		{
			fout << People[i].CustomerId << ":[ ";
			fout << "도착시간:" << People[i].EnterHour << "시 ";
			fout << People[i].EnterMin << "분  ";
			fout << "식사시간:" << People[i].EatTime << "분  ";
			fout << "퇴장시간:" << People[i].OutHour << "시  ";
			fout << People[i].OutMin << "분  ]" << endl;

		}
		fout.close();
		//terminate();
	}
	else 
	{
		cout << "자리가 없으니 잠시 대기" << endl;
		
		ReleaseSemaphore(hSemaphore[0], 1, NULL);
		ReleaseSemaphore(hSemaphore[1], 1, NULL);

	}
	
	LeaveCriticalSection(&hCriticalSection);
}

void TakeMeal(DWORD times)
{
	int number = FileNumber;

	WaitForSingleObject(hSemaphore[0], INFINITE);
	//WaitForSingleObject(hEvent, INFINITE);
	int EnterTime = CurrentTime;
	
	if (0 < chair && chair <= 30)
	{
		chair--;
		time(&FinishEat);
		_tprintf(_T("\n                            Customer %d having launch~ "), GetCurrentThreadId());
		cout << " |의자개수" << chair << endl;
		Sleep(500*times);	// 식사중인 상태를 시뮬레이션 하는 함수.
	}
	
	chair++;
	_tprintf(_T("                                         Out Customer %d~ "), GetCurrentThreadId());
	cout << "(입장시간: " << (int)((EnterTime + ((float)(FinishEat - StartEat) * 1000)) / 3600) << "시";
	cout << (((EnterTime + ((int)(FinishEat - StartEat) * 1000)) % 3600) / 60) << "분, ";
	cout << "식사시간은 " << times << "분 ";
	cout << "퇴장시간: " << (int)((EnterTime + ((float)(FinishEat - StartEat) * 1000)+times*60) / 3600) << "시";
	cout << (((EnterTime + ((int)(FinishEat - StartEat) * 1000)+times*60) % 3600) / 60) << "분)" << endl;

	ReleaseSemaphore(hSemaphore[0], 1, NULL);
	EnterCriticalSection(&hCriticalSection);
	People[number].CustomerId=number;
	People[number].EnterHour= (int)((EnterTime + ((float)(FinishEat - StartEat) * 1000)) / 3600);
	People[number].EnterMin= (((EnterTime + ((int)(FinishEat - StartEat) * 1000)) % 3600) / 60);
	People[number].EatTime=times;
	People[number].OutHour= (int)((EnterTime + ((float)(FinishEat - StartEat) * 1000) + times * 60) / 3600);
	People[number].OutMin= (((EnterTime + ((int)(FinishEat - StartEat) * 1000) + times * 60) % 3600) / 60);
	FileNumber++;
	LeaveCriticalSection(&hCriticalSection);
}



unsigned int WINAPI ThreadProc(LPVOID lpParam)
{
	Enter();
	TakeMeal((DWORD)lpParam);
	
	return 0;
}

int _tmain(int argc, TCHAR* argv[])
{
	time_t pStart, pEnd; //프로그램 만족하는데까지 걸리는 시간 변수
	time(&pStart);
	DWORD dwThreadIDs[210];
	HANDLE hThreads[210];

	
	InitializeCriticalSection(&hCriticalSection);
	srand((unsigned)time(NULL));  	        // random function seed 설정
	int RandomCustomer = rand() % 20;       //190에서 210명의 고객이 방문예정의 설정을 위한 int random

	// 쓰레드에게 전달할 random 값 총 200개 생성.
	for (int i = 0; i < NUM_OF_CUSTOMER+ RandomCustomer; i++)
	{
		randTimeArr[i] = (DWORD)(
			((double)rand() / (double)RAND_MAX) * RANGE_MAX + RANGE_MIN
			);
	}
	// 세마포어 2개 생성. 
	hSemaphore[0] = CreateSemaphore(NULL,TABLE_CNT,TABLE_CNT,NULL); //세마포어[0] 최대값 30, 최솟값 0
	hSemaphore[1] = CreateSemaphore(NULL, 0, 1, NULL);              //세마포어[1] 최대값 1, 최솟값 0

	if (hSemaphore[0] == NULL|| hSemaphore[1] == NULL)         
	{
		_tprintf(_T("CreateSemaphore error: %d\n"), GetLastError());
	}

	
	// Customer를 의미하는 쓰레드 생성.
	for (int i = 0; i < NUM_OF_CUSTOMER+RandomCustomer; i++)
	{
		hThreads[i] = (HANDLE)
			_beginthreadex(
				NULL,
				0,
				ThreadProc,
				(void*)randTimeArr[i],
				CREATE_SUSPENDED,
				(unsigned*)&dwThreadIDs[i]
			);

		if (hThreads[i] == NULL)
		{
			_tprintf(_T("Thread creation fault! \n"));
			return -1;
		}
	}

	ReleaseSemaphore(hSemaphore[1], 1, NULL);
	for (int i = 0; i < NUM_OF_CUSTOMER + RandomCustomer; i++)
	{
		time_t Start, End;
		ResumeThread(hThreads[i]);
		time(&Start);                           //앞 손님이 들어간 순간
		Sleep(rand()%1000);                     //가산점 ㄱ. 손님의 도착시간은 임의적의로 발생.을 위한 Sleep함수
		time(&End);                             //현 손님이 들어온 시간
		CurrentTime += (int)(End - Start)*90;  //현재 시간에 앞 손님이 들어온 시간을 빼서 현재시간으로 설정
	
	}
	

	
	WaitForMultipleObjects(NUM_OF_CUSTOMER + RandomCustomer, hThreads, TRUE, INFINITE);
	time(&pEnd);
	cout << "프로그램 수행 실제 시간은 " << (int)(pEnd - pStart) << "초 입니다." << endl;
	_tprintf(_T("----END-----------\n"));

	for (int i = 0; i < NUM_OF_CUSTOMER + RandomCustomer; i++)
	{
		CloseHandle(hThreads[i]);
	}
	DeleteCriticalSection(&hCriticalSection);
	CloseHandle(hSemaphore[0]);
	CloseHandle(hSemaphore[1]);
	

	return 0;
}

