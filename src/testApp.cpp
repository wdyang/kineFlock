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
    backdrop_a = 0.1;
    
//    drawGUI=true;
//    setGUI();
//    gui->setDrawBack(false);
    
    
	glNewList(1, GL_COMPILE);
	glutSolidSphere(1, 40, 40);
	glEndList();
    
    glNewList(2, GL_COMPILE);
	glutSolidSphere(5, 40, 40);
	glEndList();
    
    
	
    flyBox_x = flyBox_x0, flyBox_y=flyBox_y0, flyBox_z = flyBox_z0;
    
	target = ofVec3f(0, 0, 0);
	
	for (int i = 0; i < boidNum; i++)
	{
		SteeredVehicle v(ofRandom(100)-50, ofRandom(100)-50, ofRandom(100)-50);
		v.maxForce = 0.5f;
		v.inSightDist = 60.0f;
		boids.push_back(v);
        follow.push_back(ofRandom(100)>70); //if true, boid will chase the tuio target
	}
	
    cam.setDistance(cam_z);
    adjustCamAngle();
    cam_center_distance0 = cam_center_distance;
    cam.disableMouseInput();
    cam.setNearClip(50);
    cam.setFarClip(1500);
    fbo.allocate(ofGetWidth(), ofGetHeight());
    backdrop.loadImage("images/clouds.jpg");
    music.loadSound("Koda - Glass Veil (CoMa Remix).mp3");
    music.play();
	
    float backdroplight = 1.0;
	GLfloat color[] = { backdroplight, backdroplight, backdroplight };
//	GLfloat color[] = { 1.0, 1.0, 1.0 };
	
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);

	glLightfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
    glLightfv(GL_LIGHT0, GL_AMBIENT, color);
    
    tuioClient.start(3333);
    kinectXMax=-100, kinectXMin=100, kinectYMax = -100, kinectYMin=100;
}

void testApp::adjustCamAngle(){
    mark_x=0; mark_z=0;
    mark_y = cam_z * tan(cam_angle);
    cam.setTarget(ofVec3f(mark_x, mark_y, mark_z));
    cam_center_distance = sqrt(cam_z*cam_z + mark_y*mark_y);
}

void testApp::adjustFlyBox(){
    flyBox_x = flyBox_x0 * cam_center_distance / cam_center_distance0;
    flyBox_y = flyBox_y0 * cam_center_distance / cam_center_distance0;
    cout<<"new fly box size: "<< flyBox_x<<" "<<flyBox_y<<" "<<flyBox_z<<"y0"<<mark_y<<endl;
}

//--------------------------------------------------------------
void testApp::update()
{
    if(bKillingBoid) killLastBoid();

    updateTuio();
    
    for (int i = 0; i < boidNum; i++)
	{
		boids[i].flock(boids);
        if(bTuioTouched){
            if(distance(boids[i].position, target)<200){  //too close, flee
                boids[i].flee(target);
            }else if(follow[i]){
                boids[i].seek(target);
            }
            if(ofRandom(100)>90) follow[i]=(ofRandom(100)>70);  //once for a while a boid change between follow and not follow
        }
		boids[i].update();
        //		boids[i].wrap(700, 500, 500);
        //        boids[i].bounce(flyBox_x, flyBox_y, flyBox_z);
        boids[i].bounceOffset(-flyBox_x/2, flyBox_x/2, -flyBox_y/2+mark_y, flyBox_y/2+mark_y, -flyBox_z/2, flyBox_z/2);
	}
}

//TUIO stream from LKB has min 0 and XMax 650, YMax 500
void testApp::updateKinectMaxMin(float x, float y){
    if (x>kinectXMax) kinectXMax = x;
    if (x<kinectXMin) kinectXMin = x;
    if (y>kinectYMax) kinectYMax = y;
    if (y<kinectYMin) kinectYMin = y;
    cout<<"Kinect Min Max "<<kinectXMin<<" "<<kinectXMax<<" "<<kinectYMin<<" "<<kinectYMax<<endl;
}

