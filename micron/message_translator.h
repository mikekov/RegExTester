#pragma once
#include "dc.h"
#include "message_map.h"


namespace mcr {


template <class T, class B>
class up_cast_t
{
public:
    typedef T result_type;
#ifdef _DEBUG
	T operator () (B b) { return dynamic_cast<T>(b); }
#else
	T operator () (B b) { return static_cast<T>(b); }
#endif
};


/*
template<int WM>
struct msg_traits
{
	typedef LRESULT Ret;
	typedef WPARAM A;
	typedef LPARAM B;
};*/


template<class Ret, class T, class A= void, class B= void, class C= void>
struct msg_callback
{
	typedef boost::function<Ret (T*, A, B, C)> CB;
};


template<class Ret, class T, class A, class B>
struct msg_callback<Ret, T, A, B, void>
{
	typedef boost::function<Ret (T*, A, B)> CB;
};


template<class Ret, class T, class A>
struct msg_callback<Ret, T, A, void, void>
{
	typedef boost::function<Ret (T*, A)> CB;
};


template<class Ret, class T>
struct msg_callback<Ret, T, void, void, void>
{
	typedef boost::function<Ret (T*)> CB;
};


typedef msg_callback<LRESULT, mcr::CCmdTarget, WPARAM, LPARAM>::CB GenericMessageCallback;
typedef msg_callback<void, mcr::CCmdTarget, WPARAM, LPARAM>::CB GenericMessageVoidCallback;


// messages are translated from their raw form (wparam/lparam) to the one where params
// are already casted into message-specific form;
// this default message translator is a pass-through, but specialized versions are used
// to provide decoding for given params
template<int WM, class T, int SIGNATURE= FN_SIG_DEFAULT>
struct msg_translator
{
	typedef LRESULT Ret;
	typedef WPARAM A;
	typedef LPARAM B;
	typedef typename msg_callback<Ret, T, A, B>::CB CB;

	static void call(T* wnd, Msg& m, const CB& handler)
	{ m.result = handler(wnd, m.wparam, m.lparam); }
};


template<class T>
struct msg_translator<WM_SIZE, T>
{
	typedef void Ret;
	typedef int A;
	typedef int B;
	typedef int C;
	typedef typename msg_callback<Ret, T, A, B, C>::CB CB;

	//template<class S>
	//static Ret (S::*DefaultHandler())(A, B, C)		{ return &S::OnSize; }

	static void call(T* wnd, Msg& m, const CB& handler)
	{ handler(wnd, static_cast<A>(m.wparam), static_cast<B>(LOWORD(m.lparam)), static_cast<C>(HIWORD(m.lparam))); }
};


template<class T>
struct msg_translator<WM_COMMAND, T>	// special; handler invoked by OnCmdMsg
{
	typedef void Ret;
	typedef typename msg_callback<Ret, T>::CB CB;

	static void call(T* wnd, Msg& m, const CB& handler)		{ handler(wnd); }
};


template<class T>
struct msg_translator<WM_COMMAND, T, FN_SIG_BOOL_UINT>
{
	typedef BOOL Ret;
	typedef UINT A;
	typedef typename msg_callback<Ret, T, A>::CB CB;

//	static void call(T* wnd, Msg& m, const CB& handler)		{ handler(wnd); }
};


template<class T>
struct msg_translator<WM_COMMAND, T, FN_SIG_VOID_UINT>
{
	typedef void Ret;
	typedef UINT A;
	typedef typename msg_callback<Ret, T, A>::CB CB;

//	static void call(T* wnd, Msg& m, const CB& handler)		{ handler(wnd); }
};


template<class T>
struct msg_translator<WM_UPDATE_COMMAND_UI, T>	// special; not a regular message
{
	typedef void Ret;
	typedef CCmdUI* A;
	typedef typename msg_callback<Ret, T, A>::CB CB;
};


template<class T>
struct msg_translator<WM_UPDATE_COMMAND_UI, T, FN_SIG_BOOL_UINT>
{
	typedef BOOL Ret;
	typedef CCmdUI* A;
	typedef UINT B;
	typedef typename msg_callback<Ret, T, A>::CB CB;
};


template<class T>
struct msg_translator<WM_PAINT, T>
{
	typedef void Ret;
	typedef typename msg_callback<Ret, T>::CB CB;

