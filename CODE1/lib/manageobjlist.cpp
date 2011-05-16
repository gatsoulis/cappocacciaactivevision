#include "manageobjlist.h"
#include "serializer.h"



ManageObjList::ManageObjList(std::string name) : Module(name){
	std::cout<< "\nConstructing " << getName() << "...";

	counterIn.setModule(this);
	objnameIn.setModule(this);

	pNode = NULL;

	mp_totalObjects = new int [1];
	*mp_totalObjects = 0;

	mp_counter = new int [1];
	*mp_counter = 0;

	mp_lastcounter = new int [1];
	*mp_lastcounter = 0;

	mp_nameExistence = new bool [1];
	*mp_nameExistence = false;
	
	m_init = false;
	debug = false;

	std::cout<< "finished.";
}

ManageObjList::~ManageObjList(){
	std::cout<< "\nDestructing " << getName() << "...";
	if(mp_totalObjects != NULL) delete [] mp_totalObjects;
	if(mp_counter != NULL) delete [] mp_counter;
	if(mp_lastcounter != NULL) delete [] mp_lastcounter;
	if(mp_nameExistence != NULL) delete [] mp_nameExistence;

	if (dataFile.is_open()) {
		dataFile.close();
	}
	std::cout<< "finished.";
}

void ManageObjList::setParam(const std::string& path, const std::string& filename) {
	m_path = path;
	m_filename = filename;

	int ind = m_path.size() - 1;
	if(m_path[ind] != '/') m_path = m_path + "/";

	if(debug) std::cout << std::endl << getName() << "::setParam()::m_path = [" << m_path << "]\n";
	checkncreatefolder(m_path);

	std::string dtdname = m_path + "list.dtd";
	checkncreatelistdtdfile(dtdname);

	m_listname = m_path + m_filename;
	if(debug) std::cout << getName() << "::setParam()::m_listname = [" << m_listname << "]\n";
	checkncreatelistxmlfile(m_listname);

/*
	std::ofstream datafileNew;

	int returnValue = chdir(m_path.c_str());
	// directory for the list.xml exists
	if(returnValue == 0){
		std::cout<< getName() << "::Directory : " << m_path << " found.\n";
		
		std::string tempname = m_path + "list.dtd";
		std::fstream datafileOld;
		datafileOld.open(tempname.c_str(), std::fstream::in);
		if(datafileOld.is_open()){
			std::cout<< getName() << "::File : " << tempname << " exists!\n";
			datafileOld.close();
		}
		else{
			//datafileOld.close();

			std::cout<< getName() << "::File : " << tempname << " doesnt exist. Creating one...\n";
			// list.dtd file doesnt exist. Creating one...
			datafileNew.open(tempname.c_str(), std::ios::out);
			datafileNew << "<!ELEMENT Database (Object*)>\n";
			datafileNew << "<!ELEMENT Object EMPTY>\n";
			datafileNew << "<!ATTLIST Object id CDATA #REQUIRED>\n";
			datafileNew << "<!ATTLIST Object objname CDATA #REQUIRED>\n";
			datafileNew << "<!ATTLIST Object filename CDATA #REQUIRED>\n";
			datafileNew.close();
		}
	}
	else {
		// directory for the list.xml doesnt exist, create one.
		mode_t mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH;
		returnValue = mkdir(m_path.c_str(), mode);
		if(returnValue == 0) std::cout<< getName() << "::initialize()::Creating directory : " << m_path << " successful....\n";
		else{ std::cerr << getName() << "::initialize()::Creating directory : " << m_path << " failed !....\n"; return; }
		returnValue = chdir(m_path.c_str());

		// create list.dtd file for use in track xml data
		std::string tempname = m_path + "list.dtd";
		datafileNew.open(tempname.c_str(), std::ios::out);
		datafileNew << "<!ELEMENT Database (Object*)>\n";
		datafileNew << "<!ELEMENT Object EMPTY>\n";
		datafileNew << "<!ATTLIST Object id CDATA #REQUIRED>\n";
		datafileNew << "<!ATTLIST Object objname CDATA #REQUIRED>\n";
		datafileNew << "<!ATTLIST Object filename CDATA #REQUIRED>\n";
		datafileNew.close();
		std::cout << "done.\n";

	}
	

	std::cout << getName() << "::setParam()::Opening file [" << m_listname << "]...";
	dataFile.open(m_listname.c_str(), std::ios::out | std::ios::in);
	temp = dataFile.is_open();
	if(temp == true) std::cout << m_listname << " opened succesfully.\n";
	else {  
		std::cout << m_listname << " failed to open.\n";
		dataFile.close();		
		std::cout << "Creating a new one...\n";
		std::ofstream datafileNew;
		datafileNew.open(m_listname.c_str(), std::ios::out);
		datafileNew << "<!DOCTYPE Database SYSTEM \"list.dtd\">\n";
		datafileNew << "<Database>\n";
		datafileNew << "</Database>\n";
		datafileNew.close();
		std::cout << "done...";
		dataFile.open(m_listname.c_str(), std::ios::out | std::ios::in);
	}
	std::cout << "done.\n";
*/

	this->loadXmlList();
	
	m_init = true;
}

