//*************************************************************************
//	Include headers
//*************************************************************************
#include "./headers/glui.h"
#include "./headers/container.h"
#include <iostream>
#include <math.h>
#include <fstream>
#include <string>
#include <sstream>
#include <cfloat>
#include <vector>
using namespace std;
//*************************************************************************
//  Declarations
//*************************************************************************
const double PI = 3.141592653589793;
double minCoordY;
double model_scale = 0.0;// fit the model to the screen
bool hasTrans;
vector<Vert> vert_table;
vector<Face> face_table;
int fileType;
//-------------------------------------------------------------------------
//  Support Functions
//-------------------------------------------------------------------------
int char2int(char*c);
bool openfile(char *filepath);
double *calFaceNormal(double*p1, double*p2, double*p3);//	calculate the face normal
double *normalize(double *n);// normalize the normal
double findOverHang(vector<Vert> vertT, double *directionNorm);
void transalteModel(vector<Vert> vertT);
void saveFile();
//*************************************************************************
//  GLUT Declarations
//*************************************************************************
const float ANGFACT = { 1.0f };// rotation factor
const float SCLFACT = { 0.005f };// scaling factor
const float MINSCALE = { 0.05f };// minimum scale value
bool LmouseDown = false;// mouse left key down
bool MmouseDown = false;// mouse mid key down
bool RmouseDown = false;// mouse right key down
int	Xmouse, Ymouse;// mouse values
float Xrot, Yrot;// rotation angles in degrees
void init();// initialization
//-------------------------------------------------------------------------
//  CallBack Functions
//-------------------------------------------------------------------------
void display(void);// isplay the model
void mouse(int button, int state, int x, int y);// mouse click control
void motion(int x, int y);// mouse motion control
void reshape(int w, int h);// called when window size changed
void control_cb(int control);// glui control
//-------------------------------------------------------------------------
//  Define the window position on screen
//-------------------------------------------------------------------------
int window_x;
int window_y;
//-------------------------------------------------------------------------
//  Variables representing the window size
//-------------------------------------------------------------------------
int window_width = 1000;
int window_height = 800;
//-------------------------------------------------------------------------
//  Display mode 0:FLAT, 1:SMOOTH, 2:WIREFRAME, 3:SHADED WITH EDGES
//-------------------------------------------------------------------------
int mode = 0;
//-------------------------------------------------------------------------
//  GLUI control window
//-------------------------------------------------------------------------
GLuint main_window;//	The id of the main window
char *window_title = const_cast<char*>("GLUI");//	window title
//*************************************************************************
//  GLUI Declarations
//*************************************************************************
GLUI *glui_window;// pointer to the right GLUI window
GLUI_EditText *statustxt;// text shows the status
GLUI_EditText *loadtext;// file open path
GLUI_EditText *savetext;// file save path
GLUI_EditText *critAngle;
GLUI_Button *loadbtn;// open button
GLUI_Button *savebtn;// save button
GLUI_Button *analysisbtn;// 
GLUI_Button *translatebtn;// 
//-------------------------------------------------------------------------
//  Declare live variables (related to GLUI)
//-------------------------------------------------------------------------
char *string_list[] = { (char*)"Flat Shaded", (char*)"Smooth Shaded", (char*)"Wireframe", (char*)"shaded with Mesh Edges" };// String list to hold render options
int displayf = 1;// Variable to control the display of the mesh
int listbox_item_id = 1;// Id of the selected item in the list box
float rotation_matrix[16]// Rotation Matrix Live Variable Array
= { 1.0, 0.0, 0.0, 0.0,
0.0, 1.0, 0.0, 0.0,
0.0, 0.0, 1.0, 0.0,
0.0, 0.0, 0.0, 1.0 };
float translate_xy[2] = { 0.0, 0.0 };// Translation XY Live Variable
float translate_z = 1.0;// Translation Z Live Variable
float zoom = 1.0;// Spinner Zoom Live Variable
void setupGLUI();// Set up the GLUI window and its components
void idle();// Idle callack function
void glui_callback(int arg);// Declare callbacks related to GLUI
//-------------------------------------------------------------------------
//  Declare the IDs of controls generating callbacks
//-------------------------------------------------------------------------
enum
{
	DISPLAY_LISTBOX = 0,
	TRANSLATION_XY,
	TRANSLATION_Z,
	ROTATION,
	ZOOM_SPINNER,
	QUIT_BUTTON
};
//-------------------------------------------------------------------------
//  Support Functions
//-------------------------------------------------------------------------
void centerOnScreen();// center the windows in the screen
void setuplight();// set up light
void drawObject();// prepare for the draw
void draw();// draw the model according to the mode
//*************************************************************************
//  GLUT Functions.rotation_matrix
//*************************************************************************
void init()
{
	int x, y, wq, hq;
	GLUI_Master.get_viewport_area(&x, &y, &wq, &hq);
	glClearColor(0.234, 0.322, 0.689, 1.0);
	glViewport(x, y, wq, hq);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (window_width <= window_height)
	{
		glOrtho(-0.7, 0.7, -0.7*hq / wq, 0.7*hq / wq, -10.0, 10.0);
	}
	else
	{
		glOrtho(-0.7*wq / hq, 0.7*wq / hq, -0.7, 0.7, -10.0, 10.0);
	}
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}
//-------------------------------------------------------------------------
//  This function is passed to the glutReshapeFunc and is called
//  whenever the window is resized.
//-------------------------------------------------------------------------
void reshape(int w, int h)
{
	window_width = w;
	window_height = h;
	int x, y, wq, hq;
	GLUI_Master.get_viewport_area(&x, &y, &wq, &hq);
	glClearColor(0.234, 0.322, 0.689, 1.0);//  Set the frame buffer clear color to black.
	glViewport(x, y, wq, hq);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (window_width <= window_height)
	{
		glOrtho(-0.7, 0.7, -0.7*hq / wq, 0.7*hq / wq, -10.0, 10.0);
	}
	else
	{
		glOrtho(-0.7*wq / hq, 0.7*wq / hq, -0.7, 0.7, -10.0, 10.0);
	}
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glutReshapeWindow(w, h);
	glui_window->refresh();
	glutPostRedisplay();
}
//-------------------------------------------------------------------------
//  This function is passed to the glutMouseFunc and is called
//  whenever the mouse is clicked.
//-------------------------------------------------------------------------
void mouse(int button, int state, int x, int y)
{
	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		switch (state)
		{
		case GLUT_DOWN:
			LmouseDown = true;
			Xmouse = x;
			Ymouse = y;
			break;
		case GLUT_UP:
			LmouseDown = false;
			break;
		}

		break;
	case GLUT_MIDDLE_BUTTON:
		switch (state)
		{
		case GLUT_DOWN:
			MmouseDown = true;
			break;
		case GLUT_UP:
			MmouseDown = false;
			break;
		}
		break;
	case GLUT_RIGHT_BUTTON:
		switch (state)
		{
		case GLUT_DOWN:
			RmouseDown = true;
			break;
		case GLUT_UP:
			RmouseDown = false;
			break;
		}
		break;
	}
}
//-------------------------------------------------------------------------
//  This function is passed to the glutMotionFunc and is called
//  whenever the mouse is dragged.
//-------------------------------------------------------------------------
void motion(int x, int y)
{
	int dx = x - Xmouse;
	int dy = y - Ymouse;
	if (LmouseDown && !MmouseDown&& !RmouseDown)
	{
		Xrot += (ANGFACT*dy);
		Yrot += (ANGFACT*dx);
	}
	if (!LmouseDown && MmouseDown&& !RmouseDown)
	{
		zoom += SCLFACT * (float)(dx - dy);
		if (zoom < MINSCALE)
		{
			zoom = MINSCALE;
		}
	}
	if (!LmouseDown && !MmouseDown&& RmouseDown)
	{
		translate_xy[0] += dx*0.001;
		translate_xy[1] -= dy*0.001;
	}
	Xmouse = x;
	Ymouse = y;
}
//	Set up light and material
void setuplight()
{
	//-------------------------------------------------------------------------
	//  Lighting set up
	//-------------------------------------------------------------------------
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);
	//-------------------------------------------------------------------------
	//  Set the light position
	//-------------------------------------------------------------------------
	GLfloat light_position[] = { 0.0, 0.0, 1.0, 1.0 };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	//-------------------------------------------------------------------------
	//  Set lighting intensity and color
	//-------------------------------------------------------------------------
	GLfloat light_ambient[] = { 0.19225f, 0.19225f, 0.19225f, 1.0f };
	GLfloat light_diffuse[] = { 0.50754f, 0.50754f, 0.50754f, 1.0f };
	GLfloat light_specular[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	GLfloat material_ambient[] = { 0.19225f, 0.19225f, 0.19225f, 1.0f };
	GLfloat material_diffuse[] = { 0.50754f, 0.50754f, 0.50754f, 1.0f };
	GLfloat material_specular[] = { 0.508273f, 0.508273f, 0.508273f, 1.0f };
	GLfloat material_shininess = { 10.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	//-------------------------------------------------------------------------
	//  Set material properties
	//-------------------------------------------------------------------------
	glMaterialfv(GL_FRONT, GL_AMBIENT, material_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular);
	glMaterialf(GL_FRONT, GL_SHININESS, material_shininess);
}
//-------------------------------------------------------------------------
//  This function sets the window x and y coordinates
//  such that the window becomes centered
//-------------------------------------------------------------------------
void centerOnScreen()
{
	window_x = (glutGet(GLUT_SCREEN_WIDTH) - window_width) / 2;
	window_y = (glutGet(GLUT_SCREEN_HEIGHT) - window_height) / 2;
}
//-------------------------------------------------------------------------
//  This function is passed to glutDisplayFunc in order to display
//	OpenGL contents on the window.
//-------------------------------------------------------------------------
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// Clear the window
	glEnable(GL_DEPTH_TEST);
	setuplight();// Set up the lighting
	drawObject();// Draw object
	glutSwapBuffers();// Swap contents of backward and forward frame buffers
}
//-------------------------------------------------------------------------
//  Draws the object.
//-------------------------------------------------------------------------
void drawObject()
{
	// Draw the object only if the Draw check box is selected
	if (displayf)
	{
		glPushMatrix();// Push the current matrix into the model view stack
		glTranslatef(translate_xy[0], translate_xy[1]-0.3, -translate_z);// Apply the translation
		glRotatef((GLfloat)Yrot, 0.0, 1.0, 0.0);
		glRotatef((GLfloat)Xrot, 1.0, 0.0, 0.0);
		glMultMatrixf(rotation_matrix);// Apply the rotation matrix
		glScalef(zoom, zoom, zoom);// Apply the scaling
		//-------------------------------------------------------------------------
		//  Apply the mode
		//-------------------------------------------------------------------------
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		// Draw the meshes
		draw();
		// End of draw
		glPopMatrix();
		glFlush();
	}
}
//-------------------------------------------------------------------------
//  This function will draw the meshed according to
//	the winged edge structure
//-------------------------------------------------------------------------
void draw()
{
	glBegin(GL_LINES);
	// draw line for x axis
	glColor3f(1.0, 0.0, 0.0);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(1.0, 0.0, 0.0);
	// draw line for y axis
	glColor3f(0.0, 1.0, 0.0);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 1.0, 0.0);
	// draw line for Z axis
	glColor3f(0.0, 0.0, 1.0);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 0.0, 1.0);
	glEnd();
	glEnable(GL_FLAT);
	glShadeModel(GL_FLAT);
	//glColor3f(0.65, 0.5, 0.39);
	double scale = (model_scale / (vert_table.size())) * 3;
	for (unsigned int i = 0; i < face_table.size(); i++)
	{
		if (face_table[i].GetOverHange())
		{
			glColor3f(0.9, 0.0, 0.0);
		}
		else
		{
			glColor3f(0.6, 0.6, 0.6);
		}
		int *vlist = face_table[i].GetAllVert();
		double *p1 = vert_table[vlist[0]].GetCoord();
		double *p2 = vert_table[vlist[1]].GetCoord();
		double *p3 = vert_table[vlist[2]].GetCoord();
		double *norm = face_table[i].GetNorm();

		glBegin(GL_TRIANGLES);
		glNormal3f(norm[0], norm[1], norm[2]);
		glVertex3f(p1[0] / scale, p1[1] / scale, p1[2] / scale);
		glVertex3f(p2[0] / scale, p2[1] / scale, p2[2] / scale);
		glVertex3f(p3[0] / scale, p3[1] / scale, p3[2] / scale);
		glEnd();
	}
}

