#pragma once

namespace mcr {



// extra 'messages' & reflected messages
enum
{
	WM_REFLECT_FLAG=		0x10000,
	WM_UPDATE_COMMAND_UI=	0x20000
};

// function signatures
enum MsgFunctionSignature
{
	FN_SIG_DEFAULT= 0,			// message-specific default
	FN_SIG_GENERIC_MESSAGE_CALLBACK,
	FN_SIG_BOOL_UINT,			// BOOL (CCmdTarget*, UINT)
	FN_SIG_VOID_UINT,			// void (CCmdTarget*, UINT)
	FN_SIG_BOOL_UINT_NMHDR_RES,	// BOOL (CCmdTarget*, UINT, NMHDR*, LRESULT*)
};

// notification codes
enum
{
	NC_COMMAND= 0,				// 'notification' code for menu or accellerator
	//NC_UPDATE_COMMAND_UI= 0-1,	//
};


class CCmdUI;


struct Msg
{
	Msg(UINT msg, WPARAM wparam, LPARAM lparam) : msg(msg), wparam(wparam), lparam(lparam), result(0)
	{}

	UINT msg;
	WPARAM wparam;
	LPARAM lparam;
	LRESULT result;
};


// currently callback (handlers) are stored in boost:any variant
typedef boost::any any_callback;


// message handlers are stored in a map; each entry in message map consists of below struct:
struct message_map_entry
{
	message_map_entry(uint32 msg, uint32 code, uint16 id) : msg(msg), signature(0), reserved(0), code(code), ctrl_id(id), ctrl_last(id)
	{}

	message_map_entry(uint32 msg, uint32 code, uint16 id_first, uint16 id_last, const any_callback& handler, MsgFunctionSignature fn_sig) : msg(msg), signature(fn_sig), reserved(0), code(code), ctrl_id(id_first), ctrl_last(id_last), handler(handler)
	{}

	uint32 msg;				// window message
	uint16 signature;		// fn signature if needed
	uint16 reserved;		// padding
	uint32 code;			// notification code
	uint16 ctrl_id;			// range of ctr ids: from
	uint16 ctrl_last;		// ..to
	any_callback handler;	// handler callback
};


// comparison functor to arrange message handlers
struct message_map_entry_less : std::binary_function<const message_map_entry&, const message_map_entry&, bool>
{
	bool operator () (const message_map_entry& a, const message_map_entry& b)
	{
		if (a.msg != b.msg)
			return a.msg < b.msg;
		if (a.code != b.code)
			return a.code < b.code;
		if (a.ctrl_id != b.ctrl_id)
			return a.ctrl_id < b.ctrl_id;
		return a.ctrl_last < b.ctrl_last;
		// note: fn signature is ignored
	}
};

// comparison functor ignoring ctrl ids
struct message_map_entry_less_ex : std::binary_function<const message_map_entry&, const message_map_entry&, bool>
{
	bool operator () (const message_map_entry& a, const message_map_entry& b)
	{
		if (a.msg != b.msg)
			return a.msg < b.msg;
		return a.code < b.code;
	}
};


inline void swap(message_map_entry& a, message_map_entry& b)
{
	std::swap(a.msg, b.msg);
	std::swap(a.signature, b.signature);
	std::swap(a.reserved, b.reserved);
	std::swap(a.code, b.code);
	std::swap(a.ctrl_id, b.ctrl_id);
	std::swap(a.ctrl_last, b.ctrl_last);
	std::swap(a.handler, b.handler);
}


#define _UPCAST_(w)		boost::bind(mcr::up_cast_t<T*, CCmdTarget*>(), (w))

namespace detail {

template<int ARITY, class T, class F, class CB>
struct callback_binder
{};

template<class T, class F, class CB>
struct callback_binder<4, T, F, CB>
{
	static any_callback cb(const F& fn)		{ return CB(boost::bind(fn, _UPCAST_(_1), _2, _3, _4)); }
};

template<class T, class F, class CB>
struct callback_binder<3, T, F, CB>
{
	static any_callback cb(const F& fn)		{ return CB(boost::bind(fn, _UPCAST_(_1), _2, _3)); }
};

template<class T, class F, class CB>
struct callback_binder<2, T, F, CB>
{
	static any_callback cb(const F& fn)		{ return CB(boost::bind(fn, _UPCAST_(_1), _2)); }
};

template<class T, class F, class CB>
struct callback_binder<1, T, F, CB>
{
	static any_callback cb(const F& fn)		{ return CB(boost::bind(fn, _UPCAST_(_1))); }
};

} // namespace

#undef _UPCAST_


// sorted vector of message handlers
class message_map_container
{
public:
	message_map_container()
	{}

