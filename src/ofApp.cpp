#include "ofApp.h"
#include <math.h>       /* fmod */
#include <stdio.h>


using namespace ofxCv;

void ofApp::setup() {
#ifdef TARGET_OSX
	//ofSetDataPathRoot("../data/");
#endif
	ofSetVerticalSync(true);
	cloneReady = false;
	cam.initGrabber(1280, 720);
	clone.setup(cam.getWidth(), cam.getHeight());
	ofFbo::Settings settings;
	settings.width = cam.getWidth();
	settings.height = cam.getHeight();
	maskFbo.allocate(settings);
	srcFbo.allocate(settings);
	camTracker.setup();
	srcTracker.setup();
	srcTracker.setIterations(25);
	srcTracker.setAttempts(4);

	faces.allowExt("jpg");
	faces.allowExt("png");
	faces.listDir("faces");
	currentFace = 0;
	if(faces.size()!=0){
		loadFace(faces.getPath(currentFace));
	}
    
    /* counter */
    ofResetElapsedTimeCounter();
    
    /*
     instantiate a slider using an ofParameter<int> instance
     */
    
    sliderInt = new ofxDatGuiSlider(ofParamInt.set("of_Paramater<int>", 50, 0, 100));
    sliderInt->setWidth(ofGetWidth(), .2); // make label area 20% of width //
    sliderInt->setPosition(0, ofGetHeight()*.95 - sliderInt->getHeight() );
    sliderInt->onSliderEvent(this, &ofApp::onSliderEvent);
    ofParamInt.addListener(this, &ofApp::onParamIntChanged);
}

void ofApp::update() {
	cam.update();
	if(cam.isFrameNew()) {
		camTracker.update(toCv(cam));
		
		cloneReady = camTracker.getFound();
		if(cloneReady) {
			ofMesh camMesh = camTracker.getImageMesh();
			camMesh.clearTexCoords();
			camMesh.addTexCoords(srcPoints);
			
            /* Any drawing that you do after begin() is drawn into the fbo rather than the screen. This is how you draw things into your ofFbo instance.*/
			maskFbo.begin();
			ofClear(0, 255);
			camMesh.draw();
			maskFbo.end();
			
			srcFbo.begin();
			ofClear(0, 255);
			src.bind();
			camMesh.draw();
			src.unbind();
			srcFbo.end();
			
            float t = ofGetElapsedTimef();
            t = fmod(t*2, 100); //t % 100.0f;
            
            clone.setStrength(ofParamInt);/*16);*/
			clone.update(srcFbo.getTextureReference(), cam.getTextureReference(), maskFbo.getTextureReference());
		}
	}
    
    sliderInt->update();
    
}

void ofApp::draw() {
	ofSetColor(255);
	
	if(src.getWidth() > 0 && cloneReady) {
		clone.draw(0, 0);
	} else {
		cam.draw(0, 0);
	}
	
	if(!camTracker.getFound()) {
		drawHighlightString("camera face not found", 10, 10);
	}
	if(src.getWidth() == 0) {
		drawHighlightString("drag an image here", 10, 30);
	} else if(!srcTracker.getFound()) {
		drawHighlightString("image face not found", 10, 30);
	}
    
    sliderInt->draw();

}

void ofApp::loadFace(string face){
    ofLog(OF_LOG_NOTICE, "loadFace, face: " + face);

	src.loadImage(face);
    /*src.loadImage("faces/fla0.jpg");*/

	if(src.getWidth() > 0) {
		srcTracker.update(toCv(src));

		srcPoints = srcTracker.getImagePoints();

	}
    
    vector<string> splitString = ofSplitString( face, "/");
    vector<string> splitString2 = ofSplitString( splitString[ splitString.size()-1 ], ".");
    string faceless = "faceless/"+splitString2[0]+ "_.jpg";
    ofLog(OF_LOG_NOTICE, "faceless: " + faceless);
    /* Replace the src image to get the faceless texture */
    src.loadImage(faceless);
}

void ofApp::dragEvent(ofDragInfo dragInfo) {
	loadFace(dragInfo.files[0]);
}

void ofApp::keyPressed(int key){
	switch(key){
	case OF_KEY_UP:
		currentFace++;
		break;
	case OF_KEY_DOWN:
		currentFace--;
		break;
	}
	currentFace = ofClamp(currentFace,0,faces.size());
	if(faces.size()!=0){
		loadFace(faces.getPath(currentFace));
	}
}

void ofApp::onParamIntChanged(int & n)
{
    //cout << "onParamIntChanged "<< n << endl;
}
void ofApp::onSliderEvent(ofxDatGuiSliderEvent e)
{
    if(e.target == sliderInt){
        //  uncomment this to print the change event received from the int slider //
        //  cout << "value = " << e.value << " : scale = " << e.scale << endl;
    }
}