//-------------------------------------------------------------------------
//  Functions for GLUI controls
//-------------------------------------------------------------------------
void control_cb(int control) {
#define SAVEID 500
#define LOADID 501
#define TRANSLATE 502
#define OVERHANG 503
	if (control == LOADID)
	{
		char *filepath = (char*)loadtext->get_text();
		cout << "File Path: " << filepath << endl;
		openfile(filepath);
	}

	if (control == SAVEID)
	{
		statustxt->set_text("Saving file...");
		saveFile();
		savetext->set_text("../smf/");
	}

	if (control == TRANSLATE)
	{
		transalteModel(vert_table);
	}
	if (control == OVERHANG)
	{
		double directionNorm[] = { 0.0, 1.0, 0.0 };
		findOverHang(vert_table, directionNorm);
	}
}
//*************************************************************************
//  GLUI Functions.
//*************************************************************************
void setupGLUI()
{
	GLUI_Master.set_glutIdleFunc(idle);// Set idle function
	//---------------------------------------------------------------------
	// Right GLUI window
	//---------------------------------------------------------------------
	glui_window = GLUI_Master.create_glui_subwindow(main_window, GLUI_SUBWINDOW_RIGHT);// Create GLUI window
	//---------------------------------------------------------------------
	// 'Status' Panel
	//---------------------------------------------------------------------
	// Add the 'File Property' Panel to the GLUI window
	GLUI_Panel *sts_panel = glui_window->add_panel("Status");
	statustxt = glui_window->add_edittext_to_panel(sts_panel, "Message: ");
	statustxt->set_text("Please open a file");
	statustxt->set_alignment(GLUI_ALIGN_CENTER);
	statustxt->set_w(200);
	statustxt->disable();
	//---------------------------------------------------------------------
	// 'File Property' Panel
	//---------------------------------------------------------------------
	// Add the 'File Property' Panel to the GLUI window
	GLUI_Panel *fp_panel = glui_window->add_panel("File Property");
	// Load file controls
	loadtext = glui_window->add_edittext_to_panel(fp_panel, "Load Path: ", GLUI_EDITTEXT_TEXT);
	loadtext->set_text("D:\\model\\bird.obj");
	loadtext->set_w(200);
	loadtext->set_alignment(GLUI_ALIGN_LEFT);
	loadbtn = glui_window->add_button_to_panel(fp_panel, "Open", LOADID, control_cb);
	loadbtn->set_alignment(GLUI_ALIGN_LEFT);
	loadbtn->set_w(200);
	// Save file controls
	savetext = glui_window->add_edittext_to_panel(fp_panel, "Save Path: ", GLUI_EDITTEXT_TEXT);
	savetext->set_text("../smf/");
	savetext->set_w(200);
	savetext->set_alignment(GLUI_ALIGN_LEFT);
	savebtn = glui_window->add_button_to_panel(fp_panel, "Save", SAVEID, control_cb);
	savebtn->set_alignment(GLUI_ALIGN_LEFT);
	savebtn->set_w(200);
	//---------------------------------------------------------------------
	// 'Analysis' Panel
	//---------------------------------------------------------------------
	GLUI_Panel *analysis_panel = glui_window->add_panel("Analysis");
	critAngle = glui_window->add_edittext_to_panel(analysis_panel, "Critical Angle: ", GLUI_EDITTEXT_TEXT);
	critAngle->set_text("45");
	critAngle->set_w(200);
	translatebtn=glui_window->add_button_to_panel(analysis_panel, "Translate", TRANSLATE, control_cb);
	translatebtn->set_w(200);
	analysisbtn = glui_window->add_button_to_panel(analysis_panel, "Overhange", OVERHANG, control_cb);
	analysisbtn->set_w(200);
	//---------------------------------------------------------------------
	// 'Camera' Panel
	//---------------------------------------------------------------------
	// Add the 'Camera' Panel to the GLUI window
	GLUI_Panel *camera_panel = glui_window->add_panel("Camera Control");
	// Create Camera panel 1 that will contain the Translation controls
	GLUI_Panel *camera_panel1 = glui_window->add_panel_to_panel(camera_panel, "");
	// Add the xy translation control
	GLUI_Translation *translation_xy = glui_window->add_translation_to_panel(camera_panel1, "Translation XY", GLUI_TRANSLATION_XY, translate_xy, TRANSLATION_XY, glui_callback);
	// Set the translation speed
	translation_xy->set_speed(0.005);
	// Add column, but don't draw it
	glui_window->add_column_to_panel(camera_panel1, false);
	// Add the z translation control
	GLUI_Translation *translation_z = glui_window->add_translation_to_panel(camera_panel1, "Translation Z", GLUI_TRANSLATION_Z, &translate_z, TRANSLATION_Z, glui_callback);
	// Set the translation speed
	translation_z->set_speed(0.01);
	// Create camera_panel 2 that will contain the rotation and spinner controls
	GLUI_Panel *camera_panel2 = glui_window->add_panel_to_panel(camera_panel, "");
	// Add the rotation control
	glui_window->add_rotation_to_panel(camera_panel2, "Rotation", rotation_matrix, ROTATION, glui_callback);
	// Add a separator
	glui_window->add_separator_to_panel(camera_panel2);
	// Add the zoom spinner
	GLUI_Spinner *spinner = glui_window->add_spinner_to_panel(camera_panel2, "Zoom", GLUI_SPINNER_FLOAT, &zoom, ZOOM_SPINNER, glui_callback);
	// Set the limits for the spinner
	spinner->set_float_limits(0.0, 4.0);
	spinner->set_alignment(GLUI_ALIGN_LEFT);
	//---------------------------------------------------------------------
	// 'Quit' Button
	//---------------------------------------------------------------------
	// Add the Quit Button
	glui_window->add_button("Quit", QUIT_BUTTON, glui_callback);
	// Let the GLUI window know where its main graphics window is
	glui_window->set_main_gfx_window(main_window);
}
//-------------------------------------------------------------------------
//  GLUI callback function.
//-------------------------------------------------------------------------
void glui_callback(int control_id)
{
	// Behave based on control ID
	switch (control_id)
	{
		// Quit Button clicked
	case QUIT_BUTTON:
		exit(1);
		break;
	}
}
//-------------------------------------------------------------------------
//  Idle Callback function.
//
//  Set the main_window as the current window to avoid sending the
//  redisplay to the GLUI window rather than the GLUT window.
//  Call the Sleep function to stop the GLUI program from causing
//  starvation.
//-------------------------------------------------------------------------
void idle()
{
	glutSetWindow(main_window);
	glutPostRedisplay();
}
//*************************************************************************
//  Program Main method.
//*************************************************************************
int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	// Window becomes centered
	centerOnScreen();
	glutInitWindowSize(window_width, window_height);
	glutInitWindowPosition(window_x, window_y);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
	main_window = glutCreateWindow(window_title);
	// Set OpenGL context initial state.
	init();
	// Set the GLUT callback functions
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	// Setup all GLUI stuff
	setupGLUI();
	// Start GLUT event processing loop
	glutMainLoop();
	return 0;
}
//*************************************************************************
//  Support method.
//*************************************************************************
bool openfile(char *filepath)
{
	fileType = 0;
	hasTrans = false;
	minCoordY = DBL_MAX;
	model_scale = 0;
	vert_table.clear();
	face_table.clear();
	//open file and read to memory
	FILE * file = fopen(filepath, "r");
	if (file == NULL)
	{
		printf("Impossible to open the file !\n");
		return false;
	}

	while (true)
	{
		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.
		if (strcmp(lineHeader, "v") == 0)
		{
			double x, y, z;
			fscanf(file, "%lf %lf %lf\n", &x, &y, &z);
			if (y < minCoordY)
			{
				minCoordY = y;
			}
			model_scale += sqrt(x*x + y*y + z*z);
			Vert *v = new Vert(x, y, z);
			vert_table.push_back(*v);
		}
		else if (strcmp(lineHeader, "f") == 0)
		{
			int *vlist = new int[3];
			char *temp1 = new char[15];
			char *temp2 = new char[15];
			char *temp3 = new char[15];
			
			int matches = fscanf(file, "%s %s %s\n", temp1, temp2, temp3);
			if (matches != 3)
			{
				printf("File can't be read by our simple parser : ( Try exporting with other options\n");
				return false;
			}
			int v1 = char2int(temp1);
			int v2 = char2int(temp2);
			int v3 = char2int(temp3);

			vlist[0] = v1;
			vlist[1] = v2;
			vlist[2] = v3;
			Face *f = new Face(vlist[0] - 1, vlist[1] - 1, vlist[2] - 1);
			f->SetNorm(calFaceNormal(vert_table[vlist[0] - 1].GetCoord(), vert_table[vlist[1] - 1].GetCoord(), vert_table[vlist[2] - 1].GetCoord()));
			face_table.push_back(*f);
		}
	}
	return true;
}