void testApp::updateTuio(){
    
    bTuioTouched = false;
    bHandsTogether = false;
    bFromKinect = false;
    bFromIphone = false;
    bHasTuioTarget = false;
    
    tuioTargets.clear();
    
    float tuio_x = 0;
    float tuio_y = 0;
    float tuio_z = 0;
    float tuio_vx = 0;
    float tuio_vy = 0;
    float tuio_speed = -1;
    
    tuioClient.getMessage();
    list<ofxTuioCursor*>cursorList = tuioClient.getTuioCursors();
    int numTouch = cursorList.size();
    if(numTouch>0) {
//        cout<<"Tuio "<<numTouch<<"touchs: ";
        bTuioTouched = true;
    }
    float handX[2], handY[2];
    int idx = 0;
    for(list<ofxTuioCursor*>::iterator it=cursorList.begin(); it!=cursorList.end(); it++){
        ofxTuioCursor *tcur = (*it);

        float pointX = tcur->getX();
        float pointY = tcur->getY();
        if(pointX*pointX + pointY*pointY < 2){   //this is from iphone touchpad, range: 0-1
            bFromIphone=true;
        }else{ //this is from Kinect range: X:0-650, Y:0-500, setting: Use only Hands, Use Hands with push method, NO from center of mass.
            bFromKinect = true;
        }
        if(bFromKinect){
            pointX /= 650;
            pointY /= 500;
        }

        float screenX, screenY, screenZ;
        screenToBox(pointX*ofGetWidth(), pointY*ofGetHeight(), screenX, screenY);
        screenZ = 0;
        tuioTargets.push_back(ofVec3f(screenX, screenY, screenZ));
        
//        cout<<"tuio "<<tcur->getX()<<" "<<tcur->getY()<<"|";
//        updateKinectMaxMin(tcur->getX(), tcur->getY());

        
        float vx = tcur->getXSpeed();
        float vy = tcur->getYSpeed();
        float speed = vx*vx+vy*vy;
        if(idx<2){ //only get the first two touch points
            handX[idx]=pointX;
            handY[idx]=pointY;
            idx++;
        }
        
        if(speed > tuio_speed){
            tuio_speed = speed;
            tuio_vx = vx;
            tuio_vy = vy;
            
            tuio_x = pointX;
            tuio_y = pointY;
            tuio_z = 0;
        }
    }

//    if(bFromKinect) cout<<"Kinect"<<endl;
    
    if(bFromKinect && numTouch>1){
        float distance = pow(handX[0]-handX[1], 2) + pow(handY[0]-handY[1], 2);
        cout<<"distance "<<distance<<endl;
        if (distance < 0.007) bHandsTogether = true;
    }

//    if(numTouch>0) cout<<endl;
    if(bTuioTouched){
//        cout<<tuio_x<<" "<<tuio_y<<" "<<tuio_vx<<" "<<tuio_vy<<endl;
        
        float boxX, boxY, boxZ;
        screenToBox(tuio_x*ofGetWidth(), tuio_y*ofGetHeight(), boxX, boxY);
        
        boxZ = flyBox_z/2;
        target = ofVec3f(boxX, boxY, boxZ);
        if(bAddBoid || bHandsTogether){
            ofVec3f source = ofVec3f(boxX + ofRandom(20)-10, boxY+ofRandom(20)-10, ofRandom(20)-10);
            addABoid(source);
        }
        bHasTuioTarget = true;
    }
//    cout<<ofGetFrameRate()<<endl;
}

float testApp::distance(ofVec3f &x0, ofVec3f &x1){
    return pow((x1.x-x0.x),2) + pow((x1.y-x0.y), 2) +pow((x1.z-x0.z),2)/100;
}

//--------------------------------------------------------------
void testApp::draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GLfloat color[] = { backdrop_r , backdrop_g, backdrop_b, backdrop_a};
//    GLfloat color[] = { 1, 0.1, 0.1, 0.2};
    
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
    backdrop.draw(0,0, ofGetWidth(), ofGetHeight());
    cam.begin();
	glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);
    ofDrawGrid();
	
	for (int i = 0; i < boidNum; i++)
	{
		glPushMatrix();
		glTranslatef(boids[i].position.x, boids[i].position.y, boids[i].position.z);
		
		GLfloat color[] = { 1, 1, 1, 1.0 };
		
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
		glCallList(1);
		glPopMatrix();
	}
    
    //Draw tuio touch points
    if(bOverlayTargets && bHasTuioTarget){
        for(int i=0; i<tuioTargets.size(); i++){
            glPushMatrix();
            glTranslatef(tuioTargets[i][0], tuioTargets[i][1], tuioTargets[i][2]);
            GLfloat color[]={0.1, 0.2, 0.6};
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
            glCallList(2);
            glPopMatrix();
        }
    }
    //Draw mouse touch points
    if(bOverlayTargets && bHasMouseTarget){
        for(int i=0; i<tuioTargets.size(); i++){
            glPushMatrix();
            glTranslatef(target[0], target[1], target[2]);
            GLfloat color[]={0.1, 0.2, 0.6};
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
            glCallList(2);
            glPopMatrix();
        }
    }

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
    if(bDrawFlyBox) drawFlyBox();
    
	cam.end();
}

void testApp::drawFlyBox(){
    ofPushMatrix();
    ofNoFill();
//    ofSetColor(0, 0.2, 0.5);
    ofVec3f pos = ofVec3f(0, mark_y, 0);
    ofTranslate(pos);
    ofScale(flyBox_x, flyBox_y, flyBox_z);
    ofBox(0, 0, 0, 1);
    ofPopMatrix();
    
    //    glPushMatrix();
//    glLineWidth(2.5);
//    glColor3f(1, 1, 1);
//    glBegin(GL_LINE);
//    glVertex3f(-flyBox_x/2, 0, 0);
//    glVertex3f(flyBox_x/2, 0, 0);
//    glVertex3f(flyBox_x/2,flyBox_y/2, flyBox_z/2);
//    glVertex3f(flyBox_x/2,-flyBox_y/2, flyBox_z/2);
//    glVertex3f(flyBox_x/2,-flyBox_y/2, -flyBox_z/2);
//    glVertex3f(flyBox_x/2, flyBox_y/2, -flyBox_z/2);
//    glVertex3f(flyBox_x/2,flyBox_y/2, flyBox_z/2);
//    glVertex3f(flyBox_x/2,-flyBox_y/2, flyBox_z/2);
//    glEnd();
//    glPopMatrix();
}