	// add message handler 'f' for message WM to the map; T is the type of f's first param
	template<int WM, class T, class F>
	message_map_container& add_(const F& f)
	{
		typedef msg_translator<WM, CCmdTarget> MT;
		return add(WM, detail::callback_binder<MT::CB::arity, T, F, MT::CB>::cb(f));
	}

	template<int WM, class T, class F>
	message_map_container& add_(const F& f, uint32 code, uint16 id_from, uint16 id_to)
	{
		typedef msg_translator<WM, CCmdTarget> MT;
		return add(WM, code, id_from, id_to, detail::callback_binder<MT::CB::arity, T, F, MT::CB>::cb(f), FN_SIG_DEFAULT);
	}

	template<int WM, class T, MsgFunctionSignature S, class F>
	message_map_container& add_(const F& f, uint32 code, uint16 id_from, uint16 id_to)
	{
		typedef msg_translator<WM, CCmdTarget, S> MT;
		return add(WM, code, id_from, id_to, detail::callback_binder<MT::CB::arity, T, F, MT::CB>::cb(f), S);
	}

	template<class CB, class T, class F>
	message_map_container& add_(int msg, const F& f, uint32 code, uint16 id_from, uint16 id_to, MsgFunctionSignature fn_sig)
	{
		return add(msg, code, id_from, id_to, detail::callback_binder<CB::arity, T, F, CB>::cb(f), fn_sig);
	}

	template<int WM, class T, MsgFunctionSignature S, class F>
	message_map_container& reflect_(const F& f, uint32 code, uint16 id_from, uint16 id_to)
	{
		typedef msg_translator<WM, CCmdTarget, S> MT;
		return add_(WM | WM_REFLECT_FLAG, code, id_from, id_to, detail::callback_binder<MT::CB::arity, T, F, MT::CB>::cb(f), S);
	}

	template<int WM, class T, class F>
	message_map_container& reflect_(const F& f, uint32 code, uint16 id_from, uint16 id_to)
	{
		typedef msg_translator<WM, CCmdTarget> MT;
		return add_(WM | WM_REFLECT_FLAG, code, id_from, id_to, detail::callback_binder<MT::CB::arity, T, F, MT::CB>::cb(f), FN_SIG_DEFAULT);
	}

	// add message handler to the map
	message_map_container& add(uint32 msg, const any_callback& handler)
	{
		return add(message_map_entry(msg, 0u, 0u, 0u, handler, FN_SIG_DEFAULT));
	}

	message_map_container& add(uint32 msg, uint32 code, const any_callback& handler)
	{
		return add(message_map_entry(msg, code, 0u, 0u, handler, FN_SIG_DEFAULT));
	}

	message_map_container& add(uint32 msg, uint32 code, uint16 id, uint16 id_to, const any_callback& handler, MsgFunctionSignature fn_sig= FN_SIG_DEFAULT)
	{
		return add(message_map_entry(msg, code, id, id_to, handler, fn_sig));
	}

	// ditto
	message_map_container& add(const message_map_entry& e)
	{
		// keep entries sorted
		v_.insert(std::lower_bound(v_.begin(), v_.end(), e, message_map_entry_less()), e);
		return *this;
	}

