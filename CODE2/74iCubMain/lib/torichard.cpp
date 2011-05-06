#include "torichard.h"



ToRichard::ToRichard(std::string name) : Module(name) {
	
	std::cout << "Destructing " << getName() << "...";

	regionsIn.setModule(this);


	debug = false;

	std::cout << "finished.\n";
}

ToRichard::~ToRichard() {
	std::cout << "Destructing " << getName() << "...";
	std::cout << "finished.\n";
}
void ToRichard::release(){
	if(array)
		free(array);
}

void ToRichard::execute() {

	CSCEllipticRegions* region = regionsIn.getBuffer();

	if(!region) {
		std::cout <<"\nNO REGION\n";
		return;
	}



	if(debug)
	{
		std::cout<<"\n"<<getName()<<"start coping region\n";
		cvWaitKey(0);
	}

	if(region) {
		int height = region->getCurrentSize();
		int width = region->getWidth();
		arraysize = &height;
		array = (int*)malloc(2*sizeof(int)*height);
		float* data = region->getData();
		for(unsigned int i = 0;i<height;i++) {
			float posx = data[i*width];
			float posy = data[i*width+1];
			array[i*2]=(int)posx;
			array[i*2+1]=(int)posy;

		}
		if(debug)
		{
			for(int i = 0; i<2*height;i++)
			{
				std::cout<<getName()<<"::array["<<i<<"]="<<array[i]<<"\n";

			}
			std::cout<<"\n"<<getName()<<"::arraysize: "<<*arraysize*2<<"\n";
			cvWaitKey(0);
		}

		arrayOut.setBuffer(array);
		arrayOut.out();
		sizearrayOut.setBuffer(arraysize);
		sizearrayOut.out();

	}

}

