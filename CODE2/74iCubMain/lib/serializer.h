#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <iostream>
//#include <vector>



	void blockToStream(std::ostream* str, char* data, long size);
	void blockFromStream(std::istream* str, char* data, long size);

	//void toStream(float* data, std::ostream* str);
	//bool fromStream(float** data, std::istream* str);

	//template <class T> void toStream(std::vector<T>* data, std::ostream* str);
	//template <class T> bool fromStream(std::vector<T>** data, std::istream* str);



/*
template <class T> void csc::toStream(std::vector<T>* data, std::ostream* str) {
	*str << "Vector" << std::endl;
	*str << data->size() << std::endl;
	typename std::vector<T>::iterator pos;
	for (pos = data->begin(); pos != data->end(); ++pos) csc::toStream(*pos, str);
	*str << "END" << std::endl;
}

template <class T> bool csc::fromStream(std::vector<T>** data, std::istream* str) {
	// check label
	std::string name;
	getline(*str,name);
	if (name != "Vector")
	{
		std::cout << "Received data is labeled to be of type '"<<name<<"', but should be 'Vector'. Ignoring..." << std::endl;
		return false;
	}
	int size;  // element count
	*str >> size;
	getline(*str,name); // remove linebreak
	
	// make the vector ready
	if ((*data) == NULL) 
	{
		*data = new std::vector<T>;	
		// create new element pointers
		for (int i=0; i<size; i++) (*data)->push_back(NULL);
	}
	else 
	{
		if (size != ((int)(*data)->size()))
		{
			// delete old vector and elements
			typename std::vector<T>::iterator pos;
			for (pos = (*data)->begin(); pos != (*data)->end(); ++pos) delete (*pos);
			(*data)->clear();
			
			// create new element pointers
			for (int i=0; i<size; i++) (*data)->push_back(NULL);
		}
	}

	// read the objects into the vector
	bool success = true;
	typename std::vector<T>::iterator pos;
	for (pos = (*data)->begin(); pos != (*data)->end(); ++pos) success = csc::fromStream(&(*pos), str) && success;

//	int i;
//	for (i=0; i<size; i++) {
//		T newObj = NULL;
//		csc::fromStream(&newObj, str);
//		(*data)->push_back(newObj);
//	}

	if (!success)
	{
		std::cout << "Object data for vector arrived incompletely!\n";
		return false;
	}
	getline(*str,name);
	if (name!="END") 
	{
		std::cout << "Data arrived incompletely!\n";
		return false;
	}
	return true;
}
*/

#endif
