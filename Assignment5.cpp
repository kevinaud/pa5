#define _GLIBCXX_USE_CXX11_ABI 0
#define NUM_OPENGL_LIGHTS 8

//#include <string.h>
#include <iostream>
#include <fstream>
#include <string>
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/glui.h>
#include "Shape.h"
#include "Cube.h"
#include "Cylinder.h"
#include "Cone.h"
#include "Sphere.h"
#include "SceneParser.h"
#include "TextureMap.h"
#include "Camera.h"

using namespace std;

struct SceneObject {
	Shape* shape;
	Matrix transform;
	Matrix invTransform;
	SceneMaterial material;
};

int MAX_RECURSION = 1;

/** These are the live variables passed into GLUI ***/
int  isectOnly = 1;

int	 camRotU = 0;
int	 camRotV = 0;
int	 camRotW = 0;
int  viewAngle = 45;
float eyeX = 2;
float eyeY = 2;
float eyeZ = 2;
float lookX = -2;
float lookY = -2;
float lookZ = -2;

/** These are GLUI control panel objects ***/
int  main_window;
string filenamePath = "data/general/test.xml";
GLUI_EditText* filenameTextField = NULL;
GLubyte* pixels = NULL;
int pixelWidth = 0, pixelHeight = 0;
int screenWidth = 0, screenHeight = 0;

std::vector<SceneObject> sceneObjects;

/** these are the global variables used for rendering **/
Cube* cube = new Cube();
Cylinder* cylinder = new Cylinder();
Cone* cone = new Cone();
Sphere* sphere = new Sphere();
SceneParser* parser = NULL;
Camera* camera = new Camera();

void setupCamera();
void updateCamera();
void flattenScene(SceneNode* node, Matrix compositeMatrix);
Point calculateColor(int closestObjectNdx, Vector normalVector, Vector ray, Point isectWorldPoint);
Point recursiveCalcColor(int closestObjectNdx, Vector normalVector, Vector ray, Point isectWorldPoint, int depth);
Vector renderPixel(int x, int y);
Vector generateRay(int x, int y);
void setpixel(GLubyte* buf, int x, int y, int r, int g, int b);
bool isObjectBetweenLight(int closestObjectNdx, Point isectWorldPoint, Point lightPos);
void callback_start(int id);
void getClosestObject(Point eyePoint, Vector ray, double &minDist, int &closestObject);

// TEMP
TextureMap* earth;

void callback_start(int id) {
	cout << "start button clicked!" << endl;

    delete earth;
    earth = new TextureMap("data/tests/image/earth.ppm");

	if (parser == NULL) {
		cout << "no scene loaded yet" << endl;
		return;
	}

	pixelWidth = screenWidth;
	pixelHeight = screenHeight;

	cout << "Processing " << pixelWidth << " colemns and " << pixelHeight << " rows." << endl;

	updateCamera();

	if (pixels != NULL) {
		delete pixels;
	}
	pixels = new GLubyte[pixelWidth  * pixelHeight * 3];
	memset(pixels, 0, pixelWidth  * pixelHeight * 3);

	for (int i = 0; i < pixelWidth; i++) {
		for (int j = 0; j < pixelHeight; j++) {
            renderPixel(i, j);
		}
	}
	cout << endl << "render complete" << endl;
	SceneGlobalData globalData;
	parser->getGlobalData(globalData);
    cout << endl << "ks = " << globalData.ks << endl;
	glutPostRedisplay();
}

Vector renderPixel(int x, int y) {
    Vector ray = generateRay(x, y);
    double minDist;
    int closestObject;
    getClosestObject(camera->GetEyePoint(), ray, minDist, closestObject);
    if (closestObject != -1) {
        // RAY INTERSECTED OBJECT
        Matrix inverseTransform = sceneObjects[closestObject].invTransform;
        Point eyePointObjectSpace = inverseTransform*camera->GetEyePoint();
        Vector rayObjectSpace = inverseTransform*ray;
        Shape* closestShape = sceneObjects[closestObject].shape;

        // get surface normal at point of intersection
        Vector normal = closestShape->findIsectNormal(eyePointObjectSpace, rayObjectSpace, minDist);
        // convert normal back to world space
        normal = transpose(inverseTransform) * normal;
        normal.normalize();

        // calc intersection point
        Point isectWorldPoint = camera->GetEyePoint() + minDist*ray;

        int depth = 0;
        // calc color
        Point color;
        if (isectOnly == 1) {
            color = calculateColor(closestObject, normal, ray, isectWorldPoint);
        } else {
            color = recursiveCalcColor(closestObject, normal, ray, isectWorldPoint, depth);
        }
        color = color * 255;
        setpixel(pixels, x, y, color[0], color[1], color[2]);
    }
}