float increment(float val, float delta){
    val += delta;
    return (val>1) ? 1 : val;
}

float decrement(float val, float delta){
    val -= delta;
    return (val<0) ? 0 : val;
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    switch(key){
        case 'f':
            ofToggleFullscreen();
            break;
        case 'F':
            cout<<"FPS: "<<ofGetFrameRate()<<endl;
            break;
        case 'h':
//            gui->toggleVisible();
            break;
        case 'r':
            backdrop_r = increment(backdrop_r, 0.01);
            break;
        case 'R':
            backdrop_r = decrement(backdrop_r, 0.01);
            break;
        case 'g':
            backdrop_g = increment(backdrop_g, 0.01);
            break;
        case 'G':
            backdrop_g = decrement(backdrop_g, 0.01);
            break;
        case 'b':
            backdrop_b = increment(backdrop_b, 0.01);
            break;
        case 'B':
            backdrop_b = decrement(backdrop_b, 0.01);
            break;
        case 'a':
            backdrop_a = increment(backdrop_a, 0.002);
            cout<<"backdrop_a: "<<backdrop_a<<endl;
            break;
        case 'A':
            backdrop_a = decrement(backdrop_a, 0.002);
            cout<<"backdrop_a: "<<backdrop_a<<endl;
            break;
        case 'z':
//            cam_z++;
            cam_angle-=0.001;
            cam_angle  = (cam_angle< -1.5) ? -1.5 : cam_angle; //minimum angle 86
            adjustCamAngle();
            cout<<"cam_angle:"<<cam_angle*180/PI;
            adjustFlyBox();
//            cam.setDistance(cam_z);
            break;
        case 'Z':
//            cam_z--;
            cam_angle+=0.001; //looking up
            cam_angle = cam_angle > 0 ? 0 : cam_angle; //maximun 0, looking straight up
            adjustCamAngle();
            cout<<"cam_angle:"<<cam_angle*180/PI;
            adjustFlyBox();
//            cam.setDistance(cam_z);
            break;
        case ' ':
            bAddBoid = !bAddBoid;
            cout<<"Adding boid is "<<bAddBoid<<endl;
            break;
        case 'k':
            bKillingBoid = !bKillingBoid;
            cout<<"Killing boid is "<<bKillingBoid<<endl;
            break;
        case 'd':
            bDrawFlyBox = !bDrawFlyBox;
            break;
        case 'o':
            bOverlayTargets = !bOverlayTargets;
            cout<<"OverlayTargets is "<<bOverlayTargets<<endl;
            break;
            
    }
//    cout<<"r"<<backdrop_r<<" g"<<backdrop_g<<" b"<<backdrop_b<<" a"<<backdrop_a<<" cam_z"<<cam_z<<endl;
//    cout<<"Frame rate: "<<ofGetFrameRate()<<endl;
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
    float framex, framey;

    screenToBox(x, y, framex, framey);
//    cout<<x<<" "<<y<<" --> "<<framex<<" "<<framey<<endl;
    for(int i=0; i<boidNum; i++){
        if(ofRandom(100)>30)
            boids[i].seek(ofVec3f(framex, framey, flyBox_z/2));
    }
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
    bHasMouseTarget = true;

    float framex, framey;
    screenToBox(x, y, framex, framey);
	target = ofVec3f(framex, framey, 0);
	
    ofVec3f loc = ofVec3f(ofRandom(20)-10+framex, ofRandom(20)-10+framey, ofRandom(100)-50);
    addABoid(loc);
}

void testApp::screenToBox(float screenX, float screenY, float &boxX, float &boxY){
    float w, h;
    w=ofGetWidth();
    h=ofGetHeight();
    float scale = max(W/w, H/h);
    float angle_x = atan((screenX-w/2.0)*scale/(W/2)*cam_half_view_x/180*PI);
    float angle_y = atan((screenY-h/2.0)*scale/(W/2)*cam_half_view_x/180*PI);
    boxX =tan(angle_x)*cam_z;
    boxY =-tan(angle_y-cam_angle)*cam_z;
}

void testApp::addABoid(ofVec3f &loc){
    if(boidNum<maxBoidNum){
        boidNum++;
        
        SteeredVehicle v(loc[0], loc[1], loc[2]);
        v.maxForce = 0.5f;
        v.inSightDist = 60.0f;
        boids.push_back(v);
        follow.push_back(ofRandom(100)>70); //if true, boid will chase the tuio target
        cout<<"Boids adding, new number: "<<boidNum<<endl;
    }
}
void testApp::killLastBoid(){
    if(boidNum>0){
        boidNum--;
        boids.pop_back();
        cout<<"Boids killing, left:"<<boids.size()<<endl;
    }else{
        bKillingBoid=false;
        cout<<"Killing boids stopped"<<endl;
    }
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
    bHasMouseTarget = false;
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