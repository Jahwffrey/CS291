#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/GLUT.h>
#include <math.h>
#include <fstream>

#define RADS 57.2958

using namespace cv;

//eww global
Mat camMatrix = Mat::eye(3,3,CV_64F);
Mat distCoeffs = Mat::zeros(4,1,CV_64F);
vector<Point3f> objPoints;
CvSize boardSize = {8,6};
CvSize imgSize = {640,480};
VideoCapture capture;

void drawFunc(){
	double fx = camMatrix.at<double>(0,0);
	double fy = camMatrix.at<double>(1,1);
	double cx = camMatrix.at<double>(2,0);
	double cy = camMatrix.at<double>(2,1);

	Mat readImage;
	capture >> readImage;
	Mat backImage;
	resize(readImage,backImage,imgSize);
	undistort(backImage,readImage,camMatrix,distCoeffs);
	//undistort(readImage,backImage,camMatrix,distCoeffs);
	//resize(backImage,readImage,imgSize);
	flip(readImage,backImage,0);

	double imWidth = imgSize.width;
	double imHeight = imgSize.height;
	double fovy = 2 * RADS * atan(imHeight/(2.0 * fy));
	double aspect = (fy/fx) * (imWidth/imHeight);

	glDrawPixels(backImage.size().width,backImage.size().height,GL_BGR,GL_UNSIGNED_BYTE,backImage.ptr());
	//Find camera extrinsics
	//Find the board
	vector<Point2f> imgCoords;
	bool fndBrd = findChessboardCorners(backImage,boardSize,imgCoords);
	
	glViewport(0,0,imWidth,imHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if(fndBrd){
		Mat rvecs;
		Mat tvecs;
		Mat rotMat;
		Mat endMat;
		solvePnP(Mat(objPoints),imgCoords,camMatrix,distCoeffs,rvecs,tvecs);
		rvecs.at<double>(1,0) = -rvecs.at<double>(1,0);
		Rodrigues(rvecs,rotMat);

		//endMat = (Mat_<double>(4,4) << rotMat.at<double>(0,0),rotMat.at<double>(1,0),rotMat.at<double>(2,0),tvecs.at<double>(0,0),
		/*double multMat[16] =  {rotMat.at<double>(0,0),rotMat.at<double>(1,0),rotMat.at<double>(2,0),tvecs.at<double>(0,0),
					       rotMat.at<double>(0,1),rotMat.at<double>(1,1),rotMat.at<double>(2,1),tvecs.at<double>(1,0),
					       rotMat.at<double>(0,2),rotMat.at<double>(1,2),rotMat.at<double>(2,2),tvecs.at<double>(2,0),
					       0,0,0,1};
		*/


		double multMat[16] =  {	rotMat.at<double>(0,0),rotMat.at<double>(0,1),rotMat.at<double>(0,2),0,
				    	rotMat.at<double>(1,0),rotMat.at<double>(1,1),rotMat.at<double>(1,2),0,
				    	rotMat.at<double>(2,0),rotMat.at<double>(2,1),rotMat.at<double>(2,2),0,
					tvecs.at<double>(0,0), -tvecs.at<double>(1,0), tvecs.at<double>(2,0),1};
		


		gluPerspective(fovy,aspect,0.01,100);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	
		
		glScalef(1.0,-1.0,-1.0);	
		
		/*glTranslatef(tvecs.at<double>(0,0),-tvecs.at<double>(1,0),tvecs.at<double>(2,0));
		glRotatef(-rvecs.at<double>(0,0) * RADS,1,0,0);
		glRotatef(rvecs.at<double>(1,0) * RADS,0,1,0);
		glRotatef(-rvecs.at<double>(2,0) * RADS,0,0,1);*/

		//glMultMatrixd(&endMat);
		glMultMatrixd(multMat);

		

		glPushMatrix();

		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
		
		glPushMatrix();
		glRotatef(-90,1,0,0);
		glTranslatef(2.5,1.5,-3.5);
		glutSolidTeapot(3);
		/*for(int i = 0;i < 6;i++){
			glPushMatrix();
			for(int j = 0;j < 8;j++){
				glutSolidSphere(0.2,20,20);	
				glTranslatef(1.0,0,0);
			}
			glPopMatrix();
			glTranslatef(0,1.0,0);
		}*/
		glPopMatrix();
	
		glPopMatrix();
	 } else {

	 }

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
	Mat captFeed;

	if(argc != 2){
		std::cout << "Please provide camera number\n";
		return 0;
	} else {
		cameraNumber = atoi(argv[1]);
	}

	glutInit(&argc, &argv[0]);
        glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
    	glutInitWindowPosition( 100, 100 );
    	glutInitWindowSize( imgSize.width, imgSize.height);

	capture = VideoCapture(cameraNumber);

	//Load camera intrinsics
	std::ifstream file("../intrins.txt");
	if(!file.is_open()){
		std::cerr << "camera intrinsics file 'intrins.txt' required in folder above\n";
		exit(1);
	}

	//Set up object points
	
	for(int i = 0;i < 6;i++){
		for(int j = 0;j < 8;j++){
			Point3f val = Point3f(j,i,0);
			objPoints.push_back(val);
		}
	}
	
	double param = 0;
	for(int i = 0;i < 3;i++){
		for(int j = 0;j < 3;j++){
			file >> param;
			camMatrix.at<double>(i,j) = param;
		}
	}
	for(int i = 0;i < 4;i++){
		file >> param;
		distCoeffs.at<double>(i,0) = param;
	}

	file.close();

	//start openGL
	glutCreateWindow( "LETS DO THIS" );    

	init();

	glutMainLoop();

	return 0;
}
