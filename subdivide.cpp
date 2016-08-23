

#include <iostream>
#include <fstream>
#include <cmath>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#ifdef OSX
#include <GLUT/glut.h>
#include <OpenGL/glu.h>
#else
#include <GL/glut.h>
#include <GL/glu.h>
#endif

#include <string.h>
#include <time.h>
#include <math.h>
#include <vector>


using namespace std;


inline float sqr(float x) { return x*x; }


class Viewport;

class Viewport {
public:
    int w, h; // width and height
};


//****************************************************
// Global Variables
//****************************************************
Viewport	viewport;
bool uniform, wireframe;
float rotate2 = 0.0;
bool flat = true;
float step, error = 0;
int numPoints = -1;
int colorcount, patch, quad, quadcount, numdiv = 0;
vector<float> points, p, n, center(3), diameter(3);
vector< vector <vector <float> > > ucurve(4), vcurve(4), patchpoints;
vector<vector<vector<vector <float> > > > controlpoints;
vector< vector<float> > allPoints, unifpoints, unifpoints2;
GLdouble zoomfactor = 1.0;
GLdouble rotatefactor, rotateUD, rotateLR = 0.0;
GLfloat translateLR, translateUD = 1.0;
float biggestX = 0, biggestY = 0, biggestZ = 0, smallestX = 0, smallestY = 0, smallestZ = 0, diam, endofadappatch = 0;
vector<vector<vector<float> > > trianglelist;
vector<vector<float> > trianglelist2;
int bezpatchcall = 0;
float normalize = 0;



void myReshape(int w, int h) {
    viewport.w = w;
    viewport.h = h;
    glViewport (0,0,viewport.w,viewport.h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    
}



//
//vector functions
//
vector<float> operator*(vector<float> vec, float scale) {
    vector<float> retVec;
    retVec.push_back(vec[0] * scale);
    retVec.push_back(vec[1] * scale);
    retVec.push_back(vec[2] * scale);
    return retVec;
}

vector<float> operator*(vector<float> vec1, vector<float> vec2){
    vector<float> retVec;
    retVec.push_back(vec1[0] * vec2[0]);
    retVec.push_back(vec1[1] * vec2[1]);
    retVec.push_back(vec1[2] * vec2[2]);
    return retVec;
}

vector<float> operator+(vector<float> vec1, vector<float> vec2){
    vector<float> retVec;
    retVec.push_back(vec1[0] + vec2[0]);
    retVec.push_back(vec1[1] + vec2[1]);
    retVec.push_back(vec1[2] + vec2[2]);
    return retVec;
}

vector<float> operator-(vector<float> vec1, vector<float> vec2){
    vector<float> retVec;
    retVec.push_back(vec1[0] - vec2[0]);
    retVec.push_back(vec1[1] - vec2[1]);
    retVec.push_back(vec1[2] - vec2[2]);
    return retVec;
}

vector<float> crossprod(vector<float> vec1, vector<float> vec2) {
    vector<float> retVec;
    retVec.push_back((vec1[1] * vec2[2]) - (vec1[2] * vec2[1]));
    retVec.push_back((vec1[2] * vec2[0]) - (vec1[0] * vec2[2]));
    retVec.push_back((vec1[0] * vec2[1]) - (vec1[1] * vec2[0]));
    return retVec;
}






void color() {
    
    //use save points to make quads
    
    numdiv = ((1+0.001)/step);

    if (10%int(10*step)!=0) {
        numdiv++;
    }
    
    GLfloat cyan[] = {0.f, .8f, .8f, 0.f};
    GLfloat white[] = {0.0f, .1f, .1f, 0.f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, cyan);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);

        for (int j=0; j<unifpoints.size(); j+=sqr(numdiv+1)) { //j = start of new patch
            for (int k = j; k<j+sqr(numdiv); k+=numdiv+1) { //k = line of patch iterating through
                for (int i=k; i<k+numdiv; i++) { //i = quad in line of subdivisions
                    
                    glColor3f(0.0f, 0.5f, 0.0f);
                    glBegin(GL_POLYGON);
                
                    glNormal3f(unifpoints2[i][0], unifpoints2[i][1], unifpoints2[i][2]);
                    glVertex3f(unifpoints[i][0], unifpoints[i][1], unifpoints[i][2]);
                    
                    glNormal3f(unifpoints2[i+numdiv+1][0], unifpoints2[i+numdiv+1][1], unifpoints2[i+numdiv+1][2]);
                    glVertex3f(unifpoints[i+numdiv+1][0], unifpoints[i+numdiv+1][1], unifpoints[i+numdiv+1][2]);
                    
                    glNormal3f(unifpoints2[i+numdiv+2][0], unifpoints2[i+numdiv+2][1], unifpoints2[i+numdiv+2][2]);
                    glVertex3f(unifpoints[i+numdiv+2][0], unifpoints[i+numdiv+2][1], unifpoints[i+numdiv+2][2]);
                    
                    glNormal3f(unifpoints2[i+1][0], unifpoints2[i+1][1], unifpoints2[i+1][2]);
                    glVertex3f(unifpoints[i+1][0], unifpoints[i+1][1], unifpoints[i+1][2]);
                    
                    glEnd();
                    
                }
            }
        }
    
}


