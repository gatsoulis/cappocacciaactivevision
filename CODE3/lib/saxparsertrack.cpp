#include "saxparsertrack.h"



SaxParserTrack::SaxParserTrack() : xmlpp::SaxParser() {
}

SaxParserTrack::~SaxParserTrack() {
	this->clear();
}


void SaxParserTrack::on_start_document() {
	this->clear();
	//std::cout << "on_start_document()" << std::endl;
}

void SaxParserTrack::clear() {
	filenamesleft.clear();
	filenamesright.clear();
	trackdata.clear();
	vergencedata.clear();
	eyeneckdata.clear();
}

void SaxParserTrack::on_end_document() {
	//std::cout << "on_end_document()" << std::endl;
}

void SaxParserTrack::on_start_element(const Glib::ustring& name, const AttributeList& attributes) {

//  std::cout << "node name=" << name << std::endl;
  // Print attributes:
//  for(xmlpp::SaxParser::AttributeList::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter)
//  {
//    std::cout << "  Attribute " << iter->name << " = " << iter->value << std::endl;
//  }


	if(name == "Sequence"){
	}	
	else if(name == "Object"){
		for(xmlpp::SaxParser::AttributeList::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter) {
			//std::cout << "  Attribute " << iter->name << " = " << iter->value << std::endl;
			if(iter->name == "name") {
				objname = iter->value;
			}
			else if(iter->name == "totalSlideNumber") {
				slidesize = atoi((iter->value).c_str());
			}
			else {
				std::cerr << "SaxParserTrack::on_start_element()::ERROR::Unknown attribute type[" << iter->name << "]\n";
			}
		}
	}	
	else if(name == "Path"){
		for(xmlpp::SaxParser::AttributeList::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter) {
			if(iter->name == "name") {
				path = iter->value;
			}
			else {
				std::cerr << "SaxParserTrack::on_start_element()::ERROR::Unknown attribute type[" << iter->name << "]\n";
			}
		}
	}	
	else if(name == "Slide"){
	}	
	else if(name == "Image"){
		std::string tempside;
		for(xmlpp::SaxParser::AttributeList::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter) {
			if(iter->name == "side") {
				tempside = iter->value;
			}
			else if(iter->name == "id") {
				std::string filename = iter->value;
				if(tempside == "left") {
					filenamesleft.push_back(filename);
				}
				else if(tempside == "right") {
					filenamesright.push_back(filename);
				}
				else {
					std::cerr << "SaxParserTrack::on_start_element()::ERROR::Unknown attribute type[" << tempside << "]\n";
				}
			}
			else {
				std::cerr << "SaxParserTrack::on_start_element()::ERROR::Unknown attribute type[" << iter->name << "]\n";
			}
		}
	}	
	else if(name == "WinnerPos"){
		for(xmlpp::SaxParser::AttributeList::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter) {
			if(iter->name == "x") {
				winner.winnerPos.x = atoi((iter->value).c_str());
			}
			else if(iter->name == "y") {
				winner.winnerPos.y = atoi((iter->value).c_str());
			}
			else {
				std::cerr << "SaxParserTrack::on_start_element()::ERROR::Unknown attribute type[" << iter->name << "]\n";
			}
		}
	}	
	else if(name == "WinnerSize"){
		for(xmlpp::SaxParser::AttributeList::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter) {
			if(iter->name == "width") {
				winner.winnerSize.width = atoi((iter->value).c_str());
			}
			else if(iter->name == "height") {
				winner.winnerSize.height = atoi((iter->value).c_str());
			}
			else {
				std::cerr << "SaxParserTrack::on_start_element()::ERROR::Unknown attribute type[" << iter->name << "]\n";
			}
		}
	}	
	else if(name == "WinnerRect"){
		for(xmlpp::SaxParser::AttributeList::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter) {
			if(iter->name == "x") {
				winner.winnerRect.x = atoi((iter->value).c_str());
			}
			else if(iter->name == "y") {
				winner.winnerRect.y = atoi((iter->value).c_str());
			}
			else if(iter->name == "width") {
				winner.winnerRect.width = atoi((iter->value).c_str());
			}
			else if(iter->name == "height") {
				winner.winnerRect.height = atoi((iter->value).c_str());
			}
			else {
				std::cerr << "SaxParserTrack::on_start_element()::ERROR::Unknown attribute type[" << iter->name << "]\n";
			}
		}
	}	
	else if(name == "Reliability"){
		for(xmlpp::SaxParser::AttributeList::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter) {
			if(iter->name == "val") {
				winner.reliability = atof((iter->value).c_str());
				TrackData temp;
				temp.winnerPos = winner.winnerPos;
				temp.winnerSize = winner.winnerSize;
				temp.winnerRect = winner.winnerRect;
				temp.reliability = winner.reliability;
				trackdata.push_back(temp);
			}
			else {
				std::cerr << "SaxParserTrack::on_start_element()::ERROR::Unknown attribute type[" << iter->name << "]\n";
			}
		}
	}	
	else if(name == "Vergence"){
		VergenceOutput tempverg;
		for(xmlpp::SaxParser::AttributeList::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter) {
			if(iter->name == "leftH") {
				tempverg.leftHorDisp = atof((iter->value).c_str());
			}
			else if(iter->name == "leftV") {
				tempverg.leftVerDisp = atof((iter->value).c_str());
			}
			else if(iter->name == "rightH") {
				tempverg.rightHorDisp = atof((iter->value).c_str());
			}
			else if(iter->name == "rightV") {
				tempverg.rightVerDisp = atof((iter->value).c_str());
			}
			else {
				std::cerr << "SaxParserTrack::on_start_element()::ERROR::Unknown attribute type[" << iter->name << "]\n";
			}
		}
		vergencedata.push_back(tempverg);
	}	
	else if(name == "EyeNeck"){
		MotorCommand tempmotor;
		for(xmlpp::SaxParser::AttributeList::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter) {
			if(iter->name == "LEyeH") {
				tempmotor.leftEyeHor = atof((iter->value).c_str());
			}
			else if(iter->name == "LEyeV") {
				tempmotor.leftEyeVer = atof((iter->value).c_str());
			}
			else if(iter->name == "REyeH") {
				tempmotor.rightEyeHor = atof((iter->value).c_str());
			}
			else if(iter->name == "REyeV") {
				tempmotor.rightEyeVer = atof((iter->value).c_str());
			}
			else if(iter->name == "NeckH") {
				tempmotor.neckHor = atof((iter->value).c_str());
			}
			else if(iter->name == "NeckV") {
				tempmotor.neckVer = atof((iter->value).c_str());
			}
			else {
				std::cerr << "SaxParserTrack::on_start_element()::ERROR::Unknown attribute type[" << iter->name << "]\n";
			}
		}
		tempmotor.eyebrows = 0.0;
		tempmotor.jaw = 0.0;
		tempmotor.mouth = 0.0;
		eyeneckdata.push_back(tempmotor);
	}
	else{
		std::cerr << "SaxParserTrack::on_start_element()::ERROR::Unknown node type[" << name << "]\n";
	}

}

void SaxParserTrack::on_end_element(const Glib::ustring& name) {
	//std::cout << "on_end_element()" << std::endl;
}

void SaxParserTrack::on_characters(const Glib::ustring& text) {
	//std::cout << "on_characters(): " << text << std::endl;
}

void SaxParserTrack::on_comment(const Glib::ustring& text) {
	//std::cout << "on_comment(): " << text << std::endl;
}

void SaxParserTrack::on_warning(const Glib::ustring& text) {
	std::cout << "on_warning(): " << text << std::endl;
}

void SaxParserTrack::on_error(const Glib::ustring& text) {
	std::cout << "on_error(): " << text << std::endl;
}

void SaxParserTrack::on_fatal_error(const Glib::ustring& text) {
	std::cout << "on_fatal_error(): " << text << std::endl;
}