Point recursiveCalcColor(
    int closestObjectNdx,
    Vector normalVector,
    Vector ray,
    Point isectWorldPoint,
    int depth
) {

	SceneGlobalData globalData;
	parser->getGlobalData(globalData);
    float ks = globalData.ks;

    if (depth > MAX_RECURSION) {
        return Point(); 
    }
    Point color = calculateColor(closestObjectNdx, normalVector, ray, isectWorldPoint);

    // calc reflection ray
    double VdotN = dot(ray, normalVector);
    Vector reflectionRay = ray - ((2 * VdotN) * normalVector);

    // check for intersecting object
    int closestObject;
    double minDist;
    getClosestObject(isectWorldPoint, reflectionRay, minDist, closestObject);
    //cout << "mindist " << minDist << endl;

    /*     cout << closestObjectNdx << " x " << closestObject << endl; */
    /* } else { */
    /*     cout << "---" << endl; */ 
    /* } */
    
    if (closestObject > -1) {
        // get color at point of intersection
        //cout << "x";

        // RAY INTERSECTED OBJECT
        Matrix inverseTransform = sceneObjects[closestObject].invTransform;
        Point eyePointObjectSpace = inverseTransform*isectWorldPoint;
        Vector rayObjectSpace = inverseTransform*reflectionRay;
        Shape* closestShape = sceneObjects[closestObject].shape;

        // get surface normal at point of intersection
        Vector normal = closestShape->findIsectNormal(eyePointObjectSpace, rayObjectSpace, minDist);
        // convert normal back to world space
        normal = transpose(inverseTransform) * normal;
        normal.normalize();

        // calc intersection point
        Point reflectedIsectWorldPoint = isectWorldPoint + minDist*reflectionRay;
        /* Point reflectedColor = recursiveCalcColor(closestObject, normal, reflectionRay, reflectedIsectWorldPoint, depth++); */

        
        Point reflectedColor = recursiveCalcColor(closestObject, normal, reflectionRay, reflectedIsectWorldPoint, depth++);

        SceneMaterial material = sceneObjects[closestObject].material;
		Point diffuse(material.cDiffuse.r, material.cDiffuse.g, material.cDiffuse.b); 

        double reflectiveness = 1.0;
        for (int j = 0; j < 3; j++) {
            double increment = globalData.ks * reflectiveness * reflectedColor[j];
            color[j] += increment;
            if (color[j] > 1.0) {
                color[j] = 1.0; 
            }
        }
    } 

    // add color to current color based of reflectiveness

    return color;
}

Vector generateRay(int x, int y) {
	double px = -1.0 + 2.0*x/ (double)screenWidth;
	double py = -1.0 + 2.0*y/ (double)screenHeight;
	Point camSreenPoint(px, py, -1);
	//Matrix worldToCamera = camera->GetScaleMatrix() * camera->GetModelViewMatrix();
	//Matrix cameraToWorld = invert(worldToCamera);
	Matrix cameraToWorld = camera->getCamera2WorldMatrix();
	Point worldScreenPoint = cameraToWorld * camSreenPoint;
	Vector ray = worldScreenPoint - camera->GetEyePoint();
	ray.normalize();
	return ray;
}

void getClosestObject(Point eyePoint, Vector ray, double &minDist, int &closestObject) {
    minDist = MIN_ISECT_DISTANCE;
    closestObject = -1;
    for (int k = 0; k < sceneObjects.size(); k++) {
        Shape* shape = sceneObjects[k].shape;
        double curDist = shape->Intersect(eyePoint, ray, sceneObjects[k].transform);
        if ((curDist < minDist) && (curDist > 0) && !(IN_RANGE(curDist, 0))) {
            minDist = curDist;
            closestObject = k;
            //*closestObject = curObject;
            //*isectPoint = eyeInObjectSpace + minDist * rayInObjectSpace;
        }
    }
}

