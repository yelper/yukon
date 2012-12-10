
#ifndef __Singleton_h__
#define __Singleton_h__

namespace MLLib
{

/**
* Template for a singleton class.
*/
template <class T>
class Singleton
{

protected:

	/**
	* Constructor.
	*/
	Singleton(){}

	/**
	* Destructor.
	*/
	virtual ~Singleton()
	{
	}

public:

	/**
	* Gets the singleton instance.
	*/
	static T* Instance()
	{
		if( mInst == NULL )
			mInst = new T();

		return mInst;
	}

	/**
	* Deletes the singleton instance, this will typically be called
	* at the end of the program execution.
	*/
	static void Release()
	{
		if( mInst != NULL )
			delete mInst;
		mInst = NULL;
	}

protected:

	static T* mInst; ///< The singleton instance.

};

template <class T> T* Singleton<T>::mInst = NULL;

}

#endif // __Singleton_h__
