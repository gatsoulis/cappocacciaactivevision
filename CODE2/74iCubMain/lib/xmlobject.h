#ifndef XML_OBJECT_H
#define XML_OBJECT_H
                                                                                                          
#include "cv.h"
#include <string>
                                                                                                          
/**
  *@author Erik Murphy-Chutorian
  */

        class XmlObject
        {
			public:
				XmlObject(std::string name = "Untitled Object", CvRect location = cvRect(0,0,0,0)) {m_name = name; m_location = location;}
				~XmlObject() {}

				std::string getName() {return m_name;}
				CvRect getLocation() {return m_location;}
            
                void setName(std::string name) {m_name = name;}
                void setLocation(CvRect location) { m_location = location;}

			private:
		
				std::string m_name;
				CvRect m_location;
        };

#endif

