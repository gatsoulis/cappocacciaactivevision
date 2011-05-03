#ifndef Module_H
#define Module_H

#include <iostream>
#include <string>
#include <sys/timeb.h>



//! class Module  -- Abstract Module. Use this as base class for creating modules
/*! class Module  -- Abstract Module. Use this as base class for creating modules

    for creating a module you have to create a new class and inherit this base class.

    Very important for derived classes:

    -INCLUDES:    include module.h, inputconnector.h and outputconnector.h (depends on what connectors you use)
    -NAMESPACE:   wrap "namespace csc { ... }" around your header file, type "using namespace csc;" above function definitions in source file
    -INHERITANCE: derive your class from base class Module, declarate constructor ClassName(std::string name="Default Name"); in header
                  and define constructor ClassName(std::string name="Default Name") : Module(name); in source file.
                  Call the superclass constructor with ": Module(name)"
    -COMPUTATION: redefine the execute() function

    Take a look at the example and copy a few lines.
    Just think of different connectors and a useful execute function...
    
    ---------
     EXAMPLE
    ---------
    
    #include <string>
    #include <iostream>
    #include "module.h"
    #include "outputconnector.h"
    #include "inputconnector.h"
    #include "cv.h"     
    
    namespace csc {
      class EmptyModule : public Module
      {
        EmptyModule(std::string name="Default Name"); // use name=<module name>
        ~ImageDisplay();
        InputConnector<IplImage*> valueIn; // Connectors are templates. 
        OutputConnector<int> valueOut; // Use pointers at structures/class instead of structures/classes
        void execute();
      }
    }
      
    using namespace csc;
    EmptyModule::EmptyModule(std::string name) : Module(name) // this constructor call is important to pass the name
    {
      std::cout<<"Constructing Empty Module module '"<<name<<"'...";
      valueIn.setModule(this); // needed for triggering connections (see class OutputConnector for more details)
      // personal init stuff...
      std::cout << "done\n";
    }

    EmptyModule::~EmptyModule() {}
   
    void EmptyModule::execute()
    {
      // do something
      valueOut.setBuffer(valueIn.getBuffer*2);
    }
    
    written 10/2003 by Boris Lau with code borrowed from Erik Murphy-Chutorian and Hyundo Kim
*/
	class Module {

		public:

			//! Module(std::string name="Unnamed module")  -- the standard constructor
			/*! Module(std::string name="Unnamed module")  -- the standard constructor
				This constructor should be called by all derived module using a special name. */
			Module(std::string name="Unnamed module");

			//! virtual ~Module()  -- the destructor
			virtual ~Module();

			//! virtual void execute()=0  -- this function must be redefined in every derived module
			/*! virtual void execute()=0  -- this function must be redefined in every derived module
				This function must contain the computation that every module does.
				It will be called from an outer loop or thread or by the execute_profiled() function.
				When you derive a module from this class you have to put some in- and outputconnectors into your module.
				You can get the input data from any InputConnector dataIn by using dataIn.getBuffer()
				store your data in an OutputConnector<T> by using dataOut.setBuffer(T data)
				Notice: Data in dataIn is const, so take a copy if you wish to modify it! */
			virtual void execute()=0;
			virtual void execute2(){}
			virtual void execute3(){}
			virtual void execute4(){}

			//! long int execute_profiled()  -- call this function for getting computation time after execution in milliseconds
			/*! long int execute_profiled()  -- call this function for getting computation time after execution in milliseconds
				the clock-value before and after execution will be compared and returned in milliseconds.
				Notice: The resolution depends on your system libraries - it can be 1 to 50 milliseconds. */
			double execute_profiled(int times=1);
			long execute_profiled_ms();
			long execute_profiled_ms_stdout();

			//! void setName(string name)  -- sets the name of the module
			/*! void setName(string name)  -- sets the name of the module
				use this function only when you can't specifiy the name in the constructor.
				Try to avoid this, window names will already be set in most cases. */
			void setName(std::string name);

			//! string getName()  -- returns the name of the module
			std::string getName();
	
			unsigned long int getMsTime();

		private:

			std::string m_name;
			timeb m_timeStruct;

	};

#endif