	static void call(T* wnd, Msg& m, const CB& handler)		{ handler(wnd); }
};


template<class T>
struct msg_translator<WM_ERASEBKGND, T>
{
	typedef BOOL Ret;
	typedef mcr::CDC* A;
	typedef typename msg_callback<Ret, T, A>::CB CB;

	static void call(T* wnd, Msg& m, const CB& handler)
	{ mcr::CDC* dc= CDC::FromHandle(reinterpret_cast<HDC>(m.wparam)); m.result = handler(wnd, dc); }
};


template<class T>
struct msg_translator<WM_NCDESTROY, T>
{
	typedef void Ret;
	typedef typename msg_callback<Ret, T>::CB CB;

	static void call(T* wnd, Msg& m, const CB& handler)		{ handler(wnd); }
};


template<class T>
struct msg_translator<WM_CLOSE, T>
{
	typedef void Ret;
	typedef typename msg_callback<Ret, T>::CB CB;

	static void call(T* wnd, Msg& m, const CB& handler)		{ handler(wnd); }
};


template<class T>
struct msg_translator<WM_INITDIALOG, T>
{
	typedef BOOL Ret;
	typedef HWND A;
	typedef void* B;
	typedef typename msg_callback<Ret, T, A, B>::CB CB;

	static void call(T* wnd, Msg& m, const CB& handler)
	{
		HWND ctrl= reinterpret_cast<HWND>(m.wparam);
		void* param= reinterpret_cast<void*>(m.lparam);
		m.result = handler(wnd, ctrl, param);
	}
};


template<class T>
struct msg_translator<WM_NOTIFY, T>
{
	typedef void Ret;
	typedef NMHDR* A;
	typedef LRESULT* B;
	typedef typename msg_callback<Ret, T, A, B>::CB CB;

	static void call(T* wnd, Msg& m, const CB& handler)
	{
		NMHDR* hdr= reinterpret_cast<NMHDR*>(m.lparam);
		m.result = 0;
		handler(wnd, hdr, &m.result);
	}
};


template<class T>
struct msg_translator<WM_NOTIFY, T, FN_SIG_BOOL_UINT_NMHDR_RES>
{
	typedef BOOL Ret;
	typedef UINT A;
	typedef NMHDR* B;
	typedef LRESULT* C;
	typedef typename msg_callback<Ret, T, A, B, C>::CB CB;
};


template<class T>
struct msg_translator<WM_DRAWITEM, T>
{
	typedef void Ret;
	typedef int A;
	typedef DRAWITEMSTRUCT* B;
	typedef typename msg_callback<Ret, T, A, B>::CB CB;

	static void call(T* wnd, Msg& m, const CB& handler)
	{
		A id= static_cast<A>(m.wparam);
		B draw_item= reinterpret_cast<B>(m.lparam);
		handler(wnd, id, draw_item);
	}
};


template<class T>
struct msg_translator<WM_MEASUREITEM, T>
{
	typedef void Ret;
	typedef int A;
	typedef MEASUREITEMSTRUCT* B;
	typedef typename msg_callback<Ret, T, A, B>::CB CB;

	static void call(T* wnd, Msg& m, const CB& handler)
	{
		A id= static_cast<A>(m.wparam);
		B measure_item= reinterpret_cast<B>(m.lparam);
		handler(wnd, id, measure_item);
	}
};


template<class T>
struct msg_translator<WM_LBUTTONDOWN, T>
{
	typedef void Ret;
	typedef UINT A;
	typedef CPoint B;
	typedef typename msg_callback<Ret, T, A, B>::CB CB;

	static void call(T* wnd, Msg& m, const CB& handler)
	{
		A flags= static_cast<A>(m.wparam);
		B pt= PointFromLParam(m.lparam);
		handler(wnd, flags, pt);
	}
};


template<class T>
struct msg_translator<WM_LBUTTONUP, T> : public msg_translator<WM_LBUTTONDOWN, T>
{};

template<class T>
struct msg_translator<WM_MOUSEMOVE, T> : public msg_translator<WM_LBUTTONDOWN, T>
{};


template<class T>
struct msg_translator<WM_TIMER, T>
{
	typedef void Ret;
	typedef UINT_PTR A;
	typedef typename msg_callback<Ret, T, A>::CB CB;

