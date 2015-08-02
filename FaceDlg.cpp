// FaceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RegEx.h"
#include "FaceDlg.h"
#include <boost/algorithm/string/replace.hpp>
#include "Base64.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// FaceDlg dialog

enum RegExType { PERL= 0, POSIX_BASIC, POSIX_EXT };
enum ShowResults { REPLACEMENTS= 0, CAPTURES };


FaceDlg::FaceDlg(CWnd* pParent /*=NULL*/)
	: CDialog(FaceDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	in_update_ = true;

	dot_matches_eol_.SetChecked(true);
	regex_type_.SetSelected(PERL);
	show_results_.SetSelected(REPLACEMENTS);
}

void FaceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_REGEX, regex_text_);
	DDX_Control(pDX, IDC_REPLACEMENT, replacement_);
	DDX_Control(pDX, IDC_INPUT_TEXT, test_text_);
	DDX_Control(pDX, IDC_RESULT, result_);
	DDX_Control(pDX, IDC_STATUS, status_);
	DDX_Control(pDX, IDC_CASE, case_insensitive_);
	DDX_Control(pDX, IDC_DOT, dot_matches_eol_);
	DDX_Control(pDX, IDC_PARTIAL, partial_matches_);
	DDX_Control(pDX, IDC_PERL, regex_type_);
	DDX_Control(pDX, IDC_REPL, show_results_);
}

static CSize dlg_min_size;

static void GetMinMaxWndSize(CWnd*, MINMAXINFO* mmi)
{
	if (dlg_min_size.cx && dlg_min_size.cy)
	{
		mmi->ptMinTrackSize.x = dlg_min_size.cx;
		mmi->ptMinTrackSize.y = dlg_min_size.cy;
	}
}

// (?<=\d)(?=(?:\d\d\d)+(?!\d))

BEGIN_MESSAGE_MAP(FaceDlg, CDialog)
	ON_EN_CHANGE(IDC_REGEX, &FaceDlg::OnRegExChanged)
	ON_EN_CHANGE(IDC_REPLACEMENT, &FaceDlg::OnRegExChanged)
	ON_EN_CHANGE(IDC_INPUT_TEXT, &FaceDlg::OnRegExChanged)
	ON_WM_GETMINMAXINFO_(&GetMinMaxWndSize)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


enum Style { STYLE_NORMAL= 0, STYLE_RED, STYLE_GREEN, STYLE_BLUE, STYLE_LIGHT, STRIKE_OUT_INDICATOR= 0 };

void SetupScintilla(CScintillaCtrl& ctrl, bool allow_undo)
{
	ctrl.SetUndoCollection(allow_undo);
	ctrl.EmptyUndoBuffer();

	ctrl.SetCodePage(SC_CP_UTF8);

	ctrl.StyleSetFont(STYLE_DEFAULT, "Lucida Console");
	ctrl.StyleSetSize(STYLE_DEFAULT, 10);

	ctrl.SetSelFore(true, ::GetSysColor(COLOR_HIGHLIGHTTEXT));
	ctrl.SetSelBack(true, ::GetSysColor(COLOR_HIGHLIGHT));

	ctrl.SetModEventMask(~(SC_MOD_BEFOREINSERT | SC_MOD_BEFOREDELETE | SC_PERFORMED_USER));
	ctrl.SetWrapMode(1);
	ctrl.SetWrapVisualFlags(SC_WRAPVISUALFLAG_END);

	ctrl.SetTabWidth(4);
	ctrl.SetIndent(4);

	ctrl.StyleSetBack(STYLE_GREEN, RGB(192, 255, 192));
	ctrl.StyleSetBack(STYLE_RED, RGB(255, 192, 192));
	ctrl.StyleSetBack(STYLE_BLUE, RGB(209, 225, 255));
	ctrl.StyleSetFore(STYLE_LIGHT, ::GetSysColor(COLOR_GRAYTEXT));

	ctrl.IndicSetStyle(STRIKE_OUT_INDICATOR, INDIC_STRIKE);
	ctrl.SetIndicatorCurrent(STRIKE_OUT_INDICATOR);

	ctrl.SetMarginWidthN(1, 0);
	ctrl.ModifyStyle(0, WS_TABSTOP);

	DWORD pixels= 1;
	::SystemParametersInfo(SPI_GETCARETWIDTH, 0, &pixels, 0);
	ctrl.SetCaretWidth(pixels);
}


