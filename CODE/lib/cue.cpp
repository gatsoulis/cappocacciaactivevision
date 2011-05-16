#include "cue.h"



Cue::Cue(std::string name) : Module(name) {

/*
	cvImageIn.setModule(this);
	cvRImageIn.setModule(this);
	cvGrayImageIn.setModule(this);
	cvSalImageIn.setModule(this);
	winnerIn.setModule(this);
	trackIn.setModule(this);
	roiIn.setModule(this);
*/

	m_init = false;
	m_threshold = 0.6;
	m_tfacs = 0.1;
	mp_cvoutputimg = NULL;

}

Cue::~Cue(){
//	if(mp_cvoutputimg) delete mp_cvoutputimg;
}

double Cue::adapt_profiled(int times) {
  clock_t tv1, tv2;
  tv1 = clock();
  int i;
  for (i=0; i<times; i++) adapt();
  tv2 = clock();
  return (double)((tv2 - tv1))*1000.0/(double)CLOCKS_PER_SEC/(double)times;
}

long Cue::adapt_profiled_ms() {
	unsigned long int startTime = getMsTime();
	adapt();
	startTime = getMsTime() - startTime;
	return startTime;
}

long Cue::adapt_profiled_ms_stdout() {
	long startTime = execute_profiled_ms();
	std::cout << getName() << " took " << startTime << " ms to execute.\n";
	return startTime;
}

void Cue::print(){
	std::cout << getName() << "::print()\n";
	std::cout << "tfacs = " << m_tfacs << ", thres = " << m_threshold << "\n";
	std::cout << "winnerPos = ( " << m_track.winnerPos.x << ", " << m_track.winnerPos.y << ")\n";
	std::cout << "winnerSize = ( " << m_track.winnerSize.width << ", " << m_track.winnerSize.height << ")\n";
	std::cout << "winnerRect = ( " << m_track.winnerRect.x << ", " << m_track.winnerRect.y << ", " << m_track.winnerRect.width << ", " << m_track.winnerRect.height << ")\n";
	std::cout << "reliability = " << m_track.reliability << "\n";
	std::cout << "imageSize = ( " << m_track.imageSize.width << ", " << m_track.imageSize.height << ")\n";
}

float Cue::computequality(CVImage* overallsaliencymap, TrackData* track) {
	float qual = 0.0;

	if(!track) { return qual; }

	if(track->reliability < m_threshold) { return qual; }

	IplImage* newsal = mp_cvoutputimg->ipl;
	int width = newsal->width;
	int height = newsal->height;
	float avgsal = (cvAvg(newsal)).val[0];

	int posx, posy, count = 0;

	float avglocal = 0.0;
	for (int x = -5; x<=5; x++) {
		for (int y=-5; y<=5; y++) {
			posx = track->winnerPos.x + x;
			posy = track->winnerPos.y + y;
			if(posx >=0 && posx < width && posy >=0 && posy < height){
				double temp = cvGetReal2D(newsal, posy, posx);
				avglocal += (float)(temp);
				count++;
			}
		}
	}
	if(count > 0) avglocal = avglocal / (float)count;
	else avglocal = 0.0;

	//cvMinMaxLoc( newsal, &minval, &maxval, &minloc, &maxloc);

	if (avglocal >= 0.0 && avglocal > avgsal) qual = avglocal - avgsal;
	else qual = 0.0;

/*
	IplImage* newSal;
	if (objectFound == true) {
		qualitySum = 0.0;
		for (int i=0; i<cueCount; i++) {
			if ((newSal = cue[i]->salImageOut.getBuffer()) ) {
				float avgsal = (cvAvg(newSal)).val[0];
				float avglocal = 0.0;
				int count = 0, tempx, tempy;
				float* data = (float*)(newSal->imageData);
			    int width = newSal->width;
				for (int x = -5; x<=5; x++) {
					for (int y=-5; y<=5; y++) {
						tempx = m_track.winnerPos.x + x;
						tempy = m_track.winnerPos.y + y;
						if(tempx >=0 && tempx < newSal->width && tempy >=0 && tempy < newSal->height){
							avglocal = avglocal + data[tempx + tempy * width];
							count = count + 1;
						}
					}
				}
				if(count > 0) avglocal = avglocal / (float)count;
				else avglocal = 0.0;
//				cvMinMaxLoc( newSal, mp_minVal, mp_maxVal, mp_minLoc, mp_maxLoc);
				if (avglocal>=0 && avglocal > (avgsal + 0.1)) qualitySum += *quality[i] = avglocal - avgsal;
				else *quality[i] = 0.0;
			}
		}
		for (int i=0; i<cueCount; i++) {
			if (qualitySum>0)  {
				*quality[i] /= qualitySum;
			}
			else *quality[i] = 0.0;
		}
	}
	else {
		qual = 0.0;
	}
*/

	return qual;
}