void ManageObjList::execute()
{
	// This function will output counter, objName and filename one by one from the list.xml file
//	this->loadXmlList();
//	this->parseXmlList(pNode, directoryName);
}

const std::string& ManageObjList::getData(const int& objindex) {
	if(!m_init) { std::cerr << getName() << "::getData()::ERROR::call setParam() first!\n"; }

	//m_index = objindex + 1;
	m_index = objindex;
	this->parseXmlList(pNode, directoryName);
	return filename;
}

const std::string& ManageObjList::getData(const std::string& objname) {
	if(!m_init) { std::cerr << getName() << "::getData()::ERROR::call setParam() first!\n"; }

	m_input = objname;
	this->parseXmlName(pNode, directoryName);
	return filename;
}

int ManageObjList::getNumberObjects() {
	if(!m_init) { std::cerr << getName() << "::getNumberObjects()::ERROR::call setParam() first!\n"; return 0; }

	*mp_totalObjects = -1;
	this->parseXmlMax(pNode, directoryName);
//	*mp_totalObjects = *mp_totalObjects + 1;
	return *mp_totalObjects;
}

void ManageObjList::parseXmlMax(const xmlpp::Node* node, const std::string& directoryName) {

    const xmlpp::ContentNode* nodeContent = dynamic_cast<const xmlpp::ContentNode*>(node);
    std::string nodename = node->get_name();

	if(nodename == "Object")  {
        std::string str_number;
        const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(node);
        const xmlpp::Element::AttributeList& attributes = nodeElement->get_attributes();
        for(xmlpp::Element::AttributeList::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter)
        {
   	        const xmlpp::Attribute* attribute = *iter;
            if(attribute->get_name() == "id")
				str_number = attribute->get_value();
            if(attribute->get_name() == "objname")
				objname = attribute->get_value();
            if(attribute->get_name() == "filename"){
				filename = attribute->get_value();
			}
        }

		*mp_totalObjects = atoi(str_number.c_str());
	}
	
    if(!nodeContent) {
        xmlpp::Node::NodeList list = node->get_children();  //Recurse through child nodes
        for(xmlpp::Node::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter) {
            parseXmlMax(*iter, directoryName); //recursive
        }
    }
}

void ManageObjList::loadXmlList() {
    parser.set_validate();
    parser.parse_file(m_listname);
    if(!parser) std::cerr << getName() << ": Could Not open XML file [" << m_listname << "] for parsing!!\n";
    else {                                                                                                                   
        //Walk the tree
		std::string templistname = m_listname;
		directoryName = templistname.erase(templistname.find_last_of("/") + 1);
		pNode = parser.get_document()->get_root_node(); //deleted by DomParser.
    }                                                                                                                     
}

void ManageObjList::parseXmlList(const xmlpp::Node* node, const std::string& directoryName) {

    const xmlpp::ContentNode* nodeContent = dynamic_cast<const xmlpp::ContentNode*>(node);
    std::string nodename = node->get_name();

	temp = false;
	if(nodename == "Object") {
		int temp_counter;
        std::string str_number;
        std::string str_objname;
        std::string str_filename;
        const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(node);
        const xmlpp::Element::AttributeList& attributes = nodeElement->get_attributes();
        for(xmlpp::Element::AttributeList::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter)
        {
   	        const xmlpp::Attribute* attribute = *iter;
            if(attribute->get_name() == "id") str_number = attribute->get_value();
            if(attribute->get_name() == "objname") str_objname = attribute->get_value();
            if(attribute->get_name() == "filename"){ 
				str_filename = attribute->get_value();
				temp = true;
			}
        }

		temp_counter = atoi(str_number.c_str());

		if(temp_counter == m_index && temp == true){
		
			objname = str_objname;
			filename = str_filename;
			*mp_counter = temp_counter;
		
			counterOut.setBuffer(mp_counter);
			counterOut.out();
			objnameOut.setBuffer(&objname);
			objnameOut.out();
			filenameOut.setBuffer(&filename);
			filenameOut.out();
			temp = false;
			nodeContent = NULL;
			return;
		}
	}

    if(!nodeContent) {
        xmlpp::Node::NodeList list = node->get_children();  //Recurse through child nodes
        for(xmlpp::Node::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter) {
            parseXmlList(*iter, directoryName); //recursive
        }
    }
}