std::wstring GetIniFileName()
{
	wchar_t buf[MAX_PATH]= { 0 };

	DWORD len= ::GetModuleFileName(AfxGetApp()->GetInstanceHandle(), buf, array_count(buf));
	if (len < 3)
		return L"";

	_tcscpy(buf + len - 3, L"ini");
	return buf;
}


// create ini file with Unicode BOM if it doesn't exist or else WritePrivateProfileStringW will create ANSI one
//void CreateInitFileIfNeeded()
//{
//	std::wstring file= GetIniFileName();
//	if (file.empty())
//		return;
//
//	if (::GetFileAttributes(file.c_str()) == DWORD(-1))
//	{
//		// create Unicode text file (with BOM)
//		CFile ini(file.c_str(), CFile::typeBinary | CFile::modeWrite | CFile::modeCreate);
//		ini.Write("\xff\xfe", 2);
//	}
//}

void MultiByteToWideString(const std::string& in, std::wstring& out, int code_page= CP_UTF8)
{
	std::vector<wchar_t> output;
	output.resize(in.size() + 32);
	int len= ::MultiByteToWideChar(code_page, 0, in.c_str(), static_cast<int>(in.size()), &output.front(), static_cast<int>(output.size()));
	out.assign(&output.front(), len);
}


bool WideStringToMultiByte(const std::wstring& in, std::string& out)
{
	auto len= ::WideCharToMultiByte(CP_UTF8, 0, in.data(), static_cast<int>(in.size()), 0, 0, 0, 0);
	if (len > 0)
	{
		out.resize(len, '\0');
		::WideCharToMultiByte(CP_UTF8, 0, in.data(), static_cast<int>(in.size()), &out[0], len, 0, 0);
		return true;
	}
	return false;
}


std::wstring GetString(const wchar_t* section, const wchar_t* key, const wchar_t* default_val)
{
	DWORD size= 1024 * 16;
	std::vector<wchar_t> buf(size + 1, 0);
	std::wstring file= GetIniFileName();
	::GetPrivateProfileStringW(section, key, L".", &buf.front(), size, file.c_str());
	std::wstring str(&buf.front());
	if (str == L".")
		str = default_val ? default_val : L"";
	else
	{
		std::string utf8= base64_decode(str);
		MultiByteToWideString(utf8, str);
	}
	return str;
}


int GetInt(const wchar_t* section, const wchar_t* key, int default_val)
{
	std::wstring file= GetIniFileName();
	return ::GetPrivateProfileIntW(section, key, default_val, file.c_str());
}


bool GetBool(const wchar_t* section, const wchar_t* key, bool default_val)
{
	std::wstring file= GetIniFileName();
	return !!::GetPrivateProfileIntW(section, key, default_val, file.c_str());
}


void SaveStringRaw(const wchar_t* section, const wchar_t* key, const std::wstring& text)
{
	std::wstring file= GetIniFileName();
	::WritePrivateProfileStringW(section, key, text.c_str(), file.c_str());
}


void SaveString(const wchar_t* section, const wchar_t* key, const std::wstring& text)
{
	std::string utf8;
	WideStringToMultiByte(text, utf8);

	std::wstring encoded= base64_encode(utf8.c_str(), utf8.size());
	SaveStringRaw(section, key, encoded);
}


void SaveString(const wchar_t* section, const wchar_t* key, CScintillaCtrl& ctrl)
{
	if (!ctrl.handle())
		return;

	std::string utf8= ctrl.GetUTF8Text();
	std::wstring encoded= base64_encode(utf8.c_str(), utf8.size());
	SaveStringRaw(section, key, encoded);
}

void SaveString(const wchar_t* section, const wchar_t* key, int n)
{
	SaveStringRaw(section, key, boost::lexical_cast<std::wstring>(n));
}

void SaveBool(const wchar_t* section, const wchar_t* key, bool b)
{
	SaveStringRaw(section, key, b ? L"1" : L"0");
}


const wchar_t section[]= L"Settings";


// FaceDlg message handlers