	static void call(T* wnd, Msg& m, const CB& handler)
	{
		A event_id= static_cast<A>(m.wparam);
		handler(wnd, event_id);
	}
};


template<class T>
struct msg_translator<WM_SETCURSOR, T>
{
	typedef BOOL Ret;
	typedef CWnd* A;
	typedef UINT B;
	typedef UINT C;
	typedef typename msg_callback<Ret, T, A, B, C>::CB CB;

	static void call(T* wnd, Msg& m, const CB& handler)
	{
		A w= CWnd::FromHandle(reinterpret_cast<HWND>(m.wparam));
		B hit_test= LOWORD(lparam);
		C msg= HIWORD(lparam);
		handler(wnd, w, hit_test, msg);
	}
};


template<class T>
struct msg_translator<WM_GETMINMAXINFO, T>
{
	typedef void Ret;
	typedef MINMAXINFO* A;
	typedef typename msg_callback<Ret, T, A>::CB CB;

	static void call(T* wnd, Msg& m, const CB& handler)
	{
		MINMAXINFO* minmax= reinterpret_cast<MINMAXINFO*>(m.lparam);
		m.result = 0;
		handler(wnd, minmax);
	}
};


//=============================================================================================================


} // namespace


#define _UPCAST_WND_(w)		boost::bind(mcr::up_cast_t<ThisClass*, mcr::CCmdTarget*>(), (w))

/* generate msg map (lua)
msg = { "Size", "Paint", "DrawItem", "MeasureItem", "NcDestroy", "LButtonUp", "LButtonDown", "MouseMove", "Timer", "SetCursor" }
len = 40
tab = 4

for i, name in ipairs(msg) do
	m = string.upper(name)
	s = "#define ON_WM_" .. m .. "_(fn)"
	n = math.max(1, math.floor((len - s:len() - 1) / tab))
	print (s .. string.rep("\t", n) .. ".add_<WM_" .. m .. ", ThisClass>((fn))")

	s = "#define ON_WM_" .. m .. "()"
	n = math.max(1, math.floor((len - s:len() - 1) / tab))
	print (s .. string.rep("\t", n) .. "ON_WM_" .. m .. "_(&ThisClass::On" .. name .. ")\n")
end
*/

#include "message_map.gen"

/*
#define ON_WM_SIZE_(fn)				.add_<WM_SIZE, ThisClass>((fn))
#define ON_WM_SIZE()				ON_WM_SIZE_(&ThisClass::OnSize)

#define ON_WM_PAINT_(fn)			.add_<WM_PAINT, ThisClass>((fn))
#define ON_WM_PAINT()				ON_WM_PAINT_(&ThisClass::OnPaint)

#define ON_WM_DRAWITEM_(fn)			.add_<WM_DRAWITEM, ThisClass>((fn))
#define ON_WM_DRAWITEM()			ON_WM_DRAWITEM_(&ThisClass::OnDrawItem)

#define ON_WM_MEASUREITEM_(fn)		.add_<WM_MEASUREITEM, ThisClass>((fn))
#define ON_WM_MEASUREITEM()			ON_WM_MEASUREITEM_(&ThisClass::OnMeasureItem)

#define ON_WM_NCDESTROY_(fn)		.add_<WM_NCDESTROY, ThisClass>((fn))
#define ON_WM_NCDESTROY()			ON_WM_NCDESTROY_(&ThisClass::OnNcDestroy)

#define ON_WM_LBUTTONUP_(fn)		.add_<WM_LBUTTONUP, ThisClass>((fn))
#define ON_WM_LBUTTONUP()			ON_WM_LBUTTONUP_(&ThisClass::OnLButtonUp)

#define ON_WM_LBUTTONDOWN_(fn)		.add_<WM_LBUTTONDOWN, ThisClass>((fn))
#define ON_WM_LBUTTONDOWN()			ON_WM_LBUTTONDOWN_(&ThisClass::OnLButtonDown)

#define ON_WM_MOUSEMOVE_(fn)		.add_<WM_MOUSEMOVE, ThisClass>((fn))
#define ON_WM_MOUSEMOVE()			ON_WM_MOUSEMOVE_(&ThisClass::OnMouseMove)

#define ON_WM_TIMER_(fn)			.add_<WM_TIMER, ThisClass>((fn))
#define ON_WM_TIMER()				ON_WM_TIMER_(&ThisClass::OnTimer)

#define ON_WM_SETCURSOR_(fn)		.add_<WM_SETCURSOR, ThisClass>((fn))
#define ON_WM_SETCURSOR()			ON_WM_SETCURSOR_(&ThisClass::OnSetCursor)

#define ON_WM_ERASEBKGND_(fn)		.add_<WM_ERASEBKGND, ThisClass>((fn))
#define ON_WM_ERASEBKGND()			ON_WM_ERASEBKGND_(&ThisClass::OnEraseBkgnd)

//ON_WM_DESTROY
*/

