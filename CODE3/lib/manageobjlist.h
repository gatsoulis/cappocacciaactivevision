#ifndef ManageObjList_H
#define ManageObjList_H

#include "module.h"
#include "inputconnector.h"
#include "outputconnector.h"
#include "xmlobject.h"

#include "stdio.h"
#include <libxml++/libxml++.h>
#include <iostream> 
#include <fstream>
#include <sys/stat.h>


	class ManageObjList : public Module  {
		public: 
			ManageObjList(std::string name="ManageObjList");
			~ManageObjList();

			void execute(); // This will return object's name and xml filename one by one.

			void setParam(const std::string& path, const std::string& filename = "list.xml");

			InputConnector<int*> counterIn;
			InputConnector<std::string*> objnameIn;

			OutputConnector<int*> counterOut;
			OutputConnector<std::string*> objnameOut;
			OutputConnector<std::string*> filenameOut;

			bool checkObjName(const std::string& name);
			void attachObj(const std::string& objectName);
			void removeObj(const std::string& objectName);
			//void removeObj(int index);
			const std::string& getData(const int& objindex);
			const std::string& getData(const std::string& objname);
			int getNumberObjects(); // This will use objNameIn connector
			int getIndex(const std::string& objectName);
			const std::string& getObjectName(const int& index);

			void writeToFile(const std::string& filename);
			void readFromFile(const std::string& filename);
			int getLastIndex() { return m_index; }
			
			bool debug;

		private:
		
			int* mp_totalObjects;
			int* mp_counter;
			int* mp_lastcounter;
			bool* mp_nameExistence;
			int m_indexExistence;
			xmlpp::Node* pNode;

			void loadXmlList();
			void parseXmlList(const xmlpp::Node* node, const std::string& directoryName);
			void parseXmlMax(const xmlpp::Node* node, const std::string& directoryName);
			void parseXmlName(const xmlpp::Node* node, const std::string& directoryName, const std::string& objectName);
			void parseXmlName(const xmlpp::Node* node, const std::string& directoryName);

			std::string m_path, m_filename, m_listname;
			std::string objname;
			std::string filename;
			std::string strnumber;
			xmlpp::DomParser parser;
			std::string directoryName;
			std::fstream dataFile;
			int m_index;
			bool temp;
			std::string m_input;
			
			bool m_init;

			bool checkncreatefolder(const std::string& path);
			bool checkncreatelistdtdfile(const std::string& file);
			bool checkncreatelistxmlfile(const std::string& file);

	};


#endif
