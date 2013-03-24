//-----------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
#include <assert.h>

//-----------------------------------------------------------------------------
#pragma warning(disable : 4355)

//-----------------------------------------------------------------------------
template <class T> class CSingleton
{
private:
		static T *pInstance;

		CSingleton(const CSingleton &rhs);
		CSingleton& operator= (const CSingleton &rhs);

public:
		// Get object Instance
		static T* getInstance()
		{
			assert(pInstance);
			return pInstance;
		}

		// Constructor & Destructor
		CSingleton(T& obj)
		{
			assert(!pInstance);
			pInstance = &obj;
		}

#ifdef __GNUC__
                // for g++ and gcc compilers
                virtual ~CSingleton()
#else
                // _MSC_VER // for Microsoft C++ compilers
                virtual ~CSingleton() = 0
#endif
		{
			assert(pInstance);
			pInstance = NULL;
		}

};

template <class T> T* CSingleton<T>::pInstance = NULL;

#pragma warning(default : 4355)

//-----------------------------------------------------------------------------
