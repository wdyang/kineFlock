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

    backdrop_r = 0.4;
    backdrop_g = 0.6;
    backdrop_b = 1.0;
    backdrop_a = 0.2;
    
//    drawGUI=true;
//    setGUI();
//    gui->setDrawBack(false);
    
    
	glNewList(1, GL_COMPILE);
	glutSolidSphere(1, 40, 40);
	glEndList();
	
	boidNum = 10;
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
    cam.disableMouseInput();
//    cam.setNearClip(50);
//    cam.setFarClip(1000);
    fbo.allocate(ofGetWidth(), ofGetHeight());
    backdrop.loadImage("images/clouds.jpg");
	
    float backdroplight = 1.0;
	GLfloat color[] = { backdroplight, backdroplight, backdroplight };
//	GLfloat color[] = { 1.0, 1.0, 1.0 };
	
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);

	glLightfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
    glLightfv(GL_LIGHT0, GL_AMBIENT, color);
    
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
//	fbo.begin(false);
    //    ofClear(0,0,0);
//    ofClear(0,0,0,1);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//    ofEnableAlphaBlending();
//    ofSetColor(2, 2, 2);
//    backdrop_a=0.1;
    GLfloat color[] = { backdrop_r , backdrop_g, backdrop_b, backdrop_a};
//    GLfloat color[] = { 1, 0.1, 0.1, 0.2};
    
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
    backdrop.draw(0,0, ofGetWidth(), ofGetHeight());
//    ofDisableAlphaBlending();
    cam.begin();
	glEnable(GL_DEPTH_TEST);
//    ofScale(1,-1,1);
    glDepthFunc(GL_ALWAYS);
//    glDisable(GL_DEPTH_TEST);
//    ofDrawGrid();
	
	for (int i = 0; i < boidNum; i++)
	{
		glPushMatrix();
		glTranslatef(boids[i].position.x, boids[i].position.y, boids[i].position.z);
		
//		GLfloat color[] = { 0.2, 0.8, 0.8, 1.0 };
		GLfloat color[] = { 1, 1, 1, 1.0 };
//		GLfloat color[] = { 0.8, 0.8, 0.8, 1.0 };
		
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
		glCallList(1);
		glPopMatrix();
	}
//	glDepthFunc(GL_LESS);
	cam.end();
//    ofSetColor(255, 255, 255);
//    ofEnableAlphaBlending();
//    backdrop.draw(0,0);
//    ofDisableAlphaBlending();

//    fbo.end();
//    fbo.draw(0, 0, ofGetWidth(), ofGetHeight());
}

float increment(float val){
    float delta = 0.01;
    val += delta;
    return (val>1) ? 1 : val;
}

float decrement(float val){
    float delta = 0.01;
    val -= delta;
    return (val<0) ? 0 : val;
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    float delta = 0.01;
    switch(key){
        case 'f':
            ofToggleFullscreen();
            break;
        case 'h':
            gui->toggleVisible();
            break;
        case 'r':
            backdrop_r = increment(backdrop_r);
            break;
        case 'R':
            backdrop_r = decrement(backdrop_r);
            break;
        case 'g':
            backdrop_g = increment(backdrop_g);
            break;
        case 'G':
            backdrop_g = decrement(backdrop_g);
            break;
        case 'b':
            backdrop_b = increment(backdrop_b);
            break;
        case 'B':
            backdrop_b = decrement(backdrop_b);
            break;
        case 'a':
            backdrop_a = increment(backdrop_a);
            break;
        case 'A':
            backdrop_a = decrement(backdrop_a);
            break;
    }
    cout<<backdrop_r<<" "<<backdrop_g<<" "<<backdrop_b<<" "<<backdrop_a<<endl;
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
    float framex, framey;
    float w, h;
    w=ofGetWidth();
    h=ofGetHeight();
    framex =(x-w/2)*10;
    framey =-(y-h/2)*10;
    cout<<x<<" "<<y<<" --> "<<framex<<" "<<framey<<endl;
    for(int i=0; i<boidNum; i++){
        if(ofRandom(100)>30)
            boids[i].seek(ofVec3f(framex, framey, 100));
    }
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
    float framex, framey;
    float w, h;
    w=ofGetWidth();
    h=ofGetHeight();
    framex =(x-w/2)*0.7;
    framey =-(y-h/2)*0.7;

	boidNum++;
	target = ofVec3f(0, 0, 0);
	
    SteeredVehicle v(ofRandom(20)-10+framex, ofRandom(20)-10+framey, ofRandom(100)-50+100);
    v.maxForce = 0.5f;
    v.inSightDist = 60.0f;
    boids.push_back(v);
    follow.push_back(ofRandom(100)>70); //if true, boid will chase the tuio target
    cout<<"added a boid"<<endl;

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
void testApp::setGUI(){
    backdrop_r=1; backdrop_g=1; backdrop_b=1; backdrop_a=1;
    gui = new ofxUICanvas(0, 0, 255, ofGetHeight());
    gui->addToggle("drawGUI", drawGUI);
    gui->addSlider("backdrop_r", 0, 1, backdrop_r);
    gui->addSlider("backdrop_g", 0, 1, backdrop_g);
    gui->addSlider("backdrop_b", 0, 1, backdrop_b);
    gui->addSlider("backdrop_a", 0, 1, backdrop_a);
    ofAddListener(gui->newGUIEvent, this, &testApp::guiEvent);
}
void testApp::guiEvent(ofxUIEventArgs &e){
    string name=e.widget->getName();
    cout<<"gui: "<<name<<endl;
    int kind=e.widget->getKind();
    if(name=="drawGUI"){
        ofxUIToggle *toggle = (ofxUIToggle *)e.widget;
        drawGUI = toggle->getValue();
        cout<<"drawGUI is "<<drawGUI<<endl;
    }if(name=="backdrop_r"){
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        backdrop_r = slider->getValue();
        cout<<"backdrop_r: "<<backdrop_r<<endl;
    }if(name=="backdrop_g"){
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        backdrop_g = slider->getValue();
        cout<<"backdrop_g: "<<backdrop_g<<endl;
    }if(name=="backdrop_b"){
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        backdrop_b = slider->getValue();
        cout<<"backdrop_b: "<<backdrop_b<<endl;
    }if(name=="backdrop_a"){
        ofxUISlider *slider = (ofxUISlider *)e.widget;
        backdrop_a = slider->getValue();
        cout<<"backdrop_a: "<<backdrop_a<<endl;
    }
}