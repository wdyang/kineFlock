#pragma once

#include "ofMain.h"
#include "ofxBoids.h"
#include "ofxTuio.h"
#include "ofxUI.h"

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
	
	ofEasyCam cam;
    ofFbo fbo;
    ofImage backdrop;
	
	int boidNum;
	ofVec3f target;
	vector<SteeredVehicle> boids;
    vector<bool> follow;
	float distance(ofVec3f &x0, ofVec3f &x1);
    ofxTuioClient tuioClient;
};
