import msafluid.*;
import oscP5.*;
import netP5.*;
import processing.opengl.*;
import javax.media.opengl.*;

OscP5 oscP5;
NetAddress myRemoteLocation;
final float FLUID_WIDTH = 120; 
float invWidth, invHeight;    // 
float aspectRatio, aspectRatio2;

MSAFluidSolver2D fluidSolver; 
PImage imgFluid;
boolean drawFluid = true;

void setup() {
  size(1440, 450, OPENGL);    // Usando Opengl para generar la textura
  oscP5 = new OscP5(this,10000);
  myRemoteLocation = new NetAddress("192.168.1.47",4444);
  hint( ENABLE_OPENGL_4X_SMOOTH );    // Turn on 4X antialiasing

  invWidth = 1.0f/width;
  invHeight = 1.0f/height; 
  aspectRatio = width * invHeight;//1.5
  aspectRatio2 = aspectRatio * aspectRatio;//2.25

  //Crando el fluido y los parametros
  fluidSolver = new MSAFluidSolver2D((int)(FLUID_WIDTH), (int)(FLUID_WIDTH * height/width)); //120 * 80
  fluidSolver.enableRGB(true).setFadeSpeed(0.0001).setDeltaT(0.1).setVisc(0.000001);

  //creando la imagen con el fludo
  imgFluid = createImage(fluidSolver.getWidth(), fluidSolver.getHeight(), RGB);
}


/* La escucha de osc */
void oscEvent(OscMessage theOscMessage) {
  if(theOscMessage.checkAddrPattern("/mouse/position")==true) {
    int x = theOscMessage.get(0).intValue();
    int y = theOscMessage.get(1).intValue();
    float accelX = theOscMessage.get(2).floatValue() / 400;
    float accelY = theOscMessage.get(3).floatValue() / 400 ;
    float mouseNormX = x * invWidth;
    float mouseNormY = y * invHeight;
    float mouseVelX = (x - accelX / 10) * invWidth;
    float mouseVelY = (x - accelY /10) * invHeight;
    addForce(mouseNormX, mouseNormY, accelX, accelY);	
    return;
  }
  //reset Cuando aparecen 4 blobs enviados por OSC
  if(theOscMessage.checkAddrPattern("/reset")==true){
     fluidSolver = new MSAFluidSolver2D((int)(FLUID_WIDTH), (int)(FLUID_WIDTH * height/width)); //120 * 80
     fluidSolver.enableRGB(true).setFadeSpeed(0.0001).setDeltaT(0.1).setVisc(0.000001);
     imgFluid = createImage(fluidSolver.getWidth(), fluidSolver.getHeight(), RGB);
  }
}

void draw() {
  fluidSolver.update();
  if(drawFluid) {
    for(int i=0; i<fluidSolver.getNumCells(); i++) {
      int d = 3;
      imgFluid.pixels[i] = color(fluidSolver.r[i] * d, fluidSolver.g[i] * d, fluidSolver.b[i] * d);
    }  
    imgFluid.updatePixels();//  fastblur(imgFluid, 2);
    image(imgFluid, 0, 0, width, height);
  }
}

// add force and dye to fluid, and create particles
void addForce(float x, float y, float dx, float dy) {
  float speed = dx * dx  + dy * dy * aspectRatio2;    // balance the x and y components of speed with the screen aspect ratio

  if(speed > 0) {
    if(x<0) x = 0; 
    else if(x>1) x = 1;
    if(y<0) y = 0; 
    else if(y>1) y = 1;

    float colorMult = 5;
    float velocityMult = 30.0f;

    int index = fluidSolver.getIndexForNormalizedPosition(x, y);
    color drawColor;
    colorMode(HSB, 360, 1, 1);
    float hue = ((x + y) * 180 + frameCount) % 360;
    drawColor = color(hue, 1, 1);
    colorMode(RGB, 1);  
    fluidSolver.rOld[index]  += red(drawColor) * colorMult;
    fluidSolver.gOld[index]  += green(drawColor) * colorMult;
    fluidSolver.bOld[index]  += blue(drawColor) * colorMult;
    fluidSolver.uOld[index] += dx * velocityMult;
    fluidSolver.vOld[index] += dy * velocityMult;
  }
}

