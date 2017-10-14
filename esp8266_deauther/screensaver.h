#ifdef USE_DISPLAY

time_t lastactivity = millis();
time_t saveeafter = 100000; // millesecs before screensaver start
time_t screensavertimer = 0;


float angleX, angleY, angleZ;
float lastAngleX, lastAngleY, lastAngleZ;

#define DEBUG false

const double halfC = M_PI / 180;

// Overall scale and perspective distance
uint8_t sZ = 4, scale = 16, scaleMax = 16;
// screen center coordinates (calculated from screen dimensions)
uint8_t centerX = 64;
uint8_t centerY = 32;

typedef struct {
    double x;
    double y;
    double z;
} Coord3DSet;

typedef struct {
    double x;
    double y;
} Coord2DSet;

typedef struct {
    uint16_t id1;
    uint16_t id2;
} Lines;  


/* https://codepen.io/ge1doot/pen/grWrLe */

static Coord3DSet CubePoints3DArray[21] = {
  {  1,  1,  1 },
  {  1,  1, -1 },
  {  1, -1,  1 },
  {  1, -1, -1 },
  { -1,  1,  1 },
  { -1,  1, -1 },
  { -1, -1,  1 },
  { -1, -1, -1 },

  {  1,  1,  0 },
  {  1,  0,  1 },
  {  0,  1,  1 },

  {  -1,  1,  0 },
  {  -1,  0,  1 },
  {  0,  -1,  1 },

  {  1,  -1,  0 },
  {  1,  0,  -1 },
  {  0,  1,  -1 },

  {  -1,  -1,  0 },
  {  -1,  0,  -1 },
  {  0,  -1,  -1 },

  {0, 0, 0}
  
};

static Coord3DSet CubePoints2DArray[21] = {
  { 0,0 },
  { 0,0 },
  { 0,0 },
  { 0,0 },
  { 0,0 },
  { 0,0 },
  { 0,0 },
  { 0,0 },

  { 0,0 },
  { 0,0 },
  { 0,0 },
  { 0,0 },
  { 0,0 },
  { 0,0 },
  { 0,0 },
  { 0,0 },
  { 0,0 },
  
  { 0,0 },
  { 0,0 },
  { 0,0 },

  { 0,0 }
};

static Lines LinesArray[12] = {
  { 0, 1 },
  { 0, 2 },
  { 0, 4 },
  { 1, 3 },
  { 1, 5 },
  { 2, 3 },
  { 2, 6 },
  { 3, 7 },
  { 4, 5 },
  { 4, 6 },
  { 5, 7 },
  { 6, 7 }
/*
  { 1, 4 },
  { 2, 3 },
  { 1, 6 },
  { 2, 5 },
  { 2, 8 },
  { 6, 4 },
  { 4, 7 },
  { 3, 8 },
  { 1, 7 },
  { 3, 5 },
  { 5, 8 },
  { 7, 6 }
 */
  
};

// used for sorting points by depth
uint16_t zsortedpoints[21] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};

uint16_t totalpoints = sizeof(CubePoints3DArray) / sizeof(CubePoints3DArray[0]);
uint16_t totallines = sizeof(LinesArray) / sizeof(LinesArray[0]);


void vectorRotateXYZ(double angle, int axe) {
  int8_t m1; // coords polarity
  uint8_t i1, i2; // coords index
  double t1, t2;
  uint16_t i;
  for( i=0; i<totalpoints; i++ ) {
    switch(axe) {
      case 1: // X
        m1 = -1;
        t1 = CubePoints3DArray[i].y;
        t2 = CubePoints3DArray[i].z;
        CubePoints3DArray[i].y = t1*cos(angle)+(m1*t2)*sin(angle);
        CubePoints3DArray[i].z = (-m1*t1)*sin(angle)+t2*cos(angle);
      break;
      case 2: // Y
        m1 = 1;
        t1 = CubePoints3DArray[i].x;
        t2 = CubePoints3DArray[i].z;
        CubePoints3DArray[i].x = t1*cos(angle)+(m1*t2)*sin(angle);
        CubePoints3DArray[i].z = (-m1*t1)*sin(angle)+t2*cos(angle);
      break;
      case 3: // Z
        m1 = 1;
        t1 = CubePoints3DArray[i].x;
        t2 = CubePoints3DArray[i].y;
        CubePoints3DArray[i].x = t1*cos(angle)+(m1*t2)*sin(angle);
        CubePoints3DArray[i].y = (-m1*t1)*sin(angle)+t2*cos(angle);
      break;
    }
  }
}

/* sort xyz by z depth */
void zSortPoints() {
  bool swapped;
  uint16_t temp;
  float radius, nextradius;
  do {
    swapped = false;
    for(uint16_t i=0; i!=totalpoints-1; i++ ) {
      radius     = (-CubePoints3DArray[zsortedpoints[i]].z+3)*2;
      nextradius = (-CubePoints3DArray[zsortedpoints[i+1]].z+3)*2;
      if (radius > nextradius) {
        temp = zsortedpoints[i];
        zsortedpoints[i] = zsortedpoints[i + 1];
        zsortedpoints[i + 1] = temp;
        swapped = true;
      }
    }
  } while (swapped);
}


/* draw scaled spheres from background to foreground */
void spherePlot() {
  uint16_t i;
  int radius, halfradius;
  int transid;
  for( i=0; i<totalpoints; i++ ) {
    transid = zsortedpoints[i];
    CubePoints2DArray[transid].x = centerX + scale/(1+CubePoints3DArray[transid].z/sZ)*CubePoints3DArray[transid].x; 
    CubePoints2DArray[transid].y = centerY + scale/(1+CubePoints3DArray[transid].z/sZ)*CubePoints3DArray[transid].y;
    radius = (-CubePoints3DArray[transid].z+3)*2.5;
    halfradius = radius / 2;
    display.setColor(BLACK);  
    //display.fillCircle(CubePoints2DArray[transid].x, CubePoints2DArray[transid].y, radius-1);
    display.fillCircle(CubePoints2DArray[transid].x, CubePoints2DArray[transid].y, radius+1);
    display.setColor(WHITE);
    display.drawCircle(CubePoints2DArray[transid].x, CubePoints2DArray[transid].y, radius);
    display.fillCircle(CubePoints2DArray[transid].x+halfradius-1, CubePoints2DArray[transid].y+halfradius-1, halfradius);
  }
}

/* draw lines between given pairs of points */
void meshPlot() {
  uint16_t i;
  uint16_t id1, id2;
  for( i=0; i<totallines; i++ ) {
    id1 = LinesArray[i].id1;
    id2 = LinesArray[i].id2;
    display.drawLine(CubePoints2DArray[id1].x, CubePoints2DArray[id1].y, CubePoints2DArray[id2].x, CubePoints2DArray[id2].y);
  }
}

void drawScreenSaver() {

  float diffAngleX, diffAngleY, diffAngleZ;

  diffAngleX = lastAngleX - angleX;
  diffAngleY = lastAngleY - angleY;
  diffAngleZ = lastAngleZ - angleZ;

  vectorRotateXYZ((double)(diffAngleY+0.1)*halfC, 1); // X
  vectorRotateXYZ((double)(diffAngleX+0.1)*halfC, 2); // Y
  vectorRotateXYZ((double)diffAngleZ*halfC, 3); // Z

  zSortPoints();
  display.clear();
  meshPlot();
  spherePlot();
  display.display();

}


#endif