// internal
#define ON_WM_INITDIALOG()			.add_<WM_INITDIALOG, ThisClass>(&ThisClass::HandleInitDialog)

#define ON_MESSAGE(msg, fn) \
	.add_<mcr::GenericMessageCallback, ThisClass>((msg), (fn), 0, 0, 0, mcr::FN_SIG_GENERIC_MESSAGE_CALLBACK)

//	.add((msg), mcr::GenericMessageCallback(boost::bind((fn), _UPCAST_WND_(_1), _2, _3)), mcr::FN_SIG_GENERIC_MESSAGE_CALLBACK)


#define ON_CONTROL(notifyCode, id, fn) \
	.add_<WM_COMMAND, ThisClass>((fn), (notifyCode), (id), (id))

#define ON_COMMAND_RANGE(id_first, id_last, fn) \
	.add_<WM_COMMAND, ThisClass, mcr::FN_SIG_VOID_UINT>((fn), mcr::NC_COMMAND, (id_first), (id_last))

#define ON_COMMAND(id, fn) \
	.add_<WM_COMMAND, ThisClass>((fn), mcr::NC_COMMAND, (id), (id))

#define ON_COMMAND_EX(id, fn) \
	.add_<WM_COMMAND, ThisClass, mcr::FN_SIG_BOOL_UINT>((fn), mcr::NC_COMMAND, (id), (id))

#define ON_COMMAND_EX_RANGE(id_first, id_last, fn) \
	.add_<WM_COMMAND, ThisClass, mcr::FN_SIG_BOOL_UINT>((fn), mcr::NC_COMMAND, (id_first), (id_last))


#define ON_UPDATE_COMMAND_UI(id, fn) \
	.add_<mcr::WM_UPDATE_COMMAND_UI, ThisClass>((fn), mcr::NC_COMMAND, (id), (id))

//.add(mcr::WM_UPDATE_COMMAND_UI, (id), (id), \
//	mcr::msg_translator<mcr::WM_UPDATE_COMMAND_UI, mcr::CCmdTarget>::CB(boost::bind((fn), \
//	_UPCAST_WND_(_1))))


#define ON_UPDATE_COMMAND_UI_RANGE(id_from, id_to, fn) \
	.add_<mcr::WM_UPDATE_COMMAND_UI, ThisClass, mcr::FN_SIG_BOOL_UINT>((fn), mcr::NC_COMMAND, (id_from), (id_to))

//.add(mcr::WM_UPDATE_COMMAND_UI, (id), (id), \
//	mcr::msg_translator<mcr::WM_UPDATE_COMMAND_UI, mcr::CCmdTarget, mcr::FN_SIG_BOOL_UINT>::CB(boost::bind((fn), \
//	_UPCAST_WND_(_1))), FN_SIG_BOOL_UINT)


#define ON_NOTIFY_REFLECT(notification_code, fn) \
	.reflect_<WM_NOTIFY, ThisClass>((fn), (notification_code), 0, 0)
//.add(WM_NOTIFY | mcr::WM_REFLECT_FLAG, (notification_code), 0, 0, mcr::msg_translator<WM_NOTIFY, mcr::CCmdTarget>::CB(boost::bind((fn), _UPCAST_WND_(_1), _2, _3)))


#define ON_NOTIFY_RANGE(notification_code, id_first, id_last, fn) \
	.add_<WM_NOTIFY, ThisClass>((fn), (notification_code), (id_first), (id_last))

