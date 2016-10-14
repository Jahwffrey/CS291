#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/GLUT.h>


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


void drawFunc(){
	Mat readImage = imread(fileNames[0],CV_LOAD_IMAGE_COLOR);
	Mat backImage = Mat(readImage);
	flip(readImage,backImage,0);

	glDrawPixels(backImage.size().width,backImage.size().height,GL_BGR,GL_UNSIGNED_BYTE,backImage.ptr());
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
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, 1, 1, 20);
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

     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
     glMatrixMode(GL_MODELVIEW);
     glLoadIdentity();
     gluLookAt(0.0, 2.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
     glPushMatrix();

     drawFunc();

     glPopMatrix();
     glutSwapBuffers();
}

void init(){
	glClearColor(0.0, 0.0, 0.0, 0.0);
        glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
        glEnable(GL_NORMALIZE);
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

	//start openGL
	glutInit(&argc, &argv[0]);
        glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
    	glutInitWindowPosition( 100, 100 );
    	glutInitWindowSize( width, height );
    	glutCreateWindow( "LETS DO THIS" );    

	init();

	capture = VideoCapture(cameraNumber);

	
	/*for(int i = 0;i < 48;i++){
		std::cout << "("<< objPoints[0][i].x << "," << objPoints[0][i].y << "," << objPoints[0][i].z << ")"; 
		if(i % 8 == 7) std::cout << "\n";
	}*/

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
		
				//imshow(fileNames[i],calibImage);
				bool foundBoard = findChessboardCorners(calibImage,boardSize,corners);
				if(foundBoard){
					imgPoints.push_back(corners);
					/*for(int j = 0;j < corners.size();j++){
						for(int k = -5;k < 6;k++){
							Vec3b & col = calibImage.at<Vec3b>(corners[j].y+k,corners[j].x);
							col[0] = 0;   //b
							col[1] = 0;   //g
							col[2] = 255; //r
						}
					}
					imshow(fileNames[i],calibImage);*/
				} else {
					std::cout << "Failed to find chessboard in " << fileNames[i] << "\n"; 
				}
			}
		}

		/*for(int i = 0;i < 48;i++){
			std::cout << "("<< imgPoints[0][i].x << "," << imgPoints[0][i].y << ",0)"; 
			if(i % 8 == 7) std::cout << "\n";
		}*/

		///float camMatrix[3][3];
		//OutputArray camMatrix = create(3,3,float);
		Mat camMatrix = Mat::eye(3,3,CV_64F);
		Mat distCoeffs = Mat::zeros(4,1,CV_64F);
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



	/*while(true){
		capture >> captFeed;
		imshow("Capture",captFeed);
	}*/
	
	glutMainLoop();

	return 0;
}
