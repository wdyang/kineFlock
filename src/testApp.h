#pragma once

#include "ofMain.h"
#include "ofxBoids.h"
#include "ofxTuio.h"
#include "ofxUI.h"

#include "ofxOsc.h"
//#define HOST_IPAD "192.168.2.2" //ipad ip
//#define HOST_IPAD "10.0.1.5" //ipad ip
#define PORT_TO_IPAD 9000
#define PORT_IN 8000

#define W 1024
#define H 768


class testApp : public ofBaseApp{

public:
    void setup();
    void update();
    void draw();

    void keyPressed  (int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
	
//    GUI, not used for now
    void setGUI();
    void guiEvent(ofxUIEventArgs &e);
    ofxUICanvas *gui;
    bool drawGUI;
    
    
//    cam, backdrop
    float backdrop_r=0.4, backdrop_g=0.6, backdrop_b=1.0, backdrop_a=0.3, backdrop_rate=14.63;
    int backdrop_x, backdrop_y;
    bool bBrighter=false, bDarker=false;
    void updateBackDrop();

    float cam_center_distance0, cam_center_distance;//distance from camera to the point intercept screen plan
    float cam_z=250, cam_half_view_x=44; //half_view in degree
//    float cam_angle=-31.3;
//    float cam_angle=-10;
    float cam_angle=-70, cam_theta = 0;
    float mark_x, mark_y, mark_z; //where the camera is looking
    bool bCamAngleFar=false, bCamAngleUp=false, bCamTurnLeft = false, bCamTurnRight=false;
    void updateCamAngle();

	ofEasyCam cam;
    float nearClip=50, farClip = 5000;
    void adjustCamAngle();
    ofFbo fbo;
    ofImage backdrop, backdropWhole;
    
    const static int numMusic=4;
    ofSoundPlayer music[numMusic];
    string musicFiles[numMusic] = {
        "1-05 Glass_ Akhnaten - Act 1_ Sc. 3 - The Window Of Appearances.mp3",
        "2-01 Suite No. 2 In D Minor, BWV 1008.mp3",
        "06 Agnus Dei. Andante Molto - Allegro Moderato.mp3",
        "Koda - Glass Veil (CoMa Remix).mp3"};
    void controlMusic(int id, bool action);
    
//      boids
	int boidNum=5;
    int maxBoidNum=1500; //frame rate will drop after 1600
	vector<SteeredVehicle> boids;
    vector<bool> follow;

    float flyBox_x0=900, flyBox_y0=900, flyBox_z0=300, flyBox_max=1500;
    float flyBox_x, flyBox_y, flyBox_z;
    float followChance=0;
    void adjustFlyBox();
    void drawFlyBox();
    
    bool bAddBoid=true;
    bool bKillingBoid = false;
    void addABoid(ofVec3f & loc);
    void killLastBoid();

//    Inputs
	ofVec3f target;
    bool bHasTuioTarget=false, bHasMouseTarget=false, bEnableFollow=true;
	
    float distance(ofVec3f &x0, ofVec3f &x1);
    void screenToBox(float screenX, float screenY, float &boxX, float &boxY);

    ofxTuioClient tuioClient;
    void updateTuio();
    bool bTuioTouched;
    bool bFromIphone, bFromKinect;
    bool bHandsTogether;
    float kinectXMax, kinectXMin, kinectYMax, kinectYMin;
    void updateKinectMaxMin(float x, float y);
    vector<ofVec3f> tuioTargets;
    float lastTuioX=0, lastTuioY=0;
    
//    often change settings:
    bool bDrawFlyBox=true;
    bool bOverlayTargets=true;
    
//  OSC control
    ofxOscReceiver receiver;
    ofxOscSender    sender;
    string ipadIP;
    bool    bSenderLive;

    void parseOSCMessage();
    void oscSendInt(const string &address, int msg);
    void oscSendFloat(const string &address, float msg);
    void oscSendFormatedFloat(const string &address, float msg, int precision); //precision is the number of decimal points
    void oscSendString(const string &address, const string &msg);
    void oscSendInitConfig();
};