void setpixel(GLubyte* buf, int x, int y, int r, int g, int b) {
	buf[(y*pixelWidth + x) * 3 + 0] = (GLubyte)r;
	buf[(y*pixelWidth + x) * 3 + 1] = (GLubyte)g;
	buf[(y*pixelWidth + x) * 3 + 2] = (GLubyte)b;
}

bool isObjectBetweenLight(int closestObjectNdx, Point isectWorldPoint, Point lightPos) {
	Vector lightDir = isectWorldPoint - lightPos;
    lightDir.normalize();
    for (int i = 0; i < sceneObjects.size(); i++) {
        if (i != closestObjectNdx) {
        //    double dist = sceneObjects[i].Intersect(isectWorldPoint, lightDir, );
        }
    }
}

Point calculateColor(int closestObjectNdx, Vector normalVector, Vector ray, Point isectWorldPoint) {
    SceneObject closestObject = sceneObjects[closestObjectNdx];
	Point color;
	int i, j;

	int numLights = parser->getNumLights();

    // ITERATE OVER LIGHTS
	for (int i = 0; i < numLights; i++) {
		SceneLightData lightData;
		parser->getLightData(i, lightData);

		Vector lightDir = lightData.pos - isectWorldPoint;
		lightDir.normalize();
        
        if (isObjectBetweenLight(closestObjectNdx, isectWorldPoint, lightData.pos)) {
        
        }
			
		double dot_nl = dot(normalVector, lightDir);
        Vector v = 2 * normalVector;
		double dot_vr = dot(ray, ((v*dot_nl) - lightDir));

		if (dot_nl<0) dot_nl = 0;
		if (dot_vr<0) dot_vr = 0;

		
		double power = pow(dot_vr, closestObject.material.shininess);
		//power = power * 255;


		Point diffuse = closestObject.shape->textureAt(isectWorldPoint, closestObject.transform, earth);

        /* if (closestObject) { */
        /*     diffuse = Point(closestObject.material.cDiffuse.r, closestObject.material.cDiffuse.g, closestObject.material.cDiffuse.b); */ 
        /* } */
		//diffuse = diffuse *255;

		double blend = closestObject.material.blend;
		double r_blend = 1 - blend;

		Point specular(closestObject.material.cSpecular.r, closestObject.material.cSpecular.g, closestObject.material.cSpecular.b);

		Point lightColor(lightData.color.r, lightData.color.g, lightData.color.b);

		for (j = 0; j<3; j++) {
			color[j] = color[j] + diffuse[j] * dot_nl + specular[j] * lightColor[j] * power;
			if (color[j]>1) {
				color[j] = 1.0;
			}
		}
	}
	return color;
}






void callback_load(int id) {
	char curDirName [2048];
	if (filenameTextField == NULL) {
		return;
	}
	printf ("%s\n", filenameTextField->get_text());

	if (parser != NULL) {
		delete parser;
	}
	parser = new SceneParser (filenamePath);
	cout << "success? " << parser->parse() << endl;

	setupCamera();

	sceneObjects.clear();
	Matrix identity;
	flattenScene(parser->getRootNode(), identity);
}

Shape* findShape(int shapeType) {
	Shape* shape = NULL;
	switch (shapeType) {
	case SHAPE_CUBE:
		shape = cube;
		break;
	case SHAPE_CYLINDER:
		shape = cylinder;
		break;
	case SHAPE_CONE:
		shape = cone;
		break;
	case SHAPE_SPHERE:
		shape = sphere;
		break;
	case SHAPE_SPECIAL1:
		shape = cube;
		break;
	default:
		shape = cube;
	}
	return shape;
}


/***************************************** myGlutIdle() ***********/

void myGlutIdle(void)
{
	/* According to the GLUT specification, the current window is
	undefined during an idle callback.  So we need to explicitly change
	it if necessary */
	if (glutGetWindow() != main_window)
		glutSetWindow(main_window);

	glutPostRedisplay();
}


