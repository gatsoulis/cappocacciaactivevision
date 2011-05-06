#ifndef CSCLoad_H
#define CSCLoad_H

//#include "ipp.h"
//#include "cv.h"
//#include "cxcore.h"
//#include "highgui.h"

#include "module.h"
#include "inputconnector.h"
#include "outputconnector.h"
#include "loadcvimage.h"
#include "convertcvimagegray.h"
#include "xmlobject.h"
//#include "cvimage.h"
//#include "csctype.h"
#include "saxparsertrack.h"
#include <libxml++/libxml++.h>

#include <ctime>
#include <iostream> 
#include <fstream>
#include <cstdlib>
//#include <pthread.h>



	class CSCLoad : public Module  {
		public: 
			CSCLoad(std::string name="CSCLoad Module");
			~CSCLoad();

		    //! void execute()  -- loads both left and right images
		    /*! void execute()  -- loads both left and right images
		    this function should be executed by thread, main loop or something else. */
			void execute();

			bool setXMLFilename(std::string* filenamexml);

			void setParam(int startindex = 1, int step = 5, float scale = 1.0);
			
			int getNumSlides();

			int getCounter();

			OutputConnector<CVImage*> cvLeftImageOut;
			OutputConnector<CVImage*> cvRightImageOut;
			OutputConnector<TrackData*> trackOut;
			OutputConnector<VergenceOutput*> vergOut;
			OutputConnector<MotorCommand*> eyeneckOut;
			OutputConnector<std::string*> labelOut;
			OutputConnector<XmlObject*> objectOut;

			OutputConnector<CVImage*> cvLeftGrayImageOut;
			OutputConnector<CVImage*> cvRightGrayImageOut;

			OutputConnector<std::string*> filenameLeftOut;
			OutputConnector<std::string*> filenameRightOut;

			OutputConnector<std::string*> fullfilenameLeftOut;
			OutputConnector<std::string*> fullfilenameRightOut;

			bool checkEnd();
			
			bool debug;

		private:
		
			SaxParserTrack m_parser;
			
			LoadCVImage m_loadleft;
			LoadCVImage m_loadright;
			ConvertCVImageGray m_convleft;
			ConvertCVImageGray m_convright;
		
			int m_startindex, m_counter, m_step;
			int m_totalimagesize;
			float m_scale;

			XmlObject m_object;
			CvRect m_location;
			TrackData m_winner;
			bool m_end;
		
	};


#endif
