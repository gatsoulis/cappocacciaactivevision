/***************************************************************************
                          outputconnector.h  -  description
 ***************************************************************************/

#ifndef OUTPUTCONNECTOR_H
#define OUTPUTCONNECTOR_H

#include <vector>
#include "inputconnector.h"


//! class OutputConnector  -- Template for an Output Connector
/*! class OutputConnector  -- Template for an Output Connector
  An outputconnector buffers data of given type, holds a set of input connectors it is connected to
  and stores the data in those connectors (most likely only pointers) if the modules calls OutputConnector.out(); */
  template <class T>
  class OutputConnector
  {                   
public: 
//! OutputConnector()  -- just an empty standard constructor
    OutputConnector();
//! ~OutputConnector()  -- just an empty standard destructor
    ~OutputConnector();
//! void out()  -- calls the in() function of input connectors registered by invoking connectTo(InputConnector<T>)
    void out();
//! void setBuffer(T data)  -- stores data in the internal buffer. Try to concentrate on handling with pointers!
    T setBuffer(T data);
//! void connectTo(InputConnector<T>* target, int exec=false)  -- connects this output module with the given input module
/*! void connectTo(InputConnector<T>* target, int exec=false)  -- connects this output module with the given input module
    by adding it's address to an internal set. Use exec=true if you want the
    target module to be executed everytime you store new data in the output buffer. */
    void connectTo(InputConnector<T>* target,    int exec=0);
    void connectToVoid(InputConnector<void*>* target, int exec=0);

    void disconnectAll();

    T getBuffer();
private:
    T buffer;
    std::vector<InputConnector<T>*> m_targets;
  };



  template <class T> OutputConnector<T>::OutputConnector()
  {
		buffer = NULL;
  }
  template <class T> OutputConnector<T>::~OutputConnector() {  }



  template <class T>
  T OutputConnector<T>::setBuffer(T data)
  {
    T old = buffer;
    buffer = data;
    return old;
  }

  template <class T>
  T OutputConnector<T>::getBuffer()
  {
    return buffer;
  } 

  template <class T>
  void OutputConnector<T>::out()
  {
//	std::cout<<"OutputConnector:Line 1...\n";
    typename std::vector< InputConnector<T>* >::iterator pos;
//	std::cout<<"OutputConnector:Line 2...\n";
    for (pos=m_targets.begin(); pos!=m_targets.end(); ++pos)
    {
      (*pos)->in(buffer);
    }
//	std::cout<<"OutputConnector:Line 3...\n";
  }
  template <class T>
  void OutputConnector<T>::connectTo(InputConnector<T>* target, int exec)
  {
    m_targets.push_back(target);
    target->exec=exec;
    target->setBuffer(buffer);
  }

  template <class T>
  void OutputConnector<T>::disconnectAll()
  {
    for(unsigned int i = 0;i < m_targets.size(); i++) { 
		m_targets[i]->setBuffer(NULL);
		m_targets[i]->exec = false;
	}
    m_targets.clear();
	buffer = NULL;
  }

  template <class T>
  void OutputConnector<T>::connectToVoid(InputConnector<void*>* target, int exec)
  {
    m_targets.push_back((InputConnector<T>*)target);
    target->exec=exec;
    target->setBuffer(buffer);
  }


#endif
