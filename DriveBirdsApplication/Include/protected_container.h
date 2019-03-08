#pragma once
#if !defined __GLOBAL_H__
#define __GLOBAL_H__

#include "includes.h"

using namespace std;

class PROTECT_CONTAINER
{
public:
	PROTECT_CONTAINER()
	{
		mutex = ::CreateMutex(NULL, false, NULL); 
	};
	
	~PROTECT_CONTAINER()
	{
		CloseHandle(mutex);
	};

private:
	HANDLE	mutex;

public:
	bool enterExplicitProtectedMode(int interval = INFINITE)
	{
		if(::WaitForSingleObject(mutex, interval) != WAIT_OBJECT_0)
			return false;

		return true;
	};

	void leaveExplicitProtectedMode()
	{
		::ReleaseMutex(mutex);
	};
};

template <typename T>
class VECTOR_PROTECT_OBJECT : public PROTECT_CONTAINER
{
public:
	VECTOR_PROTECT_OBJECT(){};
	~VECTOR_PROTECT_OBJECT(){};

private:
	vector<T> vecObjects;

public:
	void clear()
	{
		vecObjects.clear();
	};

	bool addtail(T o)
	{
		vecObjects.insert(vecObjects.end(), o);

		return true;
	};

	bool empty()
	{
		return vecObjects.size() == 0;
	};

	size_t size()
	{
		return vecObjects.size();
	};

	bool find(T o)
	{
		for(size_t ii = 0; ii < vecObjects.size(); ii ++)
		{
			if(vecObjects[ii] == o)
				return true;
		}

		return false;
	};

	bool get(size_t ii, T& o)
	{
		if(vecObjects.size() <= ii)
			return false;

		o = vecObjects[ii];

		return true;
	};

	void erase(T o)
	{
        typename vector<T>::iterator it = vecObjects.begin();
			
		while(it != vecObjects.end())
		{
			if(*it != o)
				it ++;
			else
			{
				vecObjects.erase(it);
				break;
			}
		}
	};

	bool resize(int z)
	{
		vecObjects.resize(z);

		return true;
	};

	void clone(vector<T>& v)
	{
		for(size_t ii = 0; ii < vecObjects.size(); ii ++)
			v.insert(v.end(), vecObjects[ii]);
	};

	void copy(vector<T> v)
	{
		vecObjects.clear();

		for(size_t ii = 0; ii < v.size(); ii ++)
			vecObjects.insert(vecObjects.end(), v[ii]);
	};
};

#define protected_vector VECTOR_PROTECT_OBJECT

template <typename T>
class LIST_PROTECT_OBJECT : public PROTECT_CONTAINER
{
public:
	LIST_PROTECT_OBJECT(int max_size = 4096)
	{
		semaphore = ::CreateSemaphore(NULL, 0, max_size, NULL);

		it = listObjects.begin();
	};

	~LIST_PROTECT_OBJECT()
	{
		::CloseHandle(semaphore);
	};

private:
	HANDLE semaphore;

	list<T> listObjects;
	typename list<T>::iterator it;

public:
	void clear()
	{
		listObjects.clear();
		it = listObjects.begin();

		while(WaitForSingleObject(semaphore, 0) == WAIT_OBJECT_0);
	};

	bool addhead(T o)
	{
		listObjects.push_front(o);

		::ReleaseSemaphore(semaphore, 1, NULL);

		return true;
	};

	bool addtail(T o)
	{
		listObjects.push_back(o);

		::ReleaseSemaphore(semaphore, 1, NULL);

		return true;
	};

	bool removehead()
	{
		if(::WaitForSingleObject(semaphore, 0) != WAIT_OBJECT_0)
			return false;

		if(listObjects.empty())
			return false;

		listObjects.pop_front();

		return true;
	};

	bool removehead(T& o)
	{
		if(::WaitForSingleObject(semaphore, 0) != WAIT_OBJECT_0)
			return false;

		if(listObjects.empty())
			return false;

		o = *(listObjects.begin());

		listObjects.pop_front();

		return true;
	};

	bool removetail(T& o)
	{
		if(::WaitForSingleObject(semaphore, 0) != WAIT_OBJECT_0)
			return false;

		if(listObjects.empty())
			return false;

        typename list<T>::iterator it2 = listObjects.begin() ;

		while(it2 != listObjects.end())
		{
			o = *(it2);
			it2 ++;
		}

		listObjects.pop_back();

		return true;
	};

	void erase(T o)
	{
        typename list<T>::iterator it1 = listObjects.begin();
			
		while(it1 != listObjects.end())
		{
			if(*it1 != o)
				it1 ++;
			else
			{
				WaitForSingleObject(semaphore, 0);
				listObjects.erase(it1);
				break;
			}
		}
	};

