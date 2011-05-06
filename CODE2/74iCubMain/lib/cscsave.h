#ifndef CSCSave_H
#define CSCSave_H

//#include "ipp.h"
//#include "cv.h"
#include "highgui.h"

#include "module.h"
#include "inputconnector.h"
#include "outputconnector.h"
#include "cvimage.h"
#include "csctype.h"

#include <fstream>
#include <sys/stat.h>


	class CSCSave : public Module  {
		public: 
			CSCSave(std::string name="CSCSave");
			~CSCSave();

		    //! void execute()  -- grabs both left and right images and stores them in ImageOutL, ImageOutR
		    /*! void execute()  -- grabs both left and right images and stores them in ImageOutL, ImageOutR
		    this function should be executed by thread, main loop or something else. */
			void execute();

			void setParam(std::string Path = "./image/", int frameSize = 500, std::string imagetype = "png", CvSize imageSize = cvSize(640, 480));

			InputConnector<CVImage*> cvLeftImageIn;
			InputConnector<CVImage*> cvRightImageIn;
			InputConnector<TrackData*> trackIn;
			InputConnector<VergenceData*> vergIn;
			InputConnector<MotorCommand* > eyeneckIn;
			InputConnector<std::string*> labelIn;

			OutputConnector<CVImage*> cvLeftImageOut;
			OutputConnector<CVImage*> cvRightImageOut;
			OutputConnector<TrackData*> trackOut;
			OutputConnector<VergenceData*> vergOut;
			OutputConnector<MotorCommand*> eyeneckOut;
			OutputConnector<std::string*> labelOut;

			void getData(int index);
			int m_counter;

			void saveToDisk();
			void appendToDisk();
			
			void removeObj(const std::string& objectName);

			bool isBufferFull();
			//std::string* getXMLFilename();
			
			bool debug;

		private:
			CVImage* mp_cvleftImg;
			CVImage* mp_cvrightImg;
			char* mp_cvleftImgdata;
			char* mp_cvrightImgdata;
			CVImage* cvleftImage;
			CVImage* cvrightImage;

			char** mdp_dataLeft;
			char** mdp_dataRight;
			TrackData* mp_winners;
			VergenceData* mp_vergs;
			MotorCommand* mp_eyenecks;

			std::string m_path;
			std::string m_imagetype;
			std::string m_extLeftImage;
			std::string m_extRightImage;

			std::string m_filenameXML;

			int m_sizeAFrameL, m_sizeAFrameR, m_bufferSize;

			std::string* mp_label;

			std::fstream dataFile;

			bool m_initialized;
			bool m_bufferFull;
			
			TrackData* winner;
			VergenceData* verg;
			MotorCommand* eyeneck;
			
			bool checkfolder(const std::string& path);
			bool checkfile(const std::string& file);

			bool checkncreatefolder(const std::string& path);
			bool checkncreatelistdtdfile(const std::string& file);
			bool checkncreatetrackdtdfile(const std::string& file);
			
			
	};


#endif