	// add handler for WM_COMMAND (either command or notification)
	message_map_container& add_cmd(uint32 code, uint16 id_from, uint16 id_to, const any_callback& handler, MsgFunctionSignature fn_sig= FN_SIG_DEFAULT)
	{
		return add(message_map_entry(WM_COMMAND, code, id_from, id_to, handler, fn_sig));
	}

	// binary search for message handler
	const message_map_entry* find(uint32 msg, uint32 code) const
	{
		message_map_entry m(msg, code, 0u);
		std::pair<C::const_iterator, C::const_iterator> p= std::equal_range(v_.begin(), v_.end(), m, message_map_entry_less());
		return p.first != p.second ? &(*p.first) : 0;
	}

	// binary serach for a message handler for a given control id
	const message_map_entry* find(uint32 msg, uint32 code, uint16 id) const
	{
		// first find all handlers for given combination (there may be many)
		message_map_entry m(msg, code, id);
		std::pair<C::const_iterator, C::const_iterator> p= std::equal_range(v_.begin(), v_.end(), m, message_map_entry_less_ex());
		if (p.first == p.second)
			return 0;

		if (std::distance(p.first, p.second) == 1)
		{
			if (p.first->ctrl_id <= id && id <= p.first->ctrl_last)
				return &(*p.first);	// there's only one handler and it matches id
			else
				return 0;	// id mismatch
		}

		// here we have multiple handlers (some may handle a range of ids)
		// check if there's one matching id exactly
		std::pair<C::const_iterator, C::const_iterator> match= std::equal_range(p.first, p.second, m, message_map_entry_less());

		if (match.first != match.second)
			return &(*match.first);	// return exact match for id

		// check messages that handle ranges of ids
		for (C::const_iterator it= p.first; it != p.second; ++it)
		{
			if (it->ctrl_id > id)
				return 0;
			if (it->ctrl_last > id)
				return &(*it);
		}

		return 0;
	}

private:
	typedef std::vector<message_map_entry> C;
	C v_;
};

/*
template<int WM, class T>
struct add_msg_handler_t
{
	add_msg_handler_t(message_map_container& c) : c_(c)
	{}

	message_map_container& operator () ()
	{
		typedef msg_translator<WM, CCmdTarget> MT;
		typedef MT::Ret (T::*FN)(MT::A, MT::B, MT::C);

		FN f= MT::DefaultHandler<T>();

		return c_.add(WM, callback_t<FN, MT::CB, T, MT::A, MT::B, MT::C>::cb(f));
	}

	template <class FN>
	message_map_container& operator () (const FN& f)
	{
		typedef msg_translator<WM, CCmdTarget> MT;

		return c_.add(WM, callback_t<FN, MT::CB, T, MT::A, MT::B, MT::C>::cb(f));
	}

private:
	message_map_container& c_;
//	const F& f_;
};


template<int WM, class T>
add_msg_handler_t<WM, T> add_msg_handler(message_map_container& mmc)
{
	return add_msg_handler_t<WM, T>(mmc);
}
*/

struct message_map_struct
{
	const message_map_struct* (*get_base_map)();
	const message_map_container& entries;
};


} // namespace


#define DECLARE_MESSAGE_MAP() \
protected: \
	static const mcr::message_map_struct* GetThisMessageMap(); \
	virtual const mcr::message_map_struct* GetMessageMap() const;


#define BEGIN_MESSAGE_MAP(theClass, baseClass) \
	const mcr::message_map_struct* theClass::GetMessageMap() const { return GetThisMessageMap(); } \
	const mcr::message_map_struct* theClass::GetThisMessageMap() \
	{ \
		typedef theClass ThisClass;						\
		typedef baseClass TheBaseClass;					\
		static mcr::message_map_container entries;		\
		static bool initialized= false;					\
		if (!initialized)								\
		{												\
			entries


#define END_MESSAGE_MAP()	;		\
			initialized = true;		\
		}							\
		static const mcr::message_map_struct msg_map= { &TheBaseClass::GetThisMessageMap, entries }; \
		return &msg_map;			\
	}