void saveFile()
{
}

int char2int(char*c)
{
	char *t = new char[15];
	for (int i = 0; i < 15; i++)
	{
		if (c[i] == '/')
		{
			break;
		}
		t[i] = c[i];
	}

	int result = atoi(t);
	return result;
}

double *normalize(double *n)
{
	double *result = new double[3];
	result[0] = n[0] / sqrtf(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
	result[1] = n[1] / sqrtf(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
	result[2] = n[2] / sqrtf(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
	return result;
}

double *calFaceNormal(double*p1, double*p2, double*p3)
{
	double*v1 = new double[3];
	double*v2 = new double[3];
	v1[0] = p2[0] - p1[0];
	v1[1] = p2[1] - p1[1];
	v1[2] = p2[2] - p1[2];
	v2[0] = p3[0] - p1[0];
	v2[1] = p3[1] - p1[1];
	v2[2] = p3[2] - p1[2];
	double x1 = v1[0];
	double y1 = v1[1];
	double z1 = v1[2];
	double x2 = v2[0];
	double y2 = v2[1];
	double z2 = v2[2];
	double *result = new double[3];
	result[0] = y1*z2 - z1*y2;
	result[1] = z1*x2 - x1*z2;
	result[2] = x1*y2 - y1*x2;
	return normalize(result);
}

double calPlaneAngle(double *n1, double *n2)
{
	double number = abs(n1[0] * n2[0] + n1[1] * n2[1] + n1[2] * n2[2]) / (sqrt(n1[0] * n1[0] + n1[1] * n1[1] + n1[2] * n1[2])*sqrt(n2[0] * n2[0] + n2[1] * n2[1] + n2[2] * n2[2]));
	return acos(number)*(180 / PI);
}

bool IsSameDirection(double *n1, double *n2)
{
	return (n1[0] * n2[0] + n1[1] * n2[1] + n1[2] * n2[2]) > 0;
}

double findOverHang(vector<Vert> vertT, double *directionNorm)
{
	double cost = -1;
	for (int i = 0; i < face_table.size(); i++)
	{
		double *facenorm = face_table[i].GetNorm();
		if (abs(calPlaneAngle(facenorm, directionNorm) - 90) > 45 && !IsSameDirection(facenorm, directionNorm))
		{
			int*vlist = face_table[i].GetAllVert();
			for (int j = 0; j < 3; j++)
			{
				if (!vertT[vlist[j]].GetOverHang())
				{
					cost += vertT[vlist[j]].GetCoord()[1];
					vertT[vlist[j]].SetOverHang();
				}
			}
			face_table[i].SetAsOverHang();
		}
	}
	return cost;
}

void transalteModel(vector<Vert> vertT)
{
	if (!hasTrans)
	{
		for (int i = 0; i < vertT.size(); i++)
		{
			vertT[i].Transalte(abs(minCoordY));
		}
		hasTrans = true;
	}
}

