#pragma once

#include "ofMain.h"
#include "ofxBoids.h"
#include "ofxTuio.h"
#include "ofxUI.h"

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
	
    void setGUI();
    void guiEvent(ofxUIEventArgs &e);
    ofxUICanvas *gui;
    bool drawGUI;
    float backdrop_r, backdrop_g, backdrop_b, backdrop_a;
    float cam_z, cam_angle, cam_f, cam_half_view_x, cam_half_view_y;
    float mark_x, mark_y, mark_z; //where the camera is looking
    float flyBox_x, flyBox_y, flyBox_z;
    
	
	ofEasyCam cam;
    ofFbo fbo;
    ofImage backdrop;
    
    ofSoundPlayer music;
	
	int boidNum;
	ofVec3f target;
    bool drawTarget, drawMouseTarget;
	vector<SteeredVehicle> boids;
    void addABoid(ofVec3f & loc);
    vector<bool> follow;
	float distance(ofVec3f &x0, ofVec3f &x1);
    ofxTuioClient tuioClient;
};