bool ManageObjList::checkObjName(const std::string& name) {
	if(!m_init) { std::cerr << getName() << "::checkObjName()::ERROR::call setParam() first!\n"; return false; }

	*mp_nameExistence = false;
	m_indexExistence = -1;
	this->parseXmlName(pNode, directoryName, name);
	return *mp_nameExistence;
}

void ManageObjList::parseXmlName(const xmlpp::Node* node, const std::string& directoryName, const std::string& objectName) {

    const xmlpp::ContentNode* nodeContent = dynamic_cast<const xmlpp::ContentNode*>(node);
    std::string nodename = node->get_name();

	if(nodename == "Object") {
        const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(node);
        const xmlpp::Element::AttributeList& attributes = nodeElement->get_attributes();
        for(xmlpp::Element::AttributeList::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter)
        {
   	        const xmlpp::Attribute* attribute = *iter;
            if(attribute->get_name() == "id")
				strnumber = attribute->get_value();
            if(attribute->get_name() == "objname")
				objname = attribute->get_value();
            if(attribute->get_name() == "filename"){
				filename = attribute->get_value();
			}
        }
	}
	
    if(!nodeContent) {
        xmlpp::Node::NodeList list = node->get_children();  //Recurse through child nodes
        for(xmlpp::Node::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter) {
            parseXmlName(*iter, directoryName, objectName); //recursive
        }
    }
	else{
		if(objectName == objname){
			*mp_nameExistence = true;
			m_indexExistence = atoi(strnumber.c_str());
		}
	}
}

void ManageObjList::attachObj(const std::string& objectName) {
	if(!m_init) { std::cerr << getName() << "::attachObj()::ERROR::call setParam() first!\n"; return; }

	if(objectName == "") { std::cerr << "ManageObjList::attachObj()::ERROR::objectName is Null!...\n"; return; }

	this->parseXmlMax(pNode, directoryName);
	int counter = (this->getNumberObjects()) + 1;
	if(counter <= 0) counter = 1;

	std::string fileName = m_path + "XMLfolder/" + objectName + "_tracking.xml";
	std::cout << getName() << "::Attaching object " << objectName << " to the list...\n";

	dataFile.open(m_listname.c_str(), std::ios::out | std::ios::in);
	temp = dataFile.is_open();
	if(temp) {
		std::cout << m_listname << " opened succesfully.\n";
		dataFile.seekp(0, std::ios_base::end);
		long pos = dataFile.tellp();
		dataFile.seekp(pos - 12);
		dataFile << "<Object id=\"" << counter << "\" objname=\"" << objectName << "\" filename=\"" << fileName << "\"/>\n";
		dataFile << "</Database>\n";
		dataFile.flush();
		dataFile.close();
	}
	else {  
		std::cerr << getName() << "::attachObj()::ERROR::Failed to open file [" << m_listname << "]\n";
	}

	this->loadXmlList();
}


void ManageObjList::removeObj(const std::string& objectName) {
	std::cout << getName() << "::removeObj()::Not yet implemented\n";

//	bool exist = this->checkObjName(objectName);
//	if(exist == false) { std::cerr << getName() << "::removeObj()::ERROR::obj [" << objectName << "] does not exist in the list.xml file\n"; return; }


//	if(m_indexExistence != -1) {
		
//	}
//	else {
//		std::cerr << getName() << "::removeObj()::ERROR::m_indexExistence is -1\n";
//	}
	
}

/*
void ManageObjList::removeObj(int index) {
}
*/

void ManageObjList::parseXmlName(const xmlpp::Node* node, const std::string& directoryName) {
    const xmlpp::ContentNode* nodeContent = dynamic_cast<const xmlpp::ContentNode*>(node);
    std::string nodename = node->get_name();

	temp = false;
	if(nodename == "Object") {
		int temp_counter;
        std::string str_number;
        std::string str_objname;
        std::string str_filename;
        const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(node);
        const xmlpp::Element::AttributeList& attributes = nodeElement->get_attributes();
        for(xmlpp::Element::AttributeList::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter)
        {
   	        const xmlpp::Attribute* attribute = *iter;
            if(attribute->get_name() == "id") str_number = attribute->get_value();
            if(attribute->get_name() == "objname") str_objname = attribute->get_value();
            if(attribute->get_name() == "filename"){ 
				str_filename = attribute->get_value();
				temp = true;
			}
        }

		temp_counter = atoi(str_number.c_str());

		if(str_objname == m_input && temp == true){
		
			objname = str_objname;
			filename = str_filename;
			*mp_counter = temp_counter;
		
			counterOut.setBuffer(mp_counter);
			counterOut.out();
			objnameOut.setBuffer(&objname);
			objnameOut.out();
			filenameOut.setBuffer(&filename);
			filenameOut.out();
			temp = false;
			return;
		}
	}

    if(!nodeContent) {
        xmlpp::Node::NodeList list = node->get_children();  //Recurse through child nodes
        for(xmlpp::Node::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter) {
            parseXmlName(*iter, directoryName); //recursive
        }
    }
}