BOOL FaceDlg::OnInitDialog()
{
	try
	{
		dlg_min_size = GetBounds().Size();

		CRect position;
		position.left = GetInt(section, L"x", 0);
		position.top = GetInt(section, L"y", 0);
		position.SetWidth(std::max<int>(GetInt(section, L"w", dlg_min_size.cx), dlg_min_size.cx));
		position.SetHeight(std::max<int>(GetInt(section, L"h", dlg_min_size.cy), dlg_min_size.cy));

		show_results_.SetSelected(GetInt(section, L"show", show_results_.GetSelected()));
		regex_type_.SetSelected(GetInt(section, L"type", regex_type_.GetSelected()));
		partial_matches_.SetChecked(GetBool(section, L"partial", partial_matches_.GetChecked()));
		dot_matches_eol_.SetChecked(GetBool(section, L"dot", dot_matches_eol_.GetChecked()));
		case_insensitive_.SetChecked(GetBool(section, L"case", case_insensitive_.GetChecked()));

		CDialog::OnInitDialog();

		SetupScintilla(regex_text_, true);
		SetupScintilla(replacement_, true);
		SetupScintilla(test_text_, false);
		SetupScintilla(result_, false);
		SetupScintilla(status_, false);

		//CreateInitFileIfNeeded();

		regex_text_.SetText(GetString(section, L"regex", L"fox").c_str());

		result_.ModifyStyle(WS_TABSTOP, 0);

		status_.StyleSetBack(STYLE_NORMAL, ::GetSysColor(COLOR_3DFACE));
		status_.StyleSetBack(STYLE_DEFAULT, ::GetSysColor(COLOR_3DFACE));
		status_.StyleSetFont(STYLE_DEFAULT, "Microsoft Sans Serif");
		status_.StyleSetSize(STYLE_DEFAULT, 8);
		status_.ModifyStyle(WS_TABSTOP, 0);

		test_text_.SetText(GetString(section, L"test_text",
			L"1234567890 Quick brown fox jumps over the lazy dog.\n"
			L"Li Europan lingues es membres del sam familie.\n"
			L"!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ\n"
			L"[\\]^_`abcdefghijklmnopqrstuvwxyz.\n").c_str());

		replacement_.SetText(GetString(section, L"replacement", L"-").c_str());

		SetIcon(m_hIcon, TRUE);			// Set big icon
		SetIcon(m_hIcon, FALSE);		// Set small icon

		CRect b= result_.GetBounds();
		boost::shared_ptr<mcr::CDlgResizer> r= mcr::CreateSimpleDlgCtrlResizer(*this, CPoint(b.right - 1, b.bottom - 1));
		r->SetWindowResizingFlags(test_text_, mcr::CDlgResizer::RESIZE, mcr::CDlgResizer::HALF_RESIZE_V);
		r->SetWindowResizingFlags(result_, mcr::CDlgResizer::MOVE_V_RESIZE, mcr::CDlgResizer::HALF_RESIZE_V | mcr::CDlgResizer::HALF_MOVE_V);
		r->SetWindowResizingFlags(::GetDlgItem(*this, IDC_RESULT_LABEL), mcr::CDlgResizer::MOVE_V, mcr::CDlgResizer::HALF_MOVE_V);
		r->SetWindowResizingFlags(::GetDlgItem(*this, IDC_REPL), mcr::CDlgResizer::MOVE_V, mcr::CDlgResizer::HALF_MOVE_V);
		r->SetWindowResizingFlags(::GetDlgItem(*this, IDC_CAPT), mcr::CDlgResizer::MOVE_V, mcr::CDlgResizer::HALF_MOVE_V);
		SetCtrlResizer(r);

		WINDOWPLACEMENT wp;
		GetWindowPlacement(&wp);
		wp.rcNormalPosition = position;
		SetWindowPlacement(&wp);

		case_insensitive_.OnChecked(boost::bind(&FaceDlg::OnRegExChanged, this));
		dot_matches_eol_.OnChecked(boost::bind(&FaceDlg::OnRegExChanged, this));
		partial_matches_.OnChecked(boost::bind(&FaceDlg::OnRegExChanged, this));
		regex_type_.OnSelected(boost::bind(&FaceDlg::OnRegExChanged, this));
		show_results_.OnSelected(boost::bind(&FaceDlg::OnRegExChanged, this));

//		page_.Create(this, IDD_PAGE1);
//		page_.SetWindowPos(&case_insensitive_, 0,0,0,0, SWP_NOSIZE|SWP_NOMOVE);

		in_update_ = false;

		OnRegExChanged();
	}
	catch (CException* ex)
	{
		ex->ReportError();
		ex->Delete();
		EndDialog(IDCANCEL);
	}
	catch (...)
	{
		MessageBox(L"Unknown exception encountered", L"RegEx", MB_OK | MB_ICONEXCLAMATION);
		EndDialog(IDCANCEL);
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}


std::string ToUTF8(std::wstring str)
{
	// clean up: replace illegal chars with space
	std::wstring null(1, L'\0'), eol(1, '\n'), caret(1, '\r'), space(1, L' ');
	boost::algorithm::replace_all(str, null, space);
	boost::algorithm::replace_all(str, eol, space);
	boost::algorithm::replace_all(str, caret, space);

	std::string out;
	WideStringToMultiByte(str, out);
	return out;
}


std::pair<int, int> UTF8StringPosition(const std::wstring& str, int pos, int len)
{
	if (static_cast<size_t>(pos) >= str.size() || static_cast<size_t>(pos + len) >= str.size() || len < 0)
	{
		ASSERT(false);
		return std::make_pair(0, 0);
	}

	auto utf8_pos= ::WideCharToMultiByte(CP_UTF8, 0, str.data(), pos, 0, 0, 0, 0);
	auto utf8_end= ::WideCharToMultiByte(CP_UTF8, 0, str.data() + pos, len, 0, 0, 0, 0);

	return std::make_pair(utf8_pos, utf8_end);
}


void FaceDlg::OnRegExChanged()
{
	if (in_update_)
		return;

	try
	{
		std::wstring reg_expr= regex_text_.GetText().std_str();
		std::string input_text_utf8= test_text_.GetUTF8Text();
		std::wstring input_text;
		MultiByteToWideString(input_text_utf8, input_text);
		auto repl= replacement_.GetText();

		status_.SetReadOnly(false);

		result_.SetReadOnly(false);
		result_.SetText("");

		// clear all styles
		const int MASK= 0x1f;
		test_text_.StartStyling(0, MASK);
		test_text_.SetStyling(static_cast<int>(input_text_utf8.length()), STYLE_DEFAULT);
		test_text_.IndicatorClearRange(0, static_cast<int>(input_text_utf8.length()));

		boost::regex::flag_type type= 0;

		switch (regex_type_.GetSelected())
		{
		case POSIX_BASIC:
			type |= boost::regex::basic;
			break;
		case POSIX_EXT:
			type |= boost::regex::extended;
			break;
		case PERL:
		default:
			type |= boost::regex::perl;
			break;
		}

		// what to show: text modified by regex or just captures?
		bool show_replecements= show_results_.GetSelected() == REPLACEMENTS;

		if (case_insensitive_.GetChecked())
			type |= boost::regex::icase;

		// regex match
		boost::wregex e(reg_expr, type);

		status_.SetText("OK");

		auto start= input_text.begin();
		std::wstring::const_iterator end = input_text.end();

		auto flags= boost::match_perl;

		if (!dot_matches_eol_.GetChecked())
			flags |= boost::match_not_dot_newline;
		if (partial_matches_.GetChecked())
			flags |= boost::match_partial;

		boost::wsregex_iterator it(start, end, e, flags);
		boost::wsregex_iterator it_end;

		std::string captures;		// accumulate captures here
		std::string repl_results;	// accumulate replacement results here
		std::wstring::const_iterator suffix(start);

		// keep track of replacements/captures to make style coloring possible
		struct Cell
		{
			Cell(int from, int len, Style style) : from(from), len(len), style(style)
			{}
			int from, len;
			Style style;
		};
		std::vector<Cell> styles;

		for ( ; it != it_end; ++it)
		{
			const boost::wsmatch& m= *it;

			if (show_replecements)
			{
				// keep track of replacements

				int pos = static_cast<int>(m.position());
				int len = static_cast<int>(m.length());

				if (repl.empty())
				{
					//TODO: pos in utf8 strings

					if (len > 0)
					{
						auto utf8_pos= UTF8StringPosition(input_text, pos, len);
						test_text_.StartStyling(utf8_pos.first, MASK);
						test_text_.SetStyling(utf8_pos.second, STYLE_GREEN);
					}
				}
				else
				{
					if (len > 0)
					{
						auto utf8_pos= UTF8StringPosition(input_text, pos, len);
						test_text_.IndicatorFillRange(utf8_pos.first, utf8_pos.second);
						test_text_.StartStyling(utf8_pos.first, MASK);
						test_text_.SetStyling(utf8_pos.second, STYLE_RED);
					}
				}

				if (!m.empty() && m[0].matched && !repl.empty())
				{
					std::wstringstream ost;
					std::ostream_iterator<wchar_t, wchar_t> it(ost);
					m.format(it, repl.std_str());
					std::wstring txt= ost.str();

					repl_results.append(ToUTF8(m.prefix()));
					styles.push_back(Cell(static_cast<int>(repl_results.size()), static_cast<int>(txt.size()), STYLE_GREEN));
					repl_results.append(ToUTF8(txt));

					suffix = m[0].second;
				}
			}
			else
			{
				// keep track of captures

				if (!m.empty() && m[0].matched)
				{
					std::stringstream ost;
					int base= static_cast<int>(captures.size());
					for (int i= 1; i < m.size(); ++i)
					{
						int from= static_cast<int>(ost.tellp());
						ost << i << ". ";
						int to= static_cast<int>(ost.tellp());
						styles.push_back(Cell(base + from, to - from, STYLE_LIGHT));

						ost << ToUTF8(m[i].str());
						int end= static_cast<int>(ost.tellp());
						styles.push_back(Cell(base + to, end - to, STYLE_BLUE));

						if (i + 1 < m.size())
							ost << '\t';
					}
					ost << '\n';
					captures.append(ost.str());
				}
				else
				{
					captures.append("<None>\n");
				}
			}
		}

		bool apply_styles= false;

		if (show_replecements)
		{
			if (!styles.empty())
			{
				std::wstring w(suffix, end);
				repl_results.append(ToUTF8(w));

				result_.SetText(repl_results.c_str());

				if (!repl_results.empty() && repl_results[0] != 0)
					apply_styles = true;
			}
		}
		else
		{
			result_.SetText(captures.c_str());

			if (!captures.empty())
				apply_styles = true;
		}

		if (apply_styles)
		{
			const size_t count= styles.size();
			for (size_t i= 0; i < count; ++i)
			{
				auto pos= styles[i].from;
				auto len= styles[i].len;

				result_.StartStyling(pos, MASK);
				result_.SetStyling(len, styles[i].style);
			}
		}

	}
	catch (boost::regex_error& ex)
	{
		status_.SetText(ex.what());
	}
	catch (std::exception& ex)
	{
		status_.SetText(ex.what());
	}
	catch (...)
	{
		status_.SetText("Fatal error");
	}

	result_.SetReadOnly(true);
	status_.SetReadOnly(true);
}


void FaceDlg::OnClose()
{
	WINDOWPLACEMENT wp;
	if (GetWindowPlacement(&wp))
	{
		CRect r= wp.rcNormalPosition;
		SaveString(section, L"x", r.left);
		SaveString(section, L"y", r.top);
		SaveString(section, L"w", r.Width());
		SaveString(section, L"h", r.Height());
	}

	SaveString(section, L"regex", regex_text_);
	SaveString(section, L"replacement", replacement_);
	SaveString(section, L"test_text", test_text_);

	SaveString(section, L"partial", partial_matches_.GetChecked());
	SaveString(section, L"case", case_insensitive_.GetChecked());
	SaveString(section, L"dot", dot_matches_eol_.GetChecked());
	SaveString(section, L"type", regex_type_.GetSelected());
	SaveString(section, L"show", show_results_.GetSelected());

	CDialog::OnClose();
}


/////////////////////////////////////////////////////


/*

struct IndexEntry
{
	uint8 letter;
	uint32 offset;
};


struct cmp_entry
{
	bool operator () (char letter, const IndexEntry& entry)
	{
		return entry.letter > letter;
	}

	bool operator () (const IndexEntry& entry, char letter)
	{
		return entry.letter < letter;
	}

	//bool operator () (const IndexEntry& entry, const IndexEntry& entr)
	//{
	//	return false;
	//}
};


void test()
{
	IndexEntry offsets_[25];
	const int count_= 25;

	char start= 'd';

	std::pair<const IndexEntry*, const IndexEntry*> entry= std::equal_range(offsets_, offsets_ + count_, start, cmp_entry());
}
*/
