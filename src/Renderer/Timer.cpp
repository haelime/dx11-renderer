#include "Renderer/Timer.h"

Timer::Timer()
{
	LARGE_INTEGER liNow;
	QueryPerformanceFrequency(&liNow);
	m_llFrequency = liNow.QuadPart;
	m_dTotalTime = 0;
	m_fDeltaTime = 0;
	Reset();
}

Timer::~Timer() {}

void Timer::Reset()
{
	LARGE_INTEGER liNow;
	QueryPerformanceCounter(&liNow);

	m_llStartTick = liNow.QuadPart;
	m_llPrevTick = liNow.QuadPart;
}

void Timer::Tick()
{
	LARGE_INTEGER liNow;
	QueryPerformanceCounter(&liNow);

	m_dTotalTime = (liNow.QuadPart - m_llStartTick) / static_cast<double>(m_llFrequency);
	m_fDeltaTime = (liNow.QuadPart - m_llPrevTick) / static_cast<float>(m_llFrequency);
	m_llPrevTick = liNow.QuadPart;
}

double Timer::TotalTime() const
{
	return m_dTotalTime;
}

float Timer::DeltaTime() const
{
	return m_fDeltaTime > 0.1f ? 0.1f : m_fDeltaTime;
}