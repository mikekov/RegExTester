#pragma once
#include "basetypes.h"

namespace mcr {


class CFile
{
public:
	CFile();
	CFile(const wchar_t* filepath, uint32 mode);
	~CFile();

	bool Open(const TCHAR* file_name, uint32 open_flags, file_exception* exception= 0);

	enum SeekPosition { begin= 0, current= 1, end= 2 };

	uint64 Seek(int64 offset, SeekPosition pos);
	uint64 SeekToEnd();
	void SeekToBegin();
	virtual uint64 GetPosition() const;

	uint32 Read(void* buffer, uint32 size);
	virtual void Write(const void* buffer, UINT count);

//	virtual void Abort();
	virtual void Flush();
	virtual void Close();

	uint64 GetLength();
	virtual void SetLength(uint64 new_length);

	bool IsOpen() const;

	enum OpenFlags
	{
		modeRead= 0, modeWrite, modeReadWrite,
		shareCompat=      0x00000,
		shareExclusive=   0x00010,
		shareDenyWrite=   0x00020,
		shareDenyRead=    0x00030,
		shareDenyNone=    0x00040,
		modeNoInherit=    0x00080,
		modeCreate=       0x01000,
		modeNoTruncate=   0x02000,
		typeText=         0x04000,
		typeBinary=       0x08000,
		osNoBuffer=       0x10000,
		osWriteThrough=   0x20000,
		osRandomAccess=   0x40000,
		osSequentialScan= 0x80000
	};

private:
	HANDLE file_;
	bool close_on_delete_;
	CString file_name_;

	bool close_file();
};


} // namespace