#if 0
#pragma once

namespace mcr {



// extra 'messages' & reflected messages
enum
{
	WM_REFLECT_FLAG=		0x10000,
	WM_UPDATE_COMMAND_UI=	0x20000
};

// function signatures
enum MsgFunctionSignature
{
	FN_SIG_DEFAULT= 0,			// message-specific default
	FN_SIG_BOOL_UINT,			// BOOL (CCmdTarget*, UINT)
	FN_SIG_BOOL_UINT_NMHDR_RES,	// BOOL (CCmdTarget*, UINT, NMHDR*, LRESULT*)
	FN_SIG_GENERIC_MESSAGE_CALLBACK		// ON_MESSAGE
};

// notification codes
enum
{
	NC_COMMAND= 0,				// 'notification' code for menu or accellerator
	//NC_UPDATE_COMMAND_UI= 0-1,	//
};


class CCmdUI;


struct Msg
{
	Msg(UINT msg, WPARAM wparam, LPARAM lparam) : msg(msg), wparam(wparam), lparam(lparam), result(0)
	{}

	UINT msg;
	WPARAM wparam;
	LPARAM lparam;
	LRESULT result;
};


// currently callback (handlers) are stored in boost:any variant
typedef boost::any any_callback;


// message handlers are stored in a map; each entry in message map consists of below struct:
struct message_map_entry
{
	message_map_entry(uint32 msg, uint32 code, uint16 id) : msg(msg), signature(0), reserved(0), code(code), ctrl_id(id), ctrl_last(id)
	{}

	message_map_entry(uint32 msg, uint32 code, uint16 id_first, uint16 id_last, const any_callback& handler, MsgFunctionSignature fn_sig) : msg(msg), signature(fn_sig), reserved(0), code(code), ctrl_id(id_first), ctrl_last(id_last), handler(handler)
	{}

	uint32 msg;				// window message
	uint16 signature;		// fn signature if needed
	uint16 reserved;		// padding
	uint32 code;			// notification code
	uint16 ctrl_id;			// range of ctr ids: from
	uint16 ctrl_last;		// ..to
	any_callback handler;	// handler callback
};


// comparison functor to arrange message handlers
struct message_map_entry_less : std::binary_function<const message_map_entry&, const message_map_entry&, bool>
{
	bool operator () (const message_map_entry& a, const message_map_entry& b)
	{
		if (a.msg != b.msg)
			return a.msg < b.msg;
		if (a.code != b.code)
			return a.code < b.code;
		if (a.ctrl_id != b.ctrl_id)
			return a.ctrl_id < b.ctrl_id;
		return a.ctrl_last < b.ctrl_last;
		// note: fn signature is ignored
	}
};

// comparison functor ignoring ctrl ids
struct message_map_entry_less_ex : std::binary_function<const message_map_entry&, const message_map_entry&, bool>
{
	bool operator () (const message_map_entry& a, const message_map_entry& b)
	{
		if (a.msg != b.msg)
			return a.msg < b.msg;
		return a.code < b.code;
	}
};


inline void swap(message_map_entry& a, message_map_entry& b)
{
	std::swap(a.msg, b.msg);
	std::swap(a.signature, b.signature);
	std::swap(a.reserved, b.reserved);
	std::swap(a.code, b.code);
	std::swap(a.ctrl_id, b.ctrl_id);
	std::swap(a.ctrl_last, b.ctrl_last);
	std::swap(a.handler, b.handler);
}


// sorted vector of message handlers
class message_map_container
{
public:
	message_map_container()
	{}

	// add message handler to the map
	message_map_container& add(uint32 msg, const any_callback& handler, MsgFunctionSignature fn_sig= FN_SIG_DEFAULT)
	{
		return add(message_map_entry(msg, 0u, 0u, 0u, handler, fn_sig));
	}

