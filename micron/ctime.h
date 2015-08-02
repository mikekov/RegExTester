#pragma once
#include "basetypes.h"

namespace mcr {


class CTimeSpan
{
public:
	typedef __time64_t time_span_t;

	CTimeSpan() throw();
	CTimeSpan(time_span_t time) throw();
	CTimeSpan(LONG days, int hours, int mins, int secs) throw();

	LONGLONG GetDays() const throw();
	LONGLONG GetTotalHours() const throw();
	LONG GetHours() const throw();
	LONGLONG GetTotalMinutes() const throw();
	LONG GetMinutes() const throw();
	LONGLONG GetTotalSeconds() const throw();
	LONG GetSeconds() const throw();

	time_span_t GetTimeSpan() const throw();

	CTimeSpan operator + (CTimeSpan span) const throw();
	CTimeSpan operator - (CTimeSpan span) const throw();
	CTimeSpan& operator += (CTimeSpan span) throw();
	CTimeSpan& operator -= (CTimeSpan span) throw();
	bool operator == (CTimeSpan span) const throw();
	bool operator != (CTimeSpan span) const throw();
	bool operator < (CTimeSpan span) const throw();
	bool operator > (CTimeSpan span) const throw();
	bool operator <= (CTimeSpan span) const throw();
	bool operator >= (CTimeSpan span) const throw();

	CString Format(const TCHAR* format) const;
	CString Format(UINT id) const;

private:
	time_span_t time_span_;
};


class CTime
{
public:
	typedef __time64_t time_base_t;

	CTime() throw();
	CTime(time_base_t time) throw();
	CTime(int year, int month, int day, int hour, int min, int sec, int daylight_saving_time= -1);
	CTime(WORD dos_date, WORD dos_time, int daylight_saving_time= -1);
	CTime(const SYSTEMTIME& st, int daylight_saving_time= -1);
	CTime(const FILETIME& ft, int daylight_saving_time= -1);

	CTime& operator = (time_base_t time) throw();

	CTime& operator += (CTimeSpan span) throw();
	CTime& operator -= (CTimeSpan span) throw();

	CTimeSpan operator - (CTime time) const throw();
	CTime operator - (CTimeSpan span) const throw();
	CTime operator + (CTimeSpan span) const throw();

	bool operator == (CTime time) const throw();
	bool operator != (CTime time) const throw();
	bool operator < (CTime time) const throw();
	bool operator > (CTime time) const throw();
	bool operator <= (CTime time) const throw();
	bool operator >= (CTime time) const throw();

	tm* GetGmtTm(tm* ptm) const;
	tm* GetLocalTm(tm* ptm) const;

	bool GetAsSystemTime(SYSTEMTIME& st) const throw();

	time_base_t GetTime() const throw();

	int GetYear() const throw();
	int GetMonth() const throw();
	int GetDay() const throw();
	int GetHour() const throw();
	int GetMinute() const throw();
	int GetSecond() const throw();
	int GetDayOfWeek() const throw();

	// strftime
	CString Format(const TCHAR* format) const;
	CString FormatGmt(const TCHAR* format) const;
	CString Format(UINT format_id) const;
	CString FormatGmt(UINT format_id) const;

private:
	time_base_t time_;
};


} // namespace
