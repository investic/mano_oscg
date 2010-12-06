#include "testApp.h"
#include "ofxVectorMath.h"

// transform
float	depthScale;
float	depthOffset;


//--------------------------------------------------------------
void testApp::setup()
{
	synth.loadSound("sounds/uno.mp3");
	synth.setVolume(0.75f);
	
	
	kinect.init();
	kinect.setVerbose(true);
	kinect.open();

	colorImg.allocate(kinect.width, kinect.height);
	grayImage.allocate(kinect.width, kinect.height);
	grayThresh.allocate(kinect.width, kinect.height);
	grayThreshFar.allocate(kinect.width, kinect.height);

	nearThreshold = 50;
	farThreshold  = 108;
	bThreshWithOpenCV = true;
	
	ofSetFrameRate(60);
	
	// open an outgoing connection to HOST:PORT
	sender.setup( HOST, PORT );
	
}

//--------------------------------------------------------------
void testApp::update()
{
	ofBackground(100, 100, 100);
	kinect.update();

	grayImage.setFromPixels(kinect.getDepthPixels(), kinect.width, kinect.height);
			
	if( bThreshWithOpenCV ){
		
		grayThreshFar = grayImage;
		grayThresh = grayImage;
		grayThreshFar.threshold(farThreshold, true);
		grayThresh.threshold(nearThreshold);
		cvAnd(grayThresh.getCvImage(), grayThreshFar.getCvImage(), grayImage.getCvImage(), NULL);
		 
	}

	// if one blob found, find nearest point in blob area		
	//update the cv image
	grayImage.flagImageChanged();
    // find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
    // also, find holes is set to true so we will get interior contours as well....
    contourFinder.findContours(grayImage, 10, (kinect.width*kinect.height)/2, 200, false);
	ofSoundUpdate();
	
	
}

//--------------------------------------------------------------
void testApp::draw()
{
	
	ofSetHexColor(0xA4A4A4);
	ofRect(720, 15, 720, 450);
	grayImage.draw(715, 20, 715, 445);
	
	ofSetHexColor(0xA4A4A4);
	ofRect(10, 15, 720, 450);
	ofSetHexColor(0xA4A4A4);
	ofSetColor(22, 22, 22);
	grayImage.draw(10, 20, 715, 445);
    contourFinder.draw(10, 20, 715, 445);
	
	ofSetHexColor(0xFFFFFF);
	ofDrawBitmapString("accel is: " + ofToString(kinect.getMksAccel().x, 2) + " / " 
									+ ofToString(kinect.getMksAccel().y, 2) + " / "
									+ ofToString(kinect.getMksAccel().z, 2), 20, 458 );
	
	
	static ofxVec3f newPoint;
	if(contourFinder.blobs.size() == 1) {
		ofxCvBlob &blob = contourFinder.blobs[0];
		grayThreshFar.setROI(blob.boundingRect);
		double minValue, maxValue;
		CvPoint minLoc, maxLoc;
		cvMinMaxLoc(grayThreshFar.getCvImage(), &minValue, &maxValue, &minLoc, &maxLoc, NULL);
		grayThreshFar.resetROI();
		newPoint.x = maxLoc.x + blob.boundingRect.x;
		newPoint.y = maxLoc.y + blob.boundingRect.y;
		
		float accelX = kinect.getMksAccel().x;
		float accelY = kinect.getMksAccel().y;
		
		ofxOscMessage m;
		m.setAddress( "/mouse/position" );		
		m.addIntArg( newPoint.x );
		m.addIntArg( newPoint.y );
		m.addFloatArg(accelX);
		m.addFloatArg(accelY);
		sender.sendMessage( m );
		
		// when in the "beat" region:
		synth.setSpeed(0.3f + ((float)(ofGetHeight() - newPoint.y) / (float)ofGetHeight())*0.8f);
		synth.setPosition(0.0f + ((float)(ofGetHeight() - newPoint.x) / (float)ofGetHeight())*1.0f);
		
	}

	if (contourFinder.blobs.size()==3) {
		synth.setLoop(true);
		synth.play();		
		synth.setSpeed(1.0f);

	}
	
	if (contourFinder.blobs.size()==4) {
		ofxOscMessage h;
		h.setAddress("/reset");
		h.addStringArg("yes");
		sender.sendMessage(h);
	}
	
	//---------------------------  transparency
	ofSetHexColor(0x00FF33);
	//ofRect(newPoint.x,newPoint.y,100,100);
		
	char reportStr[1024];
	//printf("esto es %i", bThreshWithOpenCV);
	sprintf(reportStr, "probando Opencv+ kinect = %i (s)\nset umbral cercano %i (press: + -)\nset umbral lejano %i (press: < >) num blobs econtrados %i, fps: %f movidax: %f",bThreshWithOpenCV, nearThreshold, farThreshold, contourFinder.nBlobs, ofGetFrameRate()), newPoint.x;
	ofDrawBitmapString(reportStr, 20, 690);
}


//--------------------------------------------------------------
void testApp::exit(){
	kinect.close();
}

//--------------------------------------------------------------
void testApp::keyPressed (int key)
{
	switch (key)
	{

		case '>':
		case '.':
			farThreshold ++;
			if (farThreshold > 255) farThreshold = 255;
			break;
		case '<':		
		case ',':		
			farThreshold --;
			if (farThreshold < 0) farThreshold = 0;
			break;
			
		case '+':
		case '=':
			nearThreshold ++;
			if (nearThreshold > 255) nearThreshold = 255;
			break;
		case '-':		
			nearThreshold --;
			if (nearThreshold < 0) nearThreshold = 0;
			break;
		case 'w':
			kinect.enableDepthNearValueWhite(!kinect.isDepthNearValueWhite());
			break;
		case 'f':
			ofToggleFullscreen();	
			break;
		case 'a':
			ofxOscMessage m;
			m.setAddress( "/test" );
			m.addStringArg( "hello" );
			sender.sendMessage( m );
			break;


	}
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y)
{}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button)
{}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button)
{
	
	synth.play();
	synth.setSpeed(0.9);

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button)
{}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h)
{}