	message_map_container& add(uint32 msg, uint32 code, const any_callback& handler)
	{
		return add(message_map_entry(msg, code, 0u, 0u, handler, FN_SIG_DEFAULT));
	}

	message_map_container& add(uint32 msg, uint32 code, uint16 id, uint16 id_to, const any_callback& handler, MsgFunctionSignature fn_sig= FN_SIG_DEFAULT)
	{
		return add(message_map_entry(msg, code, id, id_to, handler, fn_sig));
	}

	// ditto
	message_map_container& add(const message_map_entry& e)
	{
		// keep entries sorted
		v_.insert(std::lower_bound(v_.begin(), v_.end(), e, message_map_entry_less()), e);
		return *this;
	}

	// add handler for WM_COMMAND (either command or notification)
	message_map_container& add_cmd(uint32 code, uint16 id_from, uint16 id_to, const any_callback& handler, MsgFunctionSignature fn_sig= FN_SIG_DEFAULT)
	{
		return add(message_map_entry(WM_COMMAND, code, id_from, id_to, handler, fn_sig));
	}

	// binary search for message handler
	const message_map_entry* find(uint32 msg, uint32 code) const
	{
		message_map_entry m(msg, code, 0u);
		std::pair<C::const_iterator, C::const_iterator> p= std::equal_range(v_.begin(), v_.end(), m, message_map_entry_less());
		return p.first != p.second ? &(*p.first) : 0;
	}

	// binary serach for a message handler for a given control id
	const message_map_entry* find(uint32 msg, uint32 code, uint16 id) const
	{
		// first find all handlers for given combination (there may be many)
		message_map_entry m(msg, code, id);
		std::pair<C::const_iterator, C::const_iterator> p= std::equal_range(v_.begin(), v_.end(), m, message_map_entry_less_ex());
		if (p.first == p.second)
			return 0;

		if (std::distance(p.first, p.second) == 1)
		{
			if (p.first->ctrl_id <= id && id <= p.first->ctrl_last)
				return &(*p.first);	// there's only one handler and it matches id
			else
				return 0;	// id mismatch
		}

		// here we have multiple handlers (some may handle a range of ids)
		// check if there's one matching id exactly
		std::pair<C::const_iterator, C::const_iterator> match= std::equal_range(p.first, p.second, m, message_map_entry_less());

		if (match.first != match.second)
			return &(*match.first);	// return exact match for id

		// check messages that handle ranges of ids
		for (C::const_iterator it= p.first; it != p.second; ++it)
		{
			if (it->ctrl_id > id)
				return 0;
			if (it->ctrl_last > id)
				return &(*it);
		}

		return 0;
	}

private:
	typedef std::vector<message_map_entry> C;
	C v_;
};


struct message_map_struct
{
	const message_map_struct* (*get_base_map)();
	const message_map_container& entries;
};


} // namespace


#define DECLARE_MESSAGE_MAP() \
protected: \
	static const mcr::message_map_struct* GetThisMessageMap(); \
	virtual const mcr::message_map_struct* GetMessageMap() const;


#define BEGIN_MESSAGE_MAP(theClass, baseClass) \
	const mcr::message_map_struct* theClass::GetMessageMap() const { return GetThisMessageMap(); } \
	const mcr::message_map_struct* theClass::GetThisMessageMap() \
	{ \
		typedef theClass ThisClass;						\
		typedef baseClass TheBaseClass;					\
		static mcr::message_map_container entries;		\
		static bool initialized= false;					\
		if (!initialized)								\
		{												\
			entries


#define END_MESSAGE_MAP()	;		\
			initialized = true;		\
		}							\
		static const mcr::message_map_struct msg_map= { &TheBaseClass::GetThisMessageMap, entries }; \
		return &msg_map;			\
	}
#endif
