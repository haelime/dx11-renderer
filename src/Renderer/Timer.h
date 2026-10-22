#pragma once

#include <windows.h>

class Timer
{
public:
	Timer();
	~Timer();
	Timer(const Timer&) = delete;
	Timer& operator=(const Timer&) = delete;

	void Reset();
	void Tick();
	double TotalTime() const;
	float DeltaTime() const;

private:
	LONGLONG m_llStartTick;
	LONGLONG m_llPrevTick;
	LONGLONG m_llFrequency;

	double m_dTotalTime;
	float m_fDeltaTime;
};