	void erase()
	{
		if(it != listObjects.end())
		{
			WaitForSingleObject(semaphore, 0);
			listObjects.erase(it);
		}
	};

	bool gethead(T& o)
	{
		if(listObjects.empty())
			return false;

		o = *(listObjects.begin());

		return true;
	};

	bool gettail(T& o)
	{
		if(listObjects.empty())
			return false;

        typename list<T>::iterator it2 = listObjects.begin() ;

		while(it2 != listObjects.end())
		{
			o = *(it2);
			it2 ++;
		}

		return true;
	};

	bool getnext(T& o)
	{
		if(listObjects.empty())
			return false;

		if(it == listObjects.end())
			return false;

		it ++;

		o = (*it);

		return true;
	};

	bool getprev(T& o)
	{
		if(listObjects.empty())
			return false;

		if(it == listObjects.begin())
			return false;

		it --;

		o = (*it);

		return true;
	};

	bool getcurrent(T& o)
	{
		if(listObjects.empty())
			return false;

		o = (*it);

		return true;
	};

	bool moveprev()
	{
		if(listObjects.empty())
			return false;

		if(it == listObjects.begin())
			return false;

		it --;

		return true;
	};

	bool movenext()
	{
		if(listObjects.empty())
			return false;

		if(it == listObjects.end())
			return false;

		it ++;

		return true;
	};

	bool movefirst()
	{
		if(listObjects.empty())
			return false;

		it = listObjects.begin();

		return true;
	};

	bool movelast()
	{
		if(listObjects.empty())
			return false;

		it = listObjects.end() --;
		
		return true;
	};

	bool empty()
	{
		return listObjects.empty();
	};

	bool end()
	{
		return (it == listObjects.end());
	};

	int size()
	{
		return (int)listObjects.size();
	};

	bool find(T o)
	{
		if(listObjects.empty())
			return false;

		it = listObjects.begin();

		while(it != listObjects.end())
		{
			if(o == (*it))
				return true;

			it ++;
		}
		
		return false;
	};

	int position(T o)
	{
		if(listObjects.empty())
			return -1;

		int pos = -1;

		it = listObjects.begin();

		while(it != listObjects.end())
		{
			pos ++;

			if(o == (*it))
				return pos;

			it ++;
		}

		return -1;
	};

	void clone(list<T>& l)
	{
		it = listObjects.begin();

		while(it != listObjects.end())
		{
			l.insert(l.end(), (*it));

			it ++;
		}
	};

	bool wait(unsigned int interval)
	{
		if(WaitForSingleObject(semaphore, interval) != WAIT_OBJECT_0)
			return false;

		ReleaseSemaphore(semaphore, 1, NULL);
	
		return true;
	};
};

#define protected_list LIST_PROTECT_OBJECT

template<typename T1, typename T2>
class MAP_PROTECT_OBJECT : public PROTECT_CONTAINER
{
public:
	MAP_PROTECT_OBJECT(){};
	~MAP_PROTECT_OBJECT(){};

private:
	map<T1, T2> mapObjects;
	typename map<T1, T2>::iterator it;

public:
	bool insert(T1 t1, T2 t2)
	{
		mapObjects.insert(map<T1, T2>::value_type(t1, t2));

		return true;
	};

	void erase(T1 t1)
	{
        typename map<T1, T2>::iterator it = mapObjects.find(t1);
		if(it != mapObjects.end())
			mapObjects.erase(it);
	};

	bool find(T1 t1, T2& t2)
	{
        typename map<T1, T2>::iterator it = mapObjects.find(t1);
		if(it != mapObjects.end())
		{
			t2 = (*it).second;
			return true;
		}

		return false;
	};

	void clear()
	{
		mapObjects.clear();
	};

	bool empty()
	{
		return mapObjects.empty();
	};

	bool end()
	{
		return (it == mapObjects.end());
	};

	bool removehead(T1& t1, T2& t2)
	{
		if(mapObjects.empty())
			return false;

        typename map<T1, T2>::iterator it = mapObjects.begin();

		t1 = (*it).first;
		t2 = (*it).second;

		mapObjects.erase(it);

		return true;
	};

	bool getcurrent(T1& t1, T2& t2)
	{
		if(mapObjects.empty())
			return false;
	
		if(it == mapObjects.end())
			return false;

		t1 = (*it).first;
		t2 = (*it).second;

		return true;
	};

	bool movefirst()
	{
		if(mapObjects.empty())
			return false;

		it = mapObjects.begin();

		return true;
	};

	bool movelast()
	{
		if(mapObjects.empty())
			return false;

		it = mapObjects.end() --;

		return true;
	};

