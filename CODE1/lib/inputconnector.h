/***************************************************************************
                            inputconnector.h  -  description
 ***************************************************************************/

#ifndef INPUTCONNECTOR_H
#define INPUTCONNECTOR_H

/**
  *@author 
  */
#include <iostream>
#include "module.h"
#define CALL_MEMBER_FN(object, ptrToMember) ((object).*(ptrToMember))



typedef void (Module::*ModuleFn)();

//! class InputConnector  -- Template for an Input Connector
/*! class InputConnector  -- Template for an Input Connector
  An input connector buffers data of given type T. This data can be stored by calling the in(T) function by a connected module.
  You can get access to the data through the function T getBuffer(). There is also a pointer to the parent module for triggering connections.
  In this case the parent module will be executed when new data is stored into the input connector */
template <class T>
class InputConnector
{

	public: 

		//! InputConnector(Module*)  -- constructor that should be called by the parent module with 'this' as parameter.
		InputConnector(Module* m = NULL, ModuleFn mF = &Module::execute);

		//! ~InputConnector()  -- just the standard destructor
		virtual ~InputConnector();

		//! void in(T)  -- a function called by connected output connectors. The parameter is the new data...
		virtual void in(T data);

		//! int exec  -- a boolean value states whether the parent module shall be executed on the income of new data
		int exec;

		//! T getbuffer()  -- returns the value stored in the internal buffer
		T getBuffer();
    
		T setBuffer(T data);

		//! void setModule(Module* m)  -- function to set the pointer to the parent module (should already be done by the constructor)
		void setModule(Module* m = NULL, ModuleFn mF = &Module::execute);

	private:

		Module* module;
		ModuleFn modFunc;
		T buffer;
		//ost::Mutex lock;
};


template <class T> InputConnector<T>::InputConnector(Module* m, ModuleFn mF)
{
	exec = 0;
	buffer = NULL;
	module = m;
	modFunc = mF;
}

template <class T>
void InputConnector<T>::setModule(Module* m, ModuleFn mF)
{
	module = m;
	modFunc = mF;
}

template <class T> InputConnector<T>::~InputConnector()  {  }

template <class T>
void InputConnector<T>::in(T data)
{
	//  lock.enterMutex(); let's assume that pointer writing/reading is atomic!
	buffer = data;
	//  lock.leaveMutex();
	if(exec == 2 && module && modFunc == &Module::execute) module->execute_profiled_ms_stdout();
	else if (exec > 0 && module) CALL_MEMBER_FN(*module, modFunc)();
}

template <class T>
T InputConnector<T>::getBuffer()
{
	//  lock.enterMutex();
	//  lock.leaveMutex();
	return buffer;
}

template <class T>
T InputConnector<T>::setBuffer(T data)
{
	T old = buffer;
	buffer = data;
	return old;
}



#endif
