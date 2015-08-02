#include "stdafx.h"
#include "micron.h"


namespace mcr {

static void fillExceptionInfo(file_exception* exception, const TCHAR* file_name)
{
	if (exception != 0)
	{
		exception->os_error_ = ::GetLastError();
		exception->cause_ = 1;//CFileException::OsErrorToException(pException->m_lOsError);
		exception->file_name_ = file_name;
	}
}


CFile::CFile(const wchar_t* filepath, uint32 mode)
{
	file_ = INVALID_HANDLE_VALUE;
	close_on_delete_ = false;

	file_exception exception;
	if (!Open(filepath, mode, &exception))
		throw exception;
}


CFile::CFile()
{
	file_ = INVALID_HANDLE_VALUE;
	close_on_delete_ = false;
}


CFile::~CFile()
{
	if (close_on_delete_)
		close_file();
}


bool CFile::close_file()
{
	bool ok= true;
	if (file_ != INVALID_HANDLE_VALUE)
		ok = ::CloseHandle(file_) != 0;

	file_ = INVALID_HANDLE_VALUE;
	close_on_delete_ = false;
	file_name_.clear();

	return ok;
}


void CFile::Close()
{
//	ASSERT_VALID(this);
	ASSERT(file_ != INVALID_HANDLE_VALUE);

	if (!close_file())
		throw file_exception(); //CFileException::ThrowOsError((LONG)::GetLastError(), m_strFileName);
}


uint64 CFile::Seek(int64 offset, SeekPosition seek_pos)
{
	LARGE_INTEGER pos;
	pos.QuadPart = offset;

	pos.LowPart = ::SetFilePointer(file_, pos.LowPart, &pos.HighPart, static_cast<DWORD>(seek_pos));

	if (pos.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
		throw mcr::micron_exception("file bad seek");//CFileException::ThrowOsError((LONG)::GetLastError(), file_name_);

	return pos.QuadPart;
}


uint32 CFile::Read(void* buffer, uint32 size)
{
	if (size == 0)
		return 0;

	ASSERT(buffer != 0);
	//ASSERT(AfxIsValidAddress(lpBuf, nCount));

	DWORD read= 0;
	if (!::ReadFile(file_, buffer, size, &read, 0))
		throw mcr::micron_exception("file read error");//CFileException::ThrowOsError((LONG)::GetLastError(), file_name_);

	return static_cast<uint32>(read);
}


uint64 CFile::GetLength()
{
	ULARGE_INTEGER size;
	size.LowPart = ::GetFileSize(file_, &size.HighPart);
	if (size.LowPart == INVALID_FILE_SIZE && ::GetLastError() != NO_ERROR)
		throw mcr::micron_exception("file get size error");//CFileException::ThrowOsError((LONG)::GetLastError(), file_name_);
		// CFileException::ThrowOsError((LONG)::GetLastError(), file_name_);

	return size.QuadPart;
}


bool CFile::Open(const TCHAR* file_name, uint32 open_flags, file_exception* exception)
{
	//ASSERT_VALID(this);
	//ASSERT(AfxIsValidString(file_name));

	//ASSERT(exception == NULL ||
	//	AfxIsValidAddress(exception, sizeof(CFileException)));
	//ASSERT((open_flags & typeText) == 0);   // text mode not supported

	// shouldn't open an already open file (it will leak)
	ASSERT(file_ == INVALID_HANDLE_VALUE);

	// CFile objects are always binary and CreateFile does not need flag
	open_flags &= ~uint32(typeBinary);

	close_on_delete_ = false;

	file_ = INVALID_HANDLE_VALUE;
	file_name_.clear();

	TCHAR temp[_MAX_PATH];
	if (file_name != 0 && _tcslen(file_name) < _MAX_PATH)
	{
		if (!::PathCanonicalize(temp, file_name))
		{
			if (exception)
			{
				exception->cause_ = CFileException::badPath;
				exception->file_name_ = file_name;
			}
			return false;
		}
//		if (_AfxFullPath2(temp, file_name,exception) == false)
//			return false;
	}
	else
	{
		// user passed in a buffer greater then _MAX_PATH
		if (exception)
		{
			exception->cause_ = CFileException::badPath;
			exception->file_name_ = file_name;
		}
		return false; // path is too long
	}
		
	file_name_ = temp;
	ASSERT(shareCompat == 0);

	// map read/write mode
	ASSERT((modeRead | modeWrite | modeReadWrite) == 3);
	DWORD access= 0;
	switch (open_flags & 3)
	{
	case modeRead:
		access = GENERIC_READ;
		break;
	case modeWrite:
		access = GENERIC_WRITE;
		break;
	case modeReadWrite:
		access = GENERIC_READ | GENERIC_WRITE;
		break;
	default:
		ASSERT(FALSE);  // invalid share mode
	}

	// map share mode
	DWORD share_mode= 0;
	switch (open_flags & 0x70)    // map compatibility mode to exclusive
	{
	default:
		ASSERT(FALSE);  // invalid share mode?
	case shareCompat:
	case shareExclusive:
		share_mode = 0;
		break;
	case shareDenyWrite:
		share_mode = FILE_SHARE_READ;
		break;
	case shareDenyRead:
		share_mode = FILE_SHARE_WRITE;
		break;
	case shareDenyNone:
		share_mode = FILE_SHARE_WRITE | FILE_SHARE_READ;
		break;
	}

	// Note: typeText and typeBinary are used in derived classes only.

	// map modeNoInherit flag
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = (open_flags & modeNoInherit) == 0;

	// map creation flags
	DWORD create_flag;
	if (open_flags & modeCreate)
	{
		if (open_flags & modeNoTruncate)
			create_flag = OPEN_ALWAYS;
		else
			create_flag = CREATE_ALWAYS;
	}
	else
		create_flag = OPEN_EXISTING;

	// system-level access flags

	// random access and sequential scan should be mutually exclusive
	ASSERT((open_flags & (osRandomAccess | osSequentialScan)) != (osRandomAccess | osSequentialScan));

	DWORD flags = FILE_ATTRIBUTE_NORMAL;
	if (open_flags & osNoBuffer)
		flags |= FILE_FLAG_NO_BUFFERING;
	if (open_flags & osWriteThrough)
		flags |= FILE_FLAG_WRITE_THROUGH;
	if (open_flags & osRandomAccess)
		flags |= FILE_FLAG_RANDOM_ACCESS;
	if (open_flags & osSequentialScan)
		flags |= FILE_FLAG_SEQUENTIAL_SCAN;

	// attempt file creation
	HANDLE file = ::CreateFile(file_name, access, share_mode, &sa, create_flag, flags, NULL);
	if (file == INVALID_HANDLE_VALUE)
	{
		fillExceptionInfo(exception, file_name);
		return false;
	}

	file_ = file;
	close_on_delete_ = true;

	return true;
}


uint64 CFile::GetPosition() const
{
	ASSERT(0);
	return 0;
}

void CFile::Write(const void* buffer, UINT count)
{
	if (count == 0)
		return;

	ASSERT(buffer != 0);
	//ASSERT(AfxIsValidAddress(buffer, count));

	DWORD written= 0;
	if (!::WriteFile(file_, buffer, count, &written, 0) || written != count)
		throw mcr::micron_exception("file write error");//CFileException::ThrowOsError((LONG)::GetLastError(), file_name_);
}

//	virtual void Abort();
void CFile::Flush()
{
	ASSERT(0);
}

void CFile::SetLength(uint64 new_length)
{
	ASSERT(0);
}

} // namespace