/**************************************** myGlutReshape() *************/

void myGlutReshape(int x, int y)
{
	float xy_aspect;

	xy_aspect = (float)x / (float)y;
	glViewport(0, 0, x, y);
	camera->SetScreenSize(x, y);

	screenWidth = x;
	screenHeight = y;

	glutPostRedisplay();
}


/***************************************** setupCamera() *****************/
void setupCamera()
{
	SceneCameraData cameraData;
	parser->getCameraData(cameraData);

	camera->Reset();
	camera->SetViewAngle(cameraData.heightAngle);
	if (cameraData.isDir == true) {
		camera->Orient(cameraData.pos, cameraData.look, cameraData.up);
	}
	else {
		camera->Orient(cameraData.pos, cameraData.lookAt, cameraData.up);
	}

	viewAngle = camera->GetViewAngle();
	Point eyeP = camera->GetEyePoint();
	Vector lookV = camera->GetLookVector();
	eyeX = eyeP[0];
	eyeY = eyeP[1];
	eyeZ = eyeP[2];
	lookX = lookV[0];
	lookY = lookV[1];
	lookZ = lookV[2];
	camRotU = 0;
	camRotV = 0;
	camRotW = 0;
	GLUI_Master.sync_live_all();
}

void updateCamera()
{
	camera->Reset();

	Point guiEye (eyeX, eyeY, eyeZ);
	Point guiLook(lookX, lookY, lookZ);
	camera->SetViewAngle(viewAngle);
    Vector v = camera->GetUpVector();
	camera->Orient(guiEye, guiLook, v);
	camera->RotateU(camRotU);
	camera->RotateV(camRotV);
	camera->RotateW(camRotW);

	camera->computeCamera2WorldMatrix();
}

void flattenScene(SceneNode* node, Matrix compositeMatrix)
{
	std::vector<SceneTransformation*> transVec = node->transformations;
	for (int i = 0; i<transVec.size(); i++) {
		SceneTransformation* trans = transVec[i];
		switch (trans->type) {
		case TRANSFORMATION_TRANSLATE:
			compositeMatrix = compositeMatrix * trans_mat(trans->translate);
			break;
		case TRANSFORMATION_SCALE:
			compositeMatrix = compositeMatrix * scale_mat(trans->scale);
			break;
		case TRANSFORMATION_ROTATE:
			compositeMatrix = compositeMatrix * rot_mat(trans->rotate, trans->angle);
			break;
		case TRANSFORMATION_MATRIX:
			compositeMatrix = compositeMatrix * trans->matrix;
			break;
		}
	}

	SceneGlobalData globalData;
	parser->getGlobalData(globalData);
	std::vector<ScenePrimitive*> objectVec = node->primitives;
	for (int j = 0; j<objectVec.size(); j++) {
		SceneObject tempObj;
		tempObj.transform = compositeMatrix;
		tempObj.invTransform = invert(compositeMatrix);

		tempObj.material = objectVec[j]->material;
		tempObj.material.cAmbient.r *= globalData.ka;
		tempObj.material.cAmbient.g *= globalData.ka;
		tempObj.material.cAmbient.b *= globalData.ka;
		tempObj.material.cDiffuse.r *= globalData.kd;
		tempObj.material.cDiffuse.g *= globalData.kd;
		tempObj.material.cDiffuse.b *= globalData.kd;
		tempObj.material.cSpecular.r *= globalData.ks;
		tempObj.material.cSpecular.g *= globalData.ks;
		tempObj.material.cSpecular.b *= globalData.ks;
		tempObj.material.cReflective.r *= globalData.ks;
		tempObj.material.cReflective.g *= globalData.ks;
		tempObj.material.cReflective.b *= globalData.ks;
		tempObj.material.cTransparent.r *= globalData.kt;
		tempObj.material.cTransparent.g *= globalData.kt;
		tempObj.material.cTransparent.b *= globalData.kt;

		tempObj.shape = findShape(objectVec[j]->type);
		sceneObjects.push_back(tempObj);
	}

	std::vector<SceneNode*> childrenVec = node->children;
	for (int k = 0; k<childrenVec.size(); k++) {
		flattenScene(childrenVec[k], compositeMatrix);
	}
}

