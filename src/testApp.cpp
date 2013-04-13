#include "testApp.h"
#include <iostream>
#include <GLUT/GLUT.h>



//--------------------------------------------------------------
void testApp::setup()
{
	ofBackground(0, 0, 0, 1);
	ofSetFrameRate(30);
	ofSetVerticalSync(false);
	ofEnableSmoothing();
	ofEnableAlphaBlending();
	//ofEnableNormalizedTexCoords();
	//ofHideCursor();
	
	glNewList(1, GL_COMPILE);
	glutSolidSphere(1, 40, 40);
	glEndList();
	
	boidNum = 400;
	target = ofVec3f(0, 0, 0);
	
	for (int i = 0; i < boidNum; i++)
	{
		SteeredVehicle v(ofRandom(100)-50, ofRandom(100)-50, ofRandom(100)-50);
		v.maxForce = 0.5f;
		v.inSightDist = 60.0f;
		boids.push_back(v);
        follow.push_back(ofRandom(100)>70); //if true, boid will chase the tuio target
	}
	
	cam.setDistance(600);
	
	GLfloat color[] = { 0.2, 0.2, 1.0 };
//	GLfloat color[] = { 1.0, 1.0, 1.0 };
	
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
    
    tuioClient.start(3333);
}

//--------------------------------------------------------------
void testApp::update()
{
    float tuio_x = 0;
    float tuio_y = 0;
    float tuio_z = 0;
    float tuio_vx = 0;
    float tuio_vy = 0;
    float tuio_speed = 0;
    
    tuioClient.getMessage();
    list<ofxTuioCursor*>cursorList = tuioClient.getTuioCursors();
    for(list<ofxTuioCursor*>::iterator it=cursorList.begin(); it!=cursorList.end(); it++){
        ofxTuioCursor *tcur = (*it);
        float vx = tcur->getXSpeed();
        float vy = tcur->getYSpeed();
        float speed = vx*vx+vy*vy;
        if(speed > tuio_speed){
            tuio_speed = speed;
            tuio_vx = vx;
            tuio_vy = vy;
            tuio_x = (tcur->getX()-0.5)*700;
            tuio_y = -(tcur->getY()-0.4)*500;
            tuio_z = -(tcur->getY()-0.5)*500;
        }
    }
    if(tuio_speed>0){
        cout<<tuio_x<<" "<<tuio_y<<" "<<tuio_vx<<" "<<tuio_vy<<endl;
    }
    ofVec3f target = ofVec3f(tuio_x, tuio_y, tuio_z);
    
	for (int i = 0; i < boidNum; i++)
	{
		boids[i].flock(boids);
        if(tuio_speed>0){
//            if(boids[i].tooClose(target)){
            if(distance(boids[i].position, target)<200){  //too close, flee
                boids[i].flee(target);
//            }else if(ofRandom(100)>30){
            }else if(follow[i]){
                boids[i].seek(target);
            }
            if(ofRandom(100)>90) follow[i]=(ofRandom(100)>70);  //once for a while a boid change between follow and not follow
        }
		boids[i].update();
//		boids[i].wrap(500, 500, 500);
        boids[i].bounce(700, 500, 500);
	}
}

float testApp::distance(ofVec3f &x0, ofVec3f &x1){
    return pow((x1.x-x0.x),2) + pow((x1.y-x0.y), 2) +pow((x1.z-x0.z),2)/20;
}

//--------------------------------------------------------------
void testApp::draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	
	cam.begin();
	
	for (int i = 0; i < boidNum; i++)
	{
		glPushMatrix();
		glTranslatef(boids[i].position.x, boids[i].position.y, boids[i].position.z);
		
		GLfloat color[] = { 0.2, 0.8, 0.8, 1.0 };
//		GLfloat color[] = { 0.8, 0.8, 0.8, 1.0 };
		
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
		glCallList(1);
		glPopMatrix();
	}
	
	cam.end();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    switch(key){
            case 'f':
            ofToggleFullscreen();
            break;
    }
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
    float framex, framey;
    framex =(x-1024.0/2)/1024.0*300;
    framey =-(y-768.0/2)/768.0*300;
    cout<<x<<" "<<y<<" --> "<<framex<<" "<<framey<<endl;
    for(int i=0; i<boidNum; i++){
        if(ofRandom(100)>60)
            boids[i].seek(ofVec3f(framex, framey, 100));
    }
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}