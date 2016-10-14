#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/GLUT.h>
#include <math.h>

using namespace cv;

char *fileNames [] = {"../boards/frame0054.jpg",	
	"../boards/frame0124.jpg",
	"../boards/frame0174.jpg",
	"../boards/frame0249.jpg",
	"../boards/frame0070.jpg",
	"../boards/frame0141.jpg",
	"../boards/frame0190.jpg",
	"../boards/frame0274.jpg",
	"../boards/frame0107.jpg",
	"../boards/frame0161.jpg",
	"../boards/frame0228.jpg"};

Mat camMatrix = Mat::eye(3,3,CV_64F);
Mat distCoeffs = Mat::zeros(4,1,CV_64F);

void drawFunc(){
	double fx = camMatrix.at<double>(0,0);
	double fy = camMatrix.at<double>(1,1);
	double cx = camMatrix.at<double>(2,0);
	double cy = camMatrix.at<double>(2,1);

	Mat readImage = imread(fileNames[0],CV_LOAD_IMAGE_COLOR);
	Mat backImage = readImage.clone();
	undistort(backImage,readImage,camMatrix,distCoeffs);
	flip(readImage,backImage,0);

	double imWidth = backImage.size().width;
	double imHeight = backImage.size().height;
	double fovy = 2 * atan((imHeight/2)/fy);

	glDrawPixels(backImage.size().width,backImage.size().height,GL_BGR,GL_UNSIGNED_BYTE,backImage.ptr());
	
	glViewport(0,0,imWidth,imHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy,imWidth/imHeight,0.01,100);
	//gluPerspective(60,backImage.size().width/backImage.size().height,0.01,100);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(0,0,5,0,0,0,0,1,0);

	glPushMatrix();

	//glutSolidSphere(1,200,200);

	glBegin(GL_TRIANGLES);
		glVertex3f(1,0,0);
		glVertex3f(0,1,0);
		glVertex3f(0,0,1);
	glEnd();

	glPopMatrix();
}

void mouseFunc(int button, int state, int x, int y){

}

void keyFunc(unsigned char k, int x, int y){
	switch(k){

	}
}

void reshape(int w, int h){
	//win_width = w;
	//win_height = h;
	glViewport(0, 0, w, h);
}

void idle(){
	glutPostRedisplay();
}


void display(){
	// check if there have been any openGL problems
      GLenum errCode = glGetError();
     if (errCode != GL_NO_ERROR)
     {
	     const GLubyte* errString = gluErrorString(errCode);
	     fprintf( stderr, "OpenGL error: %s\n", errString );
     }

     glClear(GL_COLOR_BUFFER_BIT);// | GL_DEPTH_BUFFER_BIT);

     drawFunc();

     glutSwapBuffers();
     glutPostRedisplay();
}

void init(){
	glClearColor(0.0, 0.0, 0.0, 0.0);
        glShadeModel(GL_SMOOTH);
	//glEnable(GL_DEPTH_TEST);
        glEnable(GL_NORMALIZE);
	glMatrixMode(GL_MODELVIEW);
        //glEnable(GL_TEXTURE_2D);
        //glEnable(GL_LIGHTING);
			    
        glutDisplayFunc(display);
	glutReshapeFunc(reshape);
        glutIdleFunc(idle);
        glutKeyboardFunc(keyFunc);
        glutMouseFunc(mouseFunc);
			        
        //glLightfv(GL_LIGHT1,GL_AMBIENT,lightAmbient);
	//glLightfv(GL_LIGHT1, GL_DIFFUSE, lightDiffuse);
        //glLightfv(GL_LIGHT1, GL_POSITION, lightLoc);
        //glEnable(GL_LIGHT1);
}

int main(int argc,char** argv){
	int cameraNumber;
	int doCalib;
	int width = 640;
	int height = 480;
	const int calibFilesNum = 11;

	CvSize boardSize = {8,6};//new CvSize(8,6);

	vector< vector<Point2f> > imgPoints;
	vector< vector<Point3f> > objPoints;

	Size calibSize;
	VideoCapture capture;
	Mat captFeed;

	if(argc != 3){
		std::cout << "Please provide where or not to calibrate (0 or 1) and camera number\n";
		return 0;
	} else {
		doCalib = atoi(argv[1]);
		cameraNumber = atoi(argv[2]);
	}

	
	capture = VideoCapture(cameraNumber);

	if(doCalib){
		//Set up object points
		for(int im = 0;im < calibFilesNum;im++){
			vector<Point3f> newVect;
			objPoints.push_back(newVect);
			for(int i = 0;i < 6;i++){
				for(int j = 0;j < 8;j++){
					Point3f val = Point3f(j,i,0);
					objPoints[im].push_back(val);
				}
			}
		}
		//Calibrate from image files checkerboard
		for(int i = 0;i < calibFilesNum;i++){
			Mat calibImage;
			calibImage = imread(fileNames[i],CV_LOAD_IMAGE_COLOR);
			vector<Point2f> corners;

			if(!calibImage.data){
				std::cout << "Failed to read " << fileNames[i] << "!!\n";
			} else {
				if(i == 0){
					calibSize = calibImage.size();
				}
				bool foundBoard = findChessboardCorners(calibImage,boardSize,corners);
				if(foundBoard){
					imgPoints.push_back(corners);
				} else {
					std::cout << "Failed to find chessboard in " << fileNames[i] << "\n"; 
				}
			}
		}

		vector<Mat> rvecOut;
		vector<Mat> tvecOut;
		calibrateCamera(objPoints,imgPoints,calibSize,camMatrix,distCoeffs,rvecOut,tvecOut);

		//Reprojection error
		double totalError;
		double avgError;
		int pointNum;
		for(int i = 0;i < calibFilesNum;i++){
			double error;
			vector<Point2f> immPts;
			projectPoints(Mat(objPoints[i]),rvecOut[i],tvecOut[i],camMatrix,distCoeffs,immPts);
			error = norm(Mat(imgPoints[i]),Mat(immPts),CV_L2);
			totalError += (error * error);
			pointNum += objPoints[i].size();
		}
	
		avgError = std::sqrt(totalError/pointNum);
		std::cout << "Total error: " << totalError << "\nAvg Error: " << avgError << "\n";
		std::cout << "Camera Intrinsics:\n";
		for(int i = 0;i < 3;i++){
			for(int j = 0;j < 3;j++){
				std::cout << camMatrix.at<double>(i,j) << "\n";
			}
		}
		for(int i = 0;i < 4;i++){
			std::cout << distCoeffs.at<double>(i,0) << "\n";
		}
	}

	//start openGL
	glutInit(&argc, &argv[0]);
        glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
    	glutInitWindowPosition( 100, 100 );
    	glutInitWindowSize( width, height );
    	glutCreateWindow( "LETS DO THIS" );    

	init();

	glutMainLoop();

	return 0;
}
