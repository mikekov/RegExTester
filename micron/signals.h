#pragma once


namespace mcr
{

typedef boost::signals2::scoped_connection auto_connection;
typedef boost::signals2::connection slot_connection;


//typedef boost::signal<void ()> OnChanged;	// collection has been modified


template <typename F>
class signal : public boost::signals2::signal<F>
{
public:
	typedef typename boost::signals2::signal<F>::slot_type fn;

	//slot_connection operator += (const fn& s)
	//{ return this->connect(s); }
};


typedef signal<void ()> Signal;


} // namespace