/***************************************** myGlutDisplay() *****************/

void myGlutDisplay(void)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (parser == NULL) {
		return;
	}

	if (pixels == NULL) {
		return;
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glDrawPixels(pixelWidth, pixelHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels);
	glutSwapBuffers();
}

void onExit()
{
	delete cube;
	delete cylinder;
	delete cone;
	delete sphere;
	delete camera;
	if (parser != NULL) {
		delete parser;
	}
	if (pixels != NULL) {
		delete pixels;
	}
}

/**************************************** main() ********************/

int main(int argc, char* argv[])
{
	atexit(onExit);

	/****************************************/
	/*   Initialize GLUT and create window  */
	/****************************************/

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowPosition(50, 50);
	glutInitWindowSize(500, 500);

	main_window = glutCreateWindow("CSI 4341: Assignment 5");
	glutDisplayFunc(myGlutDisplay);
	glutReshapeFunc(myGlutReshape);

	glShadeModel (GL_SMOOTH);

	glEnable(GL_DEPTH_TEST);

	// Specular reflections will be off without this, since OpenGL calculates
	// specular highlights using an infinitely far away camera by default, not
	// the actual location of the camera
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);

	// Show all ambient light for the entire scene (not one by default)
	GLfloat one[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, one);

	glPolygonOffset(1, 1);



	/****************************************/
	/*         Here's the GLUI code         */
	/****************************************/

	GLUI* glui = GLUI_Master.create_glui("GLUI");

	filenameTextField = new GLUI_EditText( glui, "Filename:", filenamePath);
	filenameTextField->set_w(300);
	glui->add_button("Load", 0, callback_load);
	glui->add_button("Start!", 0, callback_start);
	glui->add_checkbox("Isect Only", &isectOnly);
	
	GLUI_Panel *camera_panel = glui->add_panel("Camera");
	(new GLUI_Spinner(camera_panel, "RotateV:", &camRotV))
		->set_int_limits(-179, 179);
	(new GLUI_Spinner(camera_panel, "RotateU:", &camRotU))
		->set_int_limits(-179, 179);
	(new GLUI_Spinner(camera_panel, "RotateW:", &camRotW))
		->set_int_limits(-179, 179);
	(new GLUI_Spinner(camera_panel, "Angle:", &viewAngle))
		->set_int_limits(1, 179);

	glui->add_column_to_panel(camera_panel, true);

	GLUI_Spinner* eyex_widget = glui->add_spinner_to_panel(camera_panel, "EyeX:", GLUI_SPINNER_FLOAT, &eyeX);
	eyex_widget->set_float_limits(-10, 10);
	GLUI_Spinner* eyey_widget = glui->add_spinner_to_panel(camera_panel, "EyeY:", GLUI_SPINNER_FLOAT, &eyeY);
	eyey_widget->set_float_limits(-10, 10);
	GLUI_Spinner* eyez_widget = glui->add_spinner_to_panel(camera_panel, "EyeZ:", GLUI_SPINNER_FLOAT, &eyeZ);
	eyez_widget->set_float_limits(-10, 10);

	GLUI_Spinner* lookx_widget = glui->add_spinner_to_panel(camera_panel, "LookX:", GLUI_SPINNER_FLOAT, &lookX);
	lookx_widget->set_float_limits(-10, 10);
	GLUI_Spinner* looky_widget = glui->add_spinner_to_panel(camera_panel, "LookY:", GLUI_SPINNER_FLOAT, &lookY);
	looky_widget->set_float_limits(-10, 10);
	GLUI_Spinner* lookz_widget = glui->add_spinner_to_panel(camera_panel, "LookZ:", GLUI_SPINNER_FLOAT, &lookZ);
	lookz_widget->set_float_limits(-10, 10);

	glui->add_button("Quit", 0, (GLUI_Update_CB)exit);

	glui->set_main_gfx_window(main_window);

	/* We register the idle callback with GLUI, *not* with GLUT */
	GLUI_Master.set_glutIdleFunc(myGlutIdle);

	glutMainLoop();

	return EXIT_SUCCESS;
}