//.add(WM_NOTIFY, (notification_code), (id_first), (id_last), mcr::msg_translator<WM_NOTIFY, mcr::CCmdTarget>::CB(boost::bind((fn), _UPCAST_WND_(_1), _2, _3)))


#define ON_NOTIFY(notification_code, id, fn)	ON_NOTIFY_RANGE((notification_code), (id), (id), (fn))


#define ON_NOTIFY_EX_RANGE(notification_code, id_first, id_last, fn) \
	.add_<WM_NOTIFY, ThisClass, mcr::FN_SIG_BOOL_UINT_NMHDR_RES>((fn), (notification_code), (id_first), (id_last))
//.add(WM_NOTIFY, (notification_code), (id_first), (id_last), mcr::msg_translator<WM_NOTIFY, mcr::CCmdTarget, mcr::FN_SIG_BOOL_UINT_NMHDR_RES>::CB(boost::bind((fn), _UPCAST_WND_(_1), _2, _3, _4)), FN_SIG_BOOL_UINT_NMHDR_RES)


#define ON_NOTIFY_EX(notification_code, id, fn)		ON_NOTIFY_EX_RANGE((notification_code), (id), (id), (fn))


#define ON_EN_CHANGE(id, fn)			ON_CONTROL(EN_CHANGE, (id), (fn))
#define ON_BN_CLICKED(id, fn)			ON_CONTROL(BN_CLICKED, (id), (fn))
#define ON_LBN_SELCHANGE(id,fn)			ON_CONTROL(LBN_SELCHANGE, (id), (fn))







#if 0
#pragma once
#include "basetypes.h"
#include "dc.h"
#include "message_map.h"


namespace mcr {



template <class T, class B>
class up_cast_t
{
public:
    typedef T result_type;
#ifdef _DEBUG
	T operator () (B b) { return dynamic_cast<T>(b); }
#else
	T operator () (B b) { return static_cast<T>(b); }
#endif
};


/*
template<int WM>
struct msg_traits
{
	typedef LRESULT Ret;
	typedef WPARAM A;
	typedef LPARAM B;
};*/


template<class Ret, class T, class A= void, class B= void, class C= void>
struct msg_callback
{
	typedef boost::function<Ret (T*, A, B, C)> CB;
};

template<class Ret, class T, class A, class B>
struct msg_callback<Ret, T, A, B, void>
{
	typedef boost::function<Ret (T*, A, B)> CB;
};

template<class Ret, class T, class A>
struct msg_callback<Ret, T, A, void, void>
{
	typedef boost::function<Ret (T*, A)> CB;
};

template<class Ret, class T>
struct msg_callback<Ret, T, void, void, void>
{
	typedef boost::function<Ret (T*)> CB;
};


typedef msg_callback<LRESULT, mcr::CCmdTarget, WPARAM, LPARAM>::CB GenericMessageCallback;
typedef msg_callback<void, mcr::CCmdTarget, WPARAM, LPARAM>::CB GenericMessageVoidCallback;


// messages are translated from their raw form (wparam/lparam) to the one where params
// are already casted into message-specific form;
// this default message translator is a pass-through, but specialized versions are used
// to provide decoding for given params
template<int WM, class T, int SIGNATURE= FN_SIG_DEFAULT>
struct msg_translator
{
	typedef LRESULT Ret;
	typedef WPARAM A;
	typedef LPARAM B;
	typedef typename msg_callback<Ret, T, A, B>::CB CB;

	static void call(T* wnd, Msg& m, const CB& handler)
	{ m.result = handler(wnd, m.wparam, m.lparam); }
};


template<class T>
struct msg_translator<WM_SIZE, T>
{
	typedef void Ret;
	typedef int A;
	typedef int B;
	typedef int C;
	typedef typename msg_callback<Ret, T, A, B, C>::CB CB;

	static void call(T* wnd, Msg& m, const CB& handler)
	{ handler(wnd, static_cast<A>(m.wparam), static_cast<B>(LOWORD(m.lparam)), static_cast<C>(HIWORD(m.lparam))); }
};


template<class T>
struct msg_translator<WM_COMMAND, T>	// special; handler invoked by OnCmdMsg
{
	typedef void Ret;
	typedef typename msg_callback<Ret, T>::CB CB;

