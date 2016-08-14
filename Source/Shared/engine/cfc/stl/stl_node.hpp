#pragma once

template <class T>
class stl_node
{
public:
	stl_node() : m_prev(nullptr), m_next(nullptr) {}
	stl_node(const T& v) : m_data(v), m_prev(nullptr), m_next(nullptr) {}
	~stl_node() { remove(); }
	stl_node(const stl_node& o) : m_data(o.m_data), m_next(nullptr), m_prev(nullptr) { ((stl_node&)o).add(*this); }
	stl_node& operator = (const stl_node& o) { m_data = o.m_data; ((stl_node&)o).add(*this); return *this; }
	stl_node& operator = (const T& o) { m_data = o; return *this; }
	operator T&() { return m_data; }
	operator const T&() const { return m_data; }

	stl_node& operator += (stl_node& next) { add(next); return next; }

	bool add(stl_node& next)
	{
		if (next.m_prev != nullptr && next.m_next != nullptr)
			return false; // check if already in a chain - need to remove from chain first.

		next.m_prev = this;
		next.m_next = m_next;
		if (m_next)
			m_next->m_prev = &next;
		m_next = &next;

		return true;
	}
	void remove()
	{
		if (m_prev)
			m_prev->m_next = m_next;
		if (m_next)
			m_next->m_prev = m_prev;
		m_prev = nullptr;
		m_next = nullptr;
	}
	stl_node* next() const { return m_next; }
	stl_node* previous() const { return m_prev; }

	T& get() { return m_data; }
	const T& get() const { return m_data; }
protected:
	T m_data;
	stl_node* m_prev;
	stl_node* m_next;
};

class stl_node_helpers
{
public:
	template <class T, class T2> static void foreach(T& root, const T2& iterateFunction) 
	{
		T* current = &root; 
		T* next; 
		while (current != nullptr) 
		{ 
			next = current->next(); 
			iterateFunction(*current);
			current = next; 
		} 
	}
};