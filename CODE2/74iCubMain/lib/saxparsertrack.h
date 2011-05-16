#ifndef SaxParserTrack_H
#define SaxParserTrack_H

#include "csctype.h"

#include <libxml++/libxml++.h>
#include <cstdlib>
#include <iostream>


	class SaxParserTrack : public xmlpp::SaxParser {

		public:
			SaxParserTrack();
			virtual ~SaxParserTrack();

			std::string* getObjName() { return &objname; }
			unsigned int getSlideSize() { return slidesize; }
			std::string* getPath() { return &path; }
			std::string* getFilenameLeft(unsigned int ind) { return &(filenamesleft[ind]); }
			std::string* getFilenameRight(unsigned int ind) { return &(filenamesright[ind]); }
			TrackData* getTrackData(unsigned int ind) { return &(trackdata[ind]); }
			VergenceOutput* getVergenceData(unsigned int ind) { return &(vergencedata[ind]); }
			MotorCommand* getMotorData(unsigned int ind) { return &(eyeneckdata[ind]); }
			
			void clear();
  
		protected:

			//overrides:
			virtual void on_start_document();
			virtual void on_end_document();
			virtual void on_start_element(const Glib::ustring& name, const AttributeList& properties);
			virtual void on_end_element(const Glib::ustring& name);
			
			virtual void on_characters(const Glib::ustring& characters);
			virtual void on_comment(const Glib::ustring& text);
			virtual void on_warning(const Glib::ustring& text);
			virtual void on_error(const Glib::ustring& text);
			virtual void on_fatal_error(const Glib::ustring& text);
			
		private:
	
			std::string objname;
			unsigned int slidesize;
			std::string path;
			std::vector<std::string> filenamesleft;
			std::vector<std::string> filenamesright;
			std::vector<TrackData> trackdata;
			std::vector<VergenceOutput> vergencedata;
			std::vector<MotorCommand> eyeneckdata;
			TrackData winner;
	  
	};

#endif