	static void call(T* wnd, Msg& m, const CB& handler)		{ handler(wnd); }
};


template<class T>
struct msg_translator<WM_COMMAND, T, FN_SIG_BOOL_UINT>
{
	typedef BOOL Ret;
	typedef UINT A;
	typedef typename msg_callback<Ret, T, A>::CB CB;

//	static void call(T* wnd, Msg& m, const CB& handler)		{ handler(wnd); }
};


template<class T>
struct msg_translator<WM_UPDATE_COMMAND_UI, T>	// special; not a regular message
{
	typedef void Ret;
	typedef CCmdUI* A;
	typedef typename msg_callback<Ret, T, A>::CB CB;
};


template<class T>
struct msg_translator<WM_UPDATE_COMMAND_UI, T, FN_SIG_BOOL_UINT>
{
	typedef BOOL Ret;
	typedef CCmdUI* A;
	typedef UINT B;
	typedef typename msg_callback<Ret, T, A>::CB CB;
};


template<class T>
struct msg_translator<WM_PAINT, T>
{
	typedef void Ret;
	typedef typename msg_callback<Ret, T>::CB CB;

	static void call(T* wnd, Msg& m, const CB& handler)		{ handler(wnd); }
};


template<class T>
struct msg_translator<WM_ERASEBKGND, T>
{
	typedef BOOL Ret;
	typedef mcr::CDC* A;
	typedef typename msg_callback<Ret, T, A>::CB CB;

	static void call(T* wnd, Msg& m, const CB& handler)
	{ mcr::CDC* dc= CDC::FromHandle(reinterpret_cast<HDC>(m.wparam)); m.result = handler(wnd, dc); }
};


template<class T>
struct msg_translator<WM_NCDESTROY, T>
{
	typedef void Ret;
	typedef typename msg_callback<Ret, T>::CB CB;

	static void call(T* wnd, Msg& m, const CB& handler)		{ handler(wnd); }
};


template<class T>
struct msg_translator<WM_INITDIALOG, T>
{
	typedef BOOL Ret;
	typedef HWND A;
	typedef void* B;
	typedef typename msg_callback<Ret, T, A, B>::CB CB;

	static void call(T* wnd, Msg& m, const CB& handler)
	{
		HWND ctrl= reinterpret_cast<HWND>(m.wparam);
		void* param= reinterpret_cast<void*>(m.lparam);
		m.result = handler(wnd, ctrl, param);
	}
};


template<class T>
struct msg_translator<WM_NOTIFY, T>
{
	typedef void Ret;
	typedef NMHDR* A;
	typedef LRESULT* B;
	typedef typename msg_callback<Ret, T, A, B>::CB CB;

	static void call(T* wnd, Msg& m, const CB& handler)
	{
		NMHDR* hdr= reinterpret_cast<NMHDR*>(m.lparam);
		m.result = 0;
		handler(wnd, hdr, &m.result);
	}
};


template<class T>
struct msg_translator<WM_NOTIFY, T, FN_SIG_BOOL_UINT_NMHDR_RES>
{
	typedef BOOL Ret;
	typedef UINT A;
	typedef NMHDR* B;
	typedef LRESULT* C;
	typedef typename msg_callback<Ret, T, A, B, C>::CB CB;
};


template<class T>
struct msg_translator<WM_DRAWITEM, T>
{
	typedef void Ret;
	typedef int A;
	typedef DRAWITEMSTRUCT* B;
	typedef typename msg_callback<Ret, T, A, B>::CB CB;

	static void call(T* wnd, Msg& m, const CB& handler)
	{
		A id= static_cast<A>(m.wparam);
		B draw_item= reinterpret_cast<B>(m.lparam);
		handler(wnd, id, draw_item);
	}
};


template<class T>
struct msg_translator<WM_MEASUREITEM, T>
{
	typedef void Ret;
	typedef int A;
	typedef MEASUREITEMSTRUCT* B;
	typedef typename msg_callback<Ret, T, A, B>::CB CB;

	static void call(T* wnd, Msg& m, const CB& handler)
	{
		A id= static_cast<A>(m.wparam);
		B measure_item= reinterpret_cast<B>(m.lparam);
		handler(wnd, id, measure_item);
	}
};


template<class T>
struct msg_translator<WM_LBUTTONDOWN, T>
{
	typedef void Ret;
	typedef UINT A;
	typedef CPoint B;
	typedef typename msg_callback<Ret, T, A, B>::CB CB;