vector< vector<float> > bezcurveinterp(vector<vector <float> > vec, float u) {

    vector< vector<float> > pointderiv(2);
    
    vector<float> a = vec[0] * (1.0f-u) + vec[1] * u;
    vector<float> b = vec[1] * (1.0f-u) + vec[2] * u;
    vector<float> c = vec[2] * (1.0f-u) + vec[3] * u;
    
    
    vector<float> d = a * (1.0f-u) + b * u;
    vector<float> e = b * (1.0f-u) + c * u;
    
    p = d * (1.0f-u) + e * u;
    
    vector<float> dPdu = (e-d) * 3;
    
    pointderiv[0] = p;
    pointderiv[1] = dPdu;
    
    //used for setting up appropriate window coordinates in myDisplay
    if (p[0]>biggestX) {
        biggestX = p[0];
    }
    if (p[1]>biggestY) {
        biggestY = p[1];
    }
    if (p[2]>biggestZ) {
        biggestZ = p[2];
    }
    if (p[0]<smallestX) {
        smallestX = p[0];
    }
    if (p[1]<smallestY) {
        smallestY = p[1];
    }
    if (p[2]<smallestZ) {
        smallestZ = p[2];
    }
    
    return pointderiv;
}


vector<vector<float> > bezpatchinterp(vector<vector<vector<float> > > vec, float u, float v) {
    vector<vector<float> > curve(4);
    for (int i=0; i<4; i++) {
        curve[0] = vec[i][0];
        curve[1] = vec[i][1];
        curve[2] = vec[i][2];
        curve[3] = vec[i][3];
        ucurve[i] = bezcurveinterp(curve, v);
    }
    for (int i=0; i<4; i++) {
        curve[0] = vec[0][i];
        curve[1] = vec[1][i];
        curve[2] = vec[2][i];
        curve[3] = vec[3][i];
        vcurve[i] = bezcurveinterp(curve, u);
    }
    vector<vector<float> > vcurvepoints(4);
    vcurvepoints[0] = vcurve[0][0];
    vcurvepoints[1] = vcurve[1][0];
    vcurvepoints[2] = vcurve[2][0];
    vcurvepoints[3] = vcurve[3][0];
    vector<vector<float> > vcurvePointDeriv(2);
    vcurvePointDeriv = bezcurveinterp(vcurvepoints, v);
    
    vector<vector<float> > ucurvepoints(4);
    ucurvepoints[0] = ucurve[0][0];
    ucurvepoints[1] = ucurve[1][0];
    ucurvepoints[2] = ucurve[2][0];
    ucurvepoints[3] = ucurve[3][0];
    vector<vector<float> > ucurvePointDeriv(2);
    ucurvePointDeriv = bezcurveinterp(ucurvepoints, u);
    
    vector<float> norm(3);
    norm = crossprod(ucurvePointDeriv[1], vcurvePointDeriv[1]*-1);
    
    float length = sqrt(sqr(norm[0])+sqr(norm[1])+sqr(norm[2]));
    norm[0] = norm[0]/length;
    norm[1] = norm[1]/length;
    norm[2] = norm[2]/length;
    
    vector<vector<float> > retVec;
    retVec.push_back(ucurvePointDeriv[0]);
    retVec.push_back(norm);
    
    return retVec;
}


