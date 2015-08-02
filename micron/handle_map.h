#pragma once
#include "basetypes.h"


// This is wrapper class creation function template; specialize it to create derived classes for a handle
// instead of simple 'T'; use abstract factory for that purpose
template<class T, class H>
T* create_temp_object(H h)
{
	// create temp object "pointing" to handle 'h'; this wrapper doesn't own the real object behind 'h'
	// nor ever be elevated to the owner status
	T* t= new T();
	t->set_handle(h);
	return t;
}


// Make temporary wrapper forget about real object, so it won't destruct it when it is itself deleted;
// temporary wrappers invoke this function before being disposed of
template<class T>
void forget_object(T* t)
{
	if (t)
		t->Detach();
}

/*
// Temporary wrapper is held in this structure that aids with its proper creation and destruction
// (it's an not ideal solution as it costs extra allocation for this temp_obj)
template<class T, class H>
struct temp_obj : boost::noncopyable
{
	temp_obj(H h) : t_(create_temp_object<T, H>(h))
	{}

	T* obj() const		{ return t_; }

	~temp_obj()
	{
		try
		{
			forget_object(t_);
			delete t_;
		}
		catch (std::exception&)
		{
			ASSERT(false);
		}
	}

private:
	T* t_;
};
*/

// Handle map template: map between Windows handles (HWND, HDC, HFONT, etc.) and C++ wrapper objects.
// Permanent map is simply a map between handles and corresponding objects created explicitly by programmer;
// programmer owns those objects.
// Temporary map is used when handle doesn't have corresponding wrapper crated; a temporary one will be created for
// such handles and placed in a temporary map. This map owns wrapper objects it contains, but not real counterparts
// identified by handles. It is periodically emptied deleting wrappers only (detaching them prior to deletion)
//
template<class H, class T>	// handle and its wrapper class
class handle_map
{
public:
	// add mapping to the permanent object
	void add_mapping_to_permanent_obj(H h, T* t)
	{
		ASSERT(find_permanent(h) == 0);
		permanent_[h] = t;
	}

	//	Map::iterator it= temporary_.find(h);
	//	if (it != temporary_.end())
	//		permanent_.transfer(it, temporary_);
	//	else
	//		permanent_.insert(h, create_object<T>(h, true));

	//	return 0;
	//}

	// add temporary wrapper 't' for handle 'h' and place it in the map
	void add_mapping_to_temporary_obj(T* t, H h)
	{
		ASSERT(find_temporary(h) == 0);
		ASSERT(find_permanent(h) == 0);

//		Temp* temp= new Temp(h);
//		temp->obj()->set_handle(h);
		temporary_.insert(h, t);	// (exception safe as long as h is POD or doesn't throw on copy)
		// temporary_ map owns 't' from now on

		return;
	}

	void remove_permanent_mapping(H h)
	{
		permanent_.erase(h);
	}

	T* find(H h) const
	{
		// look for a permanent object first, and only them check temps (both may exists, so order is important here)
		T* t= find_permanent(h);
		return t ? t : find_temporary(h);
	}

	T* find_permanent(H h) const
	{
		PermMap::const_iterator it= permanent_.find(h);
		return it == permanent_.end() ? 0 : const_cast<T*>(it->second);
	}

	T* find_temporary(H h) const
	{
		TempMap::const_iterator it= temporary_.find(h);
		return it == temporary_.end() ? 0 : const_cast<T*>(it->second);
	}

	void delete_temps()
	{
		temporary_.clear();
	}

private:
	//typedef temp_obj<T, H> Temp;
	// big diffrence here: temp map claims ownership of its wrapper objects
	typedef boost::ptr_unordered_map<H, T> TempMap;
	// permanent map merely maps handles to objects
	typedef boost::unordered_map<H, T*> PermMap;

	TempMap temporary_;
	PermMap permanent_;
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class HM>
HM* get_handle_map()
{
//	static boost::thread_specific_ptr<HM> g_wnd_map;//(new HM());
//	static __declspec(thread) std::auto_ptr<HM> wnd_map(new HM());
	static __declspec(thread) HM* hm= 0;

//	if (g_wnd_map.get() == 0)
//		g_wnd_map.reset(new HM());
	if (hm == 0)
		hm = new HM();

	return hm;
}



template<class T, class H>
class handle_mixin
{
	H handle_;
	bool owner_;
public:
	handle_mixin() : handle_(H()), owner_(true)
	{}

	typedef handle_map<H, T> handle_map_t;

	H get() const	{ return handle_; }

	void set(H h)	{ handle_ = h; owner_ = true; }

	void set_temp(H h)	{ handle_ = h; owner_ = false; }

	bool has_ownership()	{ return owner_ && handle_ != 0; }

//	void set_ownership(bool own)	{ owner_ = own; }
//	bool ownership() const		{ return owner_; }

	bool attach(T* t, H h)
	{
		ASSERT(this->get() == 0, L"empty wrapper object expected");
		ASSERT(h != 0, L"non-empty handle expected to attach");

		if (this->get() || h == 0)
			return false;

		if (handle_map_t* map= get_handle_map<handle_map_t>())
			map->add_mapping_to_permanent_obj(h, t);

		this->set(h);

		return true;
	}

	H detach()
	{
		H h= this->get();

		if (h)
			if (handle_map_t* map= ::get_handle_map<handle_map_t>())
				map->remove_permanent_mapping(h);

		this->set(0);

		return h;
	}

	// find corresponding permanent object
	static T* find_permanent(H h)
	{
		if (handle_map_t* map= ::get_handle_map<handle_map_t>())
			return map->find_permanent(h);
		else
			return 0;
	}

	// try to find a wrapper corresponding to the handle 'h', or create one if needed
	template<class F>
	static T* find_or_create(H h, F create_temp)
	{
		if (handle_map_t* map= ::get_handle_map<handle_map_t>())
		{
			if (T* t= map->find(h))
				return t;

			T* t= create_temp(h);
			map->add_mapping_to_temporary_obj(t, h);
			return t;
		}
		else
			throw mcr::micron_exception("cannot obtain handle map");
	}
};