	bool movenext()
	{
		if(mapObjects.empty())
			return false;

		if(it == mapObjects.end())
			return false;

		it ++;

		return true;
	};

	bool moveprev()
	{
		if(mapObjects.empty())
			return false;

		if(it == mapObjects.begin())
			return false;

		it --;

		return true;
	};

	size_t size()
	{
		return mapObjects.size();
	};

	void clone(map<T1, T2>& m)
	{
		it = mapObjects.begin();

		while(it != mapObjects.end())
		{
			T1 t1 = (*it).first;
			T2 t2 = (*it).second;

			m.insert(map<T1, T2>::value_type(t1, t2));

			it ++;
		}
	};
};

#define protected_map MAP_PROTECT_OBJECT

template<typename T1, typename T2>
class BIDIRECTIONAL_MAP_PROTECT_OBJECT : public PROTECT_CONTAINER
{
public:
	BIDIRECTIONAL_MAP_PROTECT_OBJECT(){};
	~BIDIRECTIONAL_MAP_PROTECT_OBJECT(){};

private:
	map<T1, T2> mapObjects1;
	typename map<T1, T2>::iterator it1;

	map<T2, T1> mapObjects2;
	typename map<T2, T1>::iterator it2;

public:
	bool insert(T1 t1, T2 t2)
	{
		mapObjects1.insert(map<T1, T2>::value_type(t1, t2));

		mapObjects2.insert(map<T2, T1>::value_type(t2, t1));

		return true;
	};

	void eraseByT1(T1 t1)
	{
		T2 t2;

        typename map<T1, T2>::iterator it1 = mapObjects1.find(t1);
		if(it1 != mapObjects1.end())
		{
			t2 = (*it1).second;	
			mapObjects1.erase(it1);
		}

        typename map<T2, T1>::iterator it2 = mapObjects2.find(t2);
		if(it2 != mapObjects2.end())
			mapObjects2.erase(it2);
	};

	void eraseByT2(T2 t2)
	{
		T1 t1;

        typename map<T2, T1>::iterator it2 = mapObjects2.find(t2);
		if(it2 != mapObjects2.end())
		{
			t1 = (*it2).second;	
            mapObjects2.erase(it2);
		}

        typename map<T1, T2>::iterator it1 = mapObjects1.find(t1);
		if(it1 != mapObjects1.end())
			mapObjects1.erase(it1);
	};

	bool findByT1(T1 t1, T2& t2)
	{
        typename map<T1, T2>::iterator it = mapObjects1.find(t1);
		if(it != mapObjects1.end())
		{
			t2 = (*it).second;
			return true;
		}

		return false;
	};

	bool findByT2(T2 t2, T1& t1)
	{
        typename map<T2, T1>::iterator it = mapObjects2.find(t2);
		if(it != mapObjects2.end())
		{
			t1 = (*it).second;
			return true;
		}

		return false;
	};

	void clear()
	{
		mapObjects1.clear();

		mapObjects2.clear();
	};

	bool empty()
	{
		return mapObjects1.empty();
	};

	bool end()
	{
		return (it1 == mapObjects1.end());
	};

	bool removehead(T1& t1, T2& t2)
	{
		if(mapObjects1.empty())
			return false;

        typename map<T1, T2>::iterator it1 = mapObjects1.begin();

		t1 = (*it1).first;
		t2 = (*it1).second;

		mapObjects1.erase(it1);

        typename map<T2, T1>::iterator it2 = mapObjects2.find(t2);
		if(it2 != mapObjects2.end())
			mapObjects2.erase(it2);

		return true;
	};

	bool getcurrent(T1& t1, T2& t2)
	{
		if(mapObjects1.empty())
			return false;
	
		if(it1 == mapObjects1.end())
			return false;

		t1 = (*it1).first;
		t2 = (*it1).second;

		return true;
	};

	bool movefirst()
	{
		if(mapObjects1.empty())
			return false;

		it1 = mapObjects1.begin();
		it2 = mapObjects2.begin();

		return true;
	};

	bool movelast()
	{
		if(mapObjects1.empty())
			return false;

		it1 = mapObjects1.end() --;
		it2 = mapObjects2.end() --;

		return true;
	};

	bool movenext()
	{
		if(mapObjects1.empty())
			return false;

		if(it1 == mapObjects1.end())
			return false;

		it1 ++;
		it2 ++;

		return true;
	};

	bool moveprev()
	{
		if(mapObjects1.empty())
			return false;

		if(it1 == mapObjects1.begin())
			return false;

		it1 --;
		it2 --;

		return true;
	};

	size_t size()
	{
		return mapObjects1.size();
	};
};

#define bidir_protected_map BIDIRECTIONAL_MAP_PROTECT_OBJECT

#endif