int ManageObjList::getIndex(const std::string& objectName) {
	if(!m_init) { std::cerr << getName() << "::getIndex()::ERROR::call setParam() first!\n"; return false; }

	int index = 0;
	int total = this->getNumberObjects();
	for(int i = 1;i<=total;i++){
		if( getObjectName(i) == objectName ) return i;
	}
	return index;
}

const std::string& ManageObjList::getObjectName(const int& index) {
	if(!m_init) { std::cerr << getName() << "::getObjectName()::ERROR::call setParam() first!\n"; }

	this->getData(index);
	return objname;
}


void ManageObjList::writeToFile(const std::string& filename) {
	std::ofstream outFile(filename.c_str());
	if(outFile.is_open()) {

		outFile << "ManageObjList" << std::endl;
		
		outFile << m_path << std::endl;
		outFile << m_filename << std::endl;
		blockToStream(&outFile, (char*)&m_index, sizeof(int));

		outFile.close();
	}
	else{
		std::cerr << getName() << "::writeToFile()::ERROR::Creating new file [" << filename << "]\n";
	}
}

void ManageObjList::readFromFile(const std::string& filename) {
	std::ifstream inFile(filename.c_str());
	if (inFile.is_open()) {

		std::string datatype;
		getline(inFile, datatype);
		if(datatype != "ManageObjList") { std::cerr << getName() << "::readFromFile()::ERROR::Data type [" << datatype << "] is not FeatureCluster!\n"; return; }

		getline(inFile, m_path);
		getline(inFile, m_filename);
		
		this->setParam(m_path, m_filename);
		
		blockFromStream(&inFile, (char*)&m_index, sizeof(int));
		
		inFile.close();
	}
	else{
		std::cerr << getName() << "::readFromFile()::ERROR::Opening file [" << filename << "]\n";
	}
}

bool ManageObjList::checkncreatefolder(const std::string& path) {
	std::string temppath = path;
	int ind = temppath.size() - 1;
	if(temppath[ind] != '/') temppath = temppath + "/";

	int returnValue;
	std::string cwd;
	char* currentworkingdirectory = new char[400];
	getcwd(currentworkingdirectory, 400);
	cwd = currentworkingdirectory;
	delete [] currentworkingdirectory;

	mode_t mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH;

	// Check the existence of the directory
	if(debug) std::cout<< getName() << "::checkncreatefolder()::Checking existence of directory [" << temppath << "]\n";
	returnValue = chdir(temppath.c_str());
	// directory exists
	if(returnValue == 0) {
		if(debug) std::cout << getName() << "::checkncreatefolder()::Directory [" << temppath << "] found\n";
		returnValue = chdir(cwd.c_str());
	}
	else{
		// directory doesnt exist, create one
		returnValue = mkdir(temppath.c_str(), mode);
		if(returnValue == 0) {
			std::cout << getName() << "::checkncreatefolder()::Creating directory [" << temppath << "] successful\n";
		}
		else{ std::cerr << getName() << "::checkncreatefolder()::ERROR::Creating directory [" << temppath << "] failed !\n"; return false; }
	}

	return true;	
}

bool ManageObjList::checkncreatelistdtdfile(const std::string& file) {
	// Check for the [list.dtd] file in the directory
	std::fstream fileptr;
	fileptr.open (file.c_str(), std::ios::in );
	if(fileptr.is_open()) {
		fileptr.close();
	}
	else {
		if(debug) std::cout << getName() << "::checkncreatelistdtdfile()::Creating [" << file << "] file\n";
		fileptr.open (file.c_str(), std::ios::out );
		fileptr << "<!ELEMENT Database (Object*)>\n";	
		fileptr << "<!ELEMENT Object EMPTY>\n";	
		fileptr << "<!ATTLIST Object id CDATA #REQUIRED>\n";	
		fileptr << "<!ATTLIST Object objname CDATA #REQUIRED>\n";	
		fileptr << "<!ATTLIST Object filename CDATA #REQUIRED>\n";	
		fileptr.close();
	}
	return true;
}

bool ManageObjList::checkncreatelistxmlfile(const std::string& file) {
	// Check for the [list.dtd] file in the directory
	std::fstream fileptr;
	fileptr.open (file.c_str(), std::ios::in );
	if(fileptr.is_open()) {
		fileptr.close();
	}
	else {
		if(debug) std::cout << getName() << "::checkncreatelistxmlfile()::Creating [" << file << "] file\n";
		fileptr.open (file.c_str(), std::ios::out );
		fileptr << "<!DOCTYPE Database SYSTEM \"list.dtd\">\n";
		fileptr << "<Database>\n";
		fileptr << "</Database>\n";
		fileptr.close();
	}
	return true;
}