bool distanceUV(vector<float> trianglemid, vector<float> tri1, vector<float> tri2, vector<vector<vector<float> > > uvpoints) {
    
    vector<vector<float> > pointandnormtest = bezpatchinterp(uvpoints, trianglemid[0], trianglemid[1]);
    vector<vector<float> > pointandnormreal1 = bezpatchinterp(uvpoints, tri1[0], tri1[1]);
    vector<vector<float> > pointandnormreal2 = bezpatchinterp(uvpoints, tri2[0], tri2[1]);
    
    
    vector<float> disterror = (pointandnormreal1[0]+pointandnormreal2[0])*(-0.5)+pointandnormtest[0]+pointandnormtest[1];
    normalize = sqrt(sqr(disterror[0])+sqr(disterror[1])+sqr(disterror[2]));

    if (abs(normalize)<error) {
        return true;
    } else {
        return false;
    }

}

void closestpointfornormal(vector<float> tri, vector<vector<vector<float> > > uvpoints) {
    vector<float> dif(3);
    float currlength = 100;
    float dist;
    vector<float> uvnorm;
    for (int i = 0; i<uvpoints.size(); i++) {
        dif[0] = abs(tri[0]-uvpoints[i][0][0]);
        dif[1] = abs(tri[1]-uvpoints[i][0][1]);
        dif[2] = abs(tri[2]-uvpoints[i][0][2]);
        dist = sqrt(sqr(dif[0])+sqr(dif[1])+sqr(dif[2]));
        if (dist < currlength) {
            currlength = dist;
            uvnorm = uvpoints[i][1];
        }
    }
    trianglelist2.push_back(uvnorm);
}


void savesurfpointnorm(vector<float> vec, vector<float> vec2) {
        unifpoints.push_back(vec);
        unifpoints2.push_back(vec2);
}


void subdividepatch(vector<vector<vector<float> > > patch) {
    numdiv = ((1+0.001)/step);
    float u, v;
    
    
   if (10%int(10*step)!=0) {
       numdiv++;
   }
    
    for (int iu = 0; iu<=numdiv; iu++) {
    
        if (10%int(10*step)!=0 && iu==numdiv) {
            u = 1.0;
        } else {
            u = iu * step;
        }

        for (int iv = 0; iv<=numdiv; iv++) {
            if (10%int(10*step)!=0 && iv==numdiv) {
                v = 1.0;
            } else {
                v = iv * step;
            }
            p = bezpatchinterp(patch, u, v)[0];
            n = bezpatchinterp(patch, u, v)[1];
            savesurfpointnorm(p, n);
            
            //cout<<"\nv: "<<v;
        }
        //cout<<"\nfinished v loop at unifpoints item"<<unifpoints.size()-1;

    }
    //cout<<"\nfinished u and v loops (whole patch) at unifpoints item"<<unifpoints.size()-1;
}



//
//get patches from inputfile
//
void patchesfromfile (string s) {
    int xyz = 0;
    string point = "";
    float b;
    if (s=="") {
        goto skip;
    }
    //cout<<"\nstring \""<<s<<"\"\nsize "<<s.size();
    for (int i=0; i<=s.size();) {
        if (s[i]!=' ') {
            point+=s[i];
            if (i==s.size()) {
                points.push_back(atof(point.c_str()));
                numPoints++;
            }
            i++;
        } else if (s[i]==' ') {
            if (point=="") {
                while (s[i]==' ') {
                    i++;
                    if (i==s.size()) {
                        goto done;
                    }
                }
            } else {
                points.push_back(atof(point.c_str()));
                numPoints++;
                point = "";
            }
            
        }
    }
done:

    if (points.size()==12) {
        allPoints.clear();
        for (int i=0; i<12;) {
            vector<float> xyz(3);
            xyz[0] = points[i];
            xyz[1] = points[i+1];
            xyz[2] = points[i+2];
            allPoints.push_back(xyz);
            i+=3;
        }
    }
    
    patchpoints.push_back(allPoints);
    patch++;
    
    if (patch == 4) {
        subdividepatch(patchpoints);

        patchpoints.clear();
        patch = 0;
    }
    
    points.clear();

skip: ;

}






