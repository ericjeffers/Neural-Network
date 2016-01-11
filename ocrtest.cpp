/*A character recognizer that uses neural nets

TODO: YOUR NAME HERE, 10/2014


NEED TO DO ----Unhardcode diffsample count and also get the list of samples - copy code from train to test
		-----evaluate the different predictions and pick the greatest one for the final answer
		-----maybe fix the O predictions
		-----add more samples

assignment and helper code by Michael Black, 10/2014

TODO:  
	YOUR CODE WILL GO IN FUNCTIONS test() AND train()
	HERE STATE WHAT STEPS YOU ACCOMPLISHED
/////
current issues
	predicitons after training are not close to one but clearly indicate the correct letter because predictions for the desired letter are 		highest 
	trained boolean in train() is not working, running 100 iterations as hard coded for testing

usage:
ocr sample X
	pops up a window, user draws an example of an X, user doubleclicks and the X is saved for later
ocr train
	builds a neural net for each letter type, trains each of them on the samples until they predict perfectly
ocr test
	pops up a window, user draws a letter and doubleclicks, the program tries to guess which letter was drawn
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <QtGui>
#include <math.h>
//graphics defs here
#include "ocr.h"
//neural network is defined here.  assumes neural.cpp is in the same directory too.
#include "neural.h"

using namespace std;
int* getSquares();

//some global vars:
//squares wide
const int GRIDWIDTH=10;
//squares tall
const int GRIDHEIGHT=20;
//the window is this many pixels wide
const int SCREENWIDTH=400;
//and pixels tall
const int SCREENHEIGHT=400;
//an array of clickable grid squares
Square* square[GRIDWIDTH][GRIDHEIGHT];
//the window background
QGraphicsScene* thescene;
OCRView* view;


//keep track of the command line input
//operation is "sample" "train" or "test"
//symbol is the letter entered when the operation is "sample"
string operation,symbol;

//called immediately on "ocr train"
//reads the images in ocrdata.txt, builds a set of neural nets, trains them, and saves the weights to perceptron.txt
void train()
{
	//read the images from file ocrdata.txt
	ifstream datafile;	//file object
	string line;		//lines will go here
	datafile.open("ocrdata.txt");	//open the file
	//stop the program if the file isn't found
	if (!datafile.is_open())
	{
		cout<<"Couldn't open ocrdata.txt"<<endl;
		return;
	}
	int linecount=0;	//keep track of how many samples are in the file
	//go through the file and just count the number of samples
	
	while(getline(datafile,line))
	{
		linecount++;
	}
	//close the file.  we'll reopen it in a moment.
	datafile.close();
	//cout<<linecount<<endl;
	//make an array to hold the samples
	int sample_input[linecount][GRIDWIDTH*GRIDHEIGHT];
	//make another array to hold the output letter for each sample
	char sample_output[linecount];

	//reopen the file
	datafile.open("ocrdata.txt");
	//for each sample,
	int diffsamplecount=0;
	char* foundlist= new char[50];
	bool matched=false;
	for(int i=0; i<linecount; i++)
	{
		matched=false;
		//read it from the file
		getline(datafile,line);
		//the first character is the output letter
		sample_output[i]=line[0];
		for (int f=0;f<50;f++)
		{	
			if (sample_output[i]==foundlist[f])
			{	
				matched=true;
			}	
		}
		if (!matched)
		{
			
			foundlist[diffsamplecount]=sample_output[i];
			diffsamplecount++;
		}
		//then a space, then a 1 or 0 for each square on the screen
		for (int j=0; j<GRIDWIDTH*GRIDHEIGHT; j++)
		{
			sample_input[i][j]=line[j+2]=='1'?1:0;
		}
	}
	

	//now we're done with ocrdata.txt
	datafile.close();

	//Perceptron* neuron=new Perceptron(200);
	
	Perceptron* perceplist[diffsamplecount];

	for (int a=0;a<diffsamplecount;a++)
		{perceplist[a]=new Perceptron(200);}


	int desired=0;//the int var holding a desired value

	for (int q=0;q<diffsamplecount;q++)
	{
		for (int n=0;n<100;n++)//train each input,output pair 100 times - this should be a boolean using trained 
		{
			bool trained=true;//flag boolean for if the training is finished for each sample input	
			for (int b=0; b<linecount;b++)
			{
				if (sample_output[b]==foundlist[q])//plug in the desired letter
					{desired=1;}//desired is true because A
			
				else
					{desired=0;}

				if (perceplist[q]->train(sample_input[b],desired)==false) //train the input array with the des. output above
				{
					trained=false;
				}
			}
			if (trained==true)
			{
				cout<<"learned it all"<<" for the letter: "<<foundlist[q]<<endl;break;
			}	
		}
	}
	
	ofstream datafile2;
	//this is where weights are written to perceptron.txt
	datafile2.open("perceptron.txt",ios::out|ios::trunc);// (ios::trunc overwrites current file, ios::app will simply add to existing file
	for (int q=0;q<diffsamplecount;q++)
	{
		//datafile2<<"start of weights for: "<< foundlist[q]<<endl;
		for(int i=0; i<perceplist[q]->size+1; i++)//loop through outputweight array up until size+1 from neuron contructor
			datafile2<<perceplist[q]->outputweight[i]<<endl;
	
		for (int b=0;b<perceplist[q]->size;b++)
		{
			for(int j=0; j<perceplist[q]->size;j++)
				datafile2<<perceplist[q]->hiddenweight[b][j]<<endl;
		}
		
	}
	
	datafile2 << endl;
	datafile2.close();
	cout << "Wrote sample to perceptron.txt" << endl;
	
	

//TODO: MAKE SOME NEURAL NETS AND TRAIN THEM HERE, THEN SAVE THE WEIGHTS TO perceptron.txt
}


//called on "ocr test", after the user draws and double-clicks the mouse
void test()
{
	//ALL NEW STUFF HERE UP UNTIL NEXT NOTE 
	ifstream datafile2;	//file object
	string line2;		//lines will go here
	datafile2.open("ocrdata.txt");	//open the file

	int linecount=0;	//keep track of how many samples are in the file
	//go through the file and just count the number of samples
	
	while(getline(datafile2,line2))
	{
		linecount++;
	}
	datafile2.close();

	datafile2.open("ocrdata.txt");
	//for each sample,
	int diffsamplecount=0;
	char* foundlist= new char[50];
	char checkingletter;
	bool matched=false;
	for(int i=0; i<linecount; i++)
	{
		matched=false;
		//read it from the file
		getline(datafile2,line2);
		//the first character is the output letter
		checkingletter=line2[0];
		for (int f=0;f<50;f++)
		{	
			if (checkingletter==foundlist[f])
			{	
				matched=true;
			}	
		}
		if (!matched)
		{
			
			foundlist[diffsamplecount]=checkingletter;
			diffsamplecount++;
		}
		
	}

	//now we're done with ocrdata.txt
	datafile2.close();

	///ERIC UP TO HERE IS ALL NEW
	//now diffsamplecount and foundlist are established and unhardcoded
	Perceptron* brain[diffsamplecount];

	for (int a=0;a<diffsamplecount;a++)
		{brain[a]=new Perceptron(200);}


	ifstream datafile;	//file object
	string line;		//lines will go here
	datafile.open("perceptron.txt");	//open the file
	//stop the program if the file isn't found
	if (!datafile.is_open())
	{
		cout<<"Couldn't open ocrdata.txt"<<endl;
		return;
	}
	int lineinfile=0;
	for (int q=0;q<diffsamplecount;q++)
	{	
		
		for(int i=0; i<brain[q]->size+1; i++)//loop through outputweight array up until size+1 from neuron contructor
		{
		datafile>>brain[q]->outputweight[i];
		lineinfile++;
		}
	
		for (int b=0;b<brain[q]->size;b++)
		{
			for(int j=0; j<brain[q]->size;j++)
				{
				datafile>>brain[q]->hiddenweight[b][j];
				lineinfile++;
				}
		}
	}

	int* predictionSquares;
	float prediction=0;
	float highweight=-10; //set to arbitrary low number to be quickly replaced
	//string letterpred;
	for (int v=0;v<diffsamplecount;v++)
	{
		predictionSquares=getSquares();
		prediction=brain[v]->getRawPrediction(predictionSquares);
		if (prediction > highweight) //find the highest weight comparatively
		{
			highweight = prediction;
			//letterpred = brain[v];
		}
	}
	
	//weights are saved into testneuron by this point
	//int* predictionSquares=getSquares();
	///float prediction=testneuron->getRawPrediction(predictionSquares);
	//cout<<"the Prediction is:..." << prediction<<endl;
	
	datafile.close();
	cout<<"the Prediction for " <<foundlist[v]<< " is " << prediction<<endl;
	cout<<highweight<<endl;
	
	
//TODO: MAKE SOME NEURAL NETS, READ THE WEIGHTS FROM A FILE perceptron.txt, USE THE NEURAL NETS TO IDENTIFY THE LETTER
}

//read the contents of the grid and save them to the end of ocrdata.txt
void saveSample()
{
	ofstream datafile;
	datafile.open("ocrdata.txt",ios::out|ios::app);
	datafile << symbol << " ";
	int* s=getSquares();
	for(int i=0; i<GRIDWIDTH*GRIDHEIGHT; i++)
		datafile<<s[i];
	datafile << endl;
	datafile.close();
	cout << "Wrote sample to ocrdata.txt" << endl;
}

//determines what function is called when the user double clicks the window, based on the command line input
void doOperation()
{
	if(operation.compare("sample")==0)
		saveSample();
	else if (operation.compare("train")==0)
		train();
	else if (operation.compare("test")==0)
		test();
}

//returns an integer array of 0s and 1s that represents what the user drew on the window
int* getSquares()
{
	int* s=new int[GRIDWIDTH*GRIDHEIGHT];
	for(int i=0; i<GRIDWIDTH; i++)
		for(int j=0; j<GRIDHEIGHT; j++)
			s[i*GRIDHEIGHT+j]=square[i][j]->selected?1:0;
	return s;
}

//constructor for a grid square
Square::Square(int x, int y)
{
	xpos=x; ypos=y;
	xcoor1=x*SCREENWIDTH/GRIDWIDTH;
	ycoor1=y*SCREENHEIGHT/GRIDHEIGHT;
	xcoor2=(x+1)*SCREENWIDTH/GRIDWIDTH;
	ycoor2=(y+1)*SCREENHEIGHT/GRIDHEIGHT;
	selected=FALSE;
}

//location of the square on the screen, used for rendering
QRectF Square::boundingRect() const
{
	return QRectF(xcoor1,ycoor1,xcoor2-xcoor1,ycoor2-ycoor1);
}

//called when the user selects the square
void Square::click()
{
	selected=TRUE;
	update();
}

//render the square on the screen
void Square::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
	QColor color;
	if(selected)
	{
		//if the user clicked it, draw it purple
		color.setBlue(100); color.setRed(100); color.setGreen(0);
	}
	else
	{
		//otherwise draw it white
		color.setBlue(255); color.setRed(255); color.setGreen(255);
	}
	painter->setBrush(color);
	painter->drawRect(xcoor1,ycoor1,xcoor2-xcoor1,ycoor2-ycoor1);
}

OCRView::OCRView(QGraphicsScene *scene, QWidget* parent):QGraphicsView(scene, parent)
{
	//the mouse is initially assumed to be unpressed
	isPressed=FALSE;
}

//the mouse has been pressed or dragged
//figure out which grid square was selected and call it
void squareClickEvent(QMouseEvent *event)
{
	int x=(event->x()*GRIDWIDTH/SCREENWIDTH);
	int y=(event->y()*GRIDHEIGHT/SCREENHEIGHT);
	if(x<0 || y<0 || x>=GRIDWIDTH || y>=GRIDHEIGHT) return;
	square[x][y]->click();
}

//if the mouse button is down and the user moves over a square, select it
void OCRView::mouseMoveEvent(QMouseEvent *event)
{
	if(!isPressed) return;
	squareClickEvent(event);
}

//if the user presses the mouse on a square, select it
void OCRView::mousePressEvent(QMouseEvent *event)
{
	isPressed=TRUE;
	squareClickEvent(event);
}

//the mouse is released, stop selecting squares
void OCRView::mouseReleaseEvent(QMouseEvent *event)
{
	isPressed=FALSE;
}

//double click means that we're done drawing
void OCRView::mouseDoubleClickEvent(QMouseEvent *event)
{
	if(event->button() == Qt::RightButton)
	{
		// a right-button double click just exits the program
		exit(0);
	}
	if(event->button() == Qt::LeftButton)
	{
		//a left-button double click saves the sample or tests, depending on the command line parameters
		doOperation();
		exit(0);
	}
}

//program starts here
int main(int argc, char **argv)
{
	//save the command line arguments
	if(argc>=2)
		operation=argv[1];
	if(argc==3)
		symbol=argv[2];
	else
		symbol=" ";
	//if the user didn't give any, print out some help and stop
	if(argc==1)
	{
		cout << "Usage: " << endl;
		cout << " ocr sample A" << endl;
		cout << " ocr train" << endl;
		cout << " ocr test" << endl;
		exit(0);
	}

	//seed the random number generator
	qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));

	//we don't need to make a window if the user selects "train"
	if(operation.compare("train")==0)
	{
		train();
		exit(0);
	}

	//make a window
	QApplication app(argc,argv);

	thescene=new QGraphicsScene();
	thescene->setSceneRect(0,0,SCREENWIDTH+100,SCREENHEIGHT+100);

	//add in all the squares
	for(int i=0; i<GRIDWIDTH; i++)
	{
		for(int j=0; j<GRIDHEIGHT; j++)
		{
			square[i][j] = new Square(i,j);
			thescene->addItem(square[i][j]);
		}
	}

	view=new OCRView(thescene);
	view->setWindowTitle("Neural OCR");
	view->resize(SCREENWIDTH+100,SCREENHEIGHT+100);
	view->show();
	view->setMouseTracking(true);

	//make the window visible.  we're done with setup.
	return app.exec();
}
