#pragma once

template <const int Size=32>
class stl_lambda0
{
public:
	typedef stl_lambda0 SelfType;
	template <class T> stl_lambda0(const T& o): copy_destroy_func(0) { Set(o); }
	stl_lambda0() { copy_destroy_func = 0; }
	~stl_lambda0() { if(copy_destroy_func) (*copy_destroy_func)(this, 0); }
	stl_lambda0(const SelfType& o) { copy_destroy_func = 0; if(o.copy_destroy_func) (*o.copy_destroy_func)(this, (SelfType*)&o); }
	stl_lambda0& operator = (const SelfType& o) { if (copy_destroy_func) (*copy_destroy_func)(this, 0); if (o.copy_destroy_func) (o.copy_destroy_func)(this, (SelfType*)&o); else copy_destroy_func = 0; return *this; }
	bool operator == (const SelfType& o) const { if (copy_destroy_func == 0 && o.copy_destroy_func == 0) return true; else return false; }
	void operator ()() const { if (copy_destroy_func == nullptr) return; (*exec_func)((SelfType*)this); }

	template <class T> void Set(const T& data)
	{
		static_assert(sizeof(T) <= sizeof(buffer), "lambda won't fit");

		if(copy_destroy_func) 
			(*copy_destroy_func)(this,0); 
		::new ((T*)buffer) T(data); 
		copy_destroy_func = _copy_destroy_impl<T>;
		exec_func = _exec_impl<T>;
	}

	void (*copy_destroy_func)(SelfType* obj, SelfType* oobj);
	void (*exec_func)(SelfType* obj);

protected:
	char buffer[Size];
	template <class T> static void _copy_destroy_impl(SelfType* obj, SelfType* oobj)
	{
		if(oobj == 0) 
			(reinterpret_cast<T*>(obj->buffer))->~T();
		else
			obj->Set(*(T*)oobj->buffer);
	}
	template <class T> static void _exec_impl(SelfType* obj) { (*(reinterpret_cast<T*>(obj->buffer)))(); }
};