//****************************************************
// render scene
//***************************************************
void myDisplay() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// clear the color buffer

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    center[0]=(abs(biggestX+smallestX))/2;
    center[1]=(abs(biggestY+smallestY))/2;
    center[2]=(abs(biggestZ+smallestZ))/2;
    diameter[0]=abs(biggestX-smallestX);
    diameter[1]=abs(biggestY-smallestY);
    diameter[2]=abs(biggestZ-smallestZ);
    diam = max(max(diameter[0], diameter[1]), diameter[2]);
    
    glOrtho(center[0]-diam, center[0]+diam, center[1]-diam, center[1]+diam, smallestZ-2*diam, biggestZ+2*diam);
    
    glScalef(zoomfactor, zoomfactor, 1.0);


    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(center[0]+0.1, center[1]+diam, center[2],
              center[0], center[1], center[2], /* look at the center of the object */
              0, 0, 1); /* positive Z up vector */
    glEnable (GL_DEPTH_TEST);

    glTranslatef(1*translateLR, 1*translateLR, 1*translateUD);
    glRotatef(15*rotateLR, 0, 0, 1);
    glRotatef(15*rotateUD, 1, 0, 0);


    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHT2);
//    GLfloat lightpos[] = {0.0, 0., 4., 0.};
    glLightf(GL_LIGHT0, GL_SPECULAR, (diam, 0, diam));
    glLightf(GL_LIGHT1, GL_DIFFUSE, (diam, 0, diam));
    glLightf(GL_LIGHT2, GL_AMBIENT, (diam, 0, diam));
    

    if (flat == true) {
        glShadeModel(GL_FLAT);
    } else {
        glShadeModel(GL_SMOOTH);
    }
    if (wireframe == true) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    
    color();
    glFlush();
    glutSwapBuffers();
}

//Used to toggle adjustments to the viewing window
void idleInput (unsigned char key, int xmouse, int ymouse) {

    switch (key)
    
    {

        case 's': //toggle flat/smooth shading
            if (flat == true) {
                flat = false;
            } else {
                flat = true;
            }
            myDisplay();
            break;
            
        case 'w': //toggle filled/wireframe mode
            if (wireframe == true) {
                wireframe = false;
            } else {
                wireframe = true;
            }
            myDisplay();
            break;

        case '+': //zoom in
            zoomfactor+=0.25;
            myDisplay();
            break;
        case '=': //zoom in (still + key)
            zoomfactor+=0.25;
            myDisplay();
            break;

        case '-': //zoom out
            zoomfactor-=0.25;
            myDisplay();
            break;
            
        case ' ':
            exit(0);
            
        default:
            break;
    }
}

void arrowkey (int key, int x, int y) {
    int mod = glutGetModifiers();
    switch(key)
    {
        //rotate/translate adjustments for arrow keys and shift+arrow keys
        case 100:
            if (mod==GLUT_ACTIVE_SHIFT) {
                translateLR += 0.25;
            } else {
                rotateLR-=1;
            }
            myDisplay();
            break;
        case 101:
            if (mod==GLUT_ACTIVE_SHIFT) {
                translateUD += 0.25;
            } else {
                rotateUD+=1;
            }
            myDisplay();
            break;
        case 102:
            if (mod==GLUT_ACTIVE_SHIFT) {
                translateLR -= 0.25;
            } else {
                rotateLR+=1;
            }
            myDisplay();
            break;
        case 103:
            if (mod==GLUT_ACTIVE_SHIFT) {
                translateUD -= 0.25;
            } else {
                rotateUD-=1;
            }
            myDisplay();
            break;
        default:
            break;
    }
}








//****************************************************
// main
//****************************************************
int main(int argc, char *argv[]) {

	glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    
    // viewport size
    viewport.w = 400;
    viewport.h = 400;
    
    //size and position of the window
    glutInitWindowSize(viewport.w, viewport.h);
    glutInitWindowPosition(800,0);
    glutCreateWindow("as3");
    


    
    //SET INPUTFILE TO GET PATCHES FROM HERE
    ifstream inputfile(argv[1]);
    string input = argv[1];
    step = atof(argv[2]);
    uniform = true;
    

    cout << "\nFile: " << input;
    cout << "\nParameter: " << step;
    cout << "\nUniform: " << uniform;
    cout << "\n";
    std::string s;

    string numPatches;
    getline(inputfile, numPatches, '\n');
    while (getline(inputfile, s, '\n')) {
        patchesfromfile(s);
    }
    
    
    glutDisplayFunc(myDisplay);
    glutReshapeFunc(myReshape);
    glutSpecialFunc(arrowkey);
    glutKeyboardFunc(idleInput);

    glutMainLoop();

    
    return 0;
}