	static void call(T* wnd, Msg& m, const CB& handler)
	{
		A flags= static_cast<A>(m.wparam);
		B pt= PointFromLParam(m.lparam);
		handler(wnd, flags, pt);
	}
};


template<class T>
struct msg_translator<WM_LBUTTONUP, T> : public msg_translator<WM_LBUTTONDOWN, T>
{};

template<class T>
struct msg_translator<WM_MOUSEMOVE, T> : public msg_translator<WM_LBUTTONDOWN, T>
{};


template<class T>
struct msg_translator<WM_TIMER, T>
{
	typedef void Ret;
	typedef UINT_PTR A;
	typedef typename msg_callback<Ret, T, A>::CB CB;

	static void call(T* wnd, Msg& m, const CB& handler)
	{
		A event_id= static_cast<A>(m.wparam);
		handler(wnd, event_id);
	}
};


template<class T>
struct msg_translator<WM_SETCURSOR, T>
{
	typedef BOOL Ret;
	typedef CWnd* A;
	typedef UINT B;
	typedef UINT C;
	typedef typename msg_callback<Ret, T, A, B, C>::CB CB;

	static void call(T* wnd, Msg& m, const CB& handler)
	{
		A w= CWnd::FromHandle(reinterpret_cast<HWND>(m.wparam));
		B hit_test= LOWORD(lparam);
		C msg= HIWORD(lparam);
		handler(wnd, w, hit_test, msg);
	}
};



} // namespace


#define _UPCAST_WND_(w)		boost::bind(mcr::up_cast_t<ThisClass*, mcr::CCmdTarget*>(), (w))

#define _MSG_TRANS_1234_(msg, method) \
.add((msg), mcr::msg_translator<(msg), mcr::CCmdTarget>::CB(boost::bind(&ThisClass::method, _UPCAST_WND_(_1), _2, _3, _4)))

#define _MSG_TRANS_123_(msg, method) \
.add((msg), mcr::msg_translator<(msg), mcr::CCmdTarget>::CB(boost::bind(&ThisClass::method, _UPCAST_WND_(_1), _2, _3)))

#define _MSG_TRANS_12_(msg, method) \
.add((msg), mcr::msg_translator<(msg), mcr::CCmdTarget>::CB(boost::bind(&ThisClass::method, _UPCAST_WND_(_1), _2)))

#define _MSG_TRANS_1_(msg, method) \
.add((msg), mcr::msg_translator<(msg), mcr::CCmdTarget>::CB(boost::bind(&ThisClass::method, _UPCAST_WND_(_1))))



#define ON_WM_SIZE()		_MSG_TRANS_1234_(WM_SIZE, OnSize)
#define ON_WM_PAINT()		_MSG_TRANS_1_(WM_PAINT, OnPaint)
#define ON_WM_ERASEBKGND()	_MSG_TRANS_12_(WM_ERASEBKGND, OnEraseBkgnd)
#define ON_WM_DRAWITEM()	_MSG_TRANS_123_(WM_DRAWITEM, OnDrawItem)
#define ON_WM_MEASUREITEM()	_MSG_TRANS_123_(WM_MEASUREITEM, OnMeasureItem)
#define ON_WM_INITDIALOG()	_MSG_TRANS_123_(WM_INITDIALOG, HandleInitDialog)
#define ON_WM_NCDESTROY()	_MSG_TRANS_1_(WM_NCDESTROY, OnNcDestroy)
#define ON_WM_LBUTTONUP()	_MSG_TRANS_123_(WM_LBUTTONUP, OnLButtonUp)
#define ON_WM_LBUTTONDOWN()	_MSG_TRANS_123_(WM_LBUTTONDOWN, OnLButtonDown)
#define ON_WM_MOUSEMOVE()	_MSG_TRANS_123_(WM_MOUSEMOVE, OnMouseMove)
#define ON_WM_TIMER()		_MSG_TRANS_12_(WM_TIMER, OnTimer)
#define ON_WM_SETCURSOR()	_MSG_TRANS_1234_(WM_SETCURSOR, OnSetCursor)


#define ON_MESSAGE(msg, fn) \
.add((msg), mcr::GenericMessageCallback(boost::bind((fn), _UPCAST_WND_(_1), _2, _3)), mcr::FN_SIG_GENERIC_MESSAGE_CALLBACK)

//#define ON_MESSAGE_VOID(msg, fn)
//.add((msg), mcr::GenericMessageVoidCallback(boost::bind(fn, _UPCAST_WND_(_1), _2, _3)), mcr::FN_SIG_)


#define ON_CONTROL(notifyCode, id, fn) \
.add_cmd((notifyCode), (id), (id), mcr::msg_translator<WM_COMMAND, mcr::CCmdTarget>::CB(boost::bind((fn), _UPCAST_WND_(_1))))


#define ON_COMMAND_RANGE(id_first, id_last, fn) \
.add_cmd(mcr::NC_COMMAND, (id_first), (id_last), \
	mcr::msg_translator<WM_COMMAND, mcr::CCmdTarget>::CB(boost::bind((fn), \
	_UPCAST_WND_(_1))))


#define ON_COMMAND(id, fn)	ON_COMMAND_RANGE((id), (id), (fn))


#define ON_COMMAND_EX(id, fn) \
.add_cmd(mcr::NC_COMMAND, (id_first), (id_last), \
	mcr::msg_translator<WM_COMMAND, mcr::CCmdTarget, mcr::FN_SIG_BOOL_UINT>::CB(boost::bind((fn), \
	_UPCAST_WND_(_1))), FN_SIG_BOOL_UINT)


#define ON_UPDATE_COMMAND_UI(id, fn) \
.add(mcr::WM_UPDATE_COMMAND_UI, (id), (id), \
	mcr::msg_translator<mcr::WM_UPDATE_COMMAND_UI, mcr::CCmdTarget>::CB(boost::bind((fn), \
	_UPCAST_WND_(_1))))


#define ON_UPDATE_COMMAND_UI_RANGE(id_from, id_to, fn) \
.add(mcr::WM_UPDATE_COMMAND_UI, (id), (id), \
	mcr::msg_translator<mcr::WM_UPDATE_COMMAND_UI, mcr::CCmdTarget, mcr::FN_SIG_BOOL_UINT>::CB(boost::bind((fn), \
	_UPCAST_WND_(_1))), FN_SIG_BOOL_UINT)


#define ON_NOTIFY_REFLECT(notification_code, fn) \
.add(WM_NOTIFY | mcr::WM_REFLECT_FLAG, (notification_code), 0, 0, mcr::msg_translator<WM_NOTIFY, mcr::CCmdTarget>::CB(boost::bind((fn), _UPCAST_WND_(_1), _2, _3)))


#define ON_NOTIFY_RANGE(notification_code, id_first, id_last, fn) \
.add(WM_NOTIFY, (notification_code), (id_first), (id_last), mcr::msg_translator<WM_NOTIFY, mcr::CCmdTarget>::CB(boost::bind((fn), _UPCAST_WND_(_1), _2, _3)))


#define ON_NOTIFY(notification_code, id, fn)	ON_NOTIFY_RANGE((notification_code), (id), (id), (fn))


#define ON_NOTIFY_EX_RANGE(notification_code, id_first, id_last, fn) \
.add(WM_NOTIFY, (notification_code), (id_first), (id_last), mcr::msg_translator<WM_NOTIFY, mcr::CCmdTarget, mcr::FN_SIG_BOOL_UINT_NMHDR_RES>::CB(boost::bind((fn), _UPCAST_WND_(_1), _2, _3, _4)), FN_SIG_BOOL_UINT_NMHDR_RES)


#define ON_NOTIFY_EX(notification_code, id, fn)		ON_NOTIFY_EX_RANGE((notification_code), (id), (id), (fn))


#define ON_EN_CHANGE(id, fn)			ON_CONTROL(EN_CHANGE, (id), (fn))
#define ON_BN_CLICKED(id, fn)			ON_CONTROL(BN_CLICKED, (id), (fn))
#define ON_LBN_SELCHANGE(id,fn)			ON_CONTROL(LBN_SELCHANGE, (id), (fn))
#endif
