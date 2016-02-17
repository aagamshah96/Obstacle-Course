#include<bits/stdc++.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glad/glad.h>
#include <FTGL/ftgl.h>
#include <GLFW/glfw3.h>
#include <SOIL/SOIL.h>

using namespace std;

struct VAO {
	GLuint VertexArrayID;
	GLuint VertexBuffer;
	GLuint ColorBuffer;
	GLuint TextureBuffer;
	GLuint TextureID;

	GLenum PrimitiveMode; // GL_POINTS, GL_LINE_STRIP, GL_LINE_LOOP, GL_LINES, GL_LINE_STRIP_ADJACENCY, GL_LINES_ADJACENCY, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_TRIANGLES, GL_TRIANGLE_STRIP_ADJACENCY and GL_TRIANGLES_ADJACENCY
	GLenum FillMode; // GL_FILL, GL_LINE
	int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID; // For use with normal shader
	GLuint TexMatrixID; // For use with texture shader
} Matrices;

struct FTGLFont {
	FTFont* font;
	GLuint fontMatrixID;
	GLuint fontColorID;
} GL3Font;

GLuint programID, fontProgramID, textureProgramID;



class Obstacle{
	public:
	int x;int y;int z;
	int appear;
	//Obstacle();
	
};

class Platform{
	public:
	int x;int y;int z;
	int n;float velocity;int velocityflag;
	void setValues(int xco,int yco,int zco,int number,float v,int vflag);
};

void Platform::setValues(int xco,int yco,int zco,int number,float v,int vflag)
{
	x=xco;y=yco;z=zco;
	n=number;velocity=v;
	velocityflag=vflag;
}


Platform cubes[10][10];


/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	cout << "Compiling shader : " <<  vertex_file_path << endl;
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage( max(InfoLogLength, int(1)) );
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	cout << VertexShaderErrorMessage.data() << endl;

	// Compile Fragment Shader
	cout << "Compiling shader : " << fragment_file_path << endl;
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage( max(InfoLogLength, int(1)) );
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	cout << FragmentShaderErrorMessage.data() << endl;

	// Link the program
	cout << "Linking program" << endl;
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	cout << ProgramErrorMessage.data() << endl;

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
	cout << "Error: " << description << endl;
}

void quit(GLFWwindow *window)
{
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}

glm::vec3 getRGBfromHue (int hue)
{
	float intp;
	float fracp = modff(hue/60.0, &intp);
	float x = 1.0 - abs((float)((int)intp%2)+fracp-1.0);

	if (hue < 60)
		return glm::vec3(0,0,0);
	else if (hue < 120)
		return glm::vec3(0,0,0);
	else if (hue < 180)
		return glm::vec3(0,0,0);
	else if (hue < 240)
		return glm::vec3(0,0,0);
	else if (hue < 300)
		return glm::vec3(0,0,0);
	else
		return glm::vec3(0,0,0);
}

/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
	struct VAO* vao = new struct VAO;
	vao->PrimitiveMode = primitive_mode;
	vao->NumVertices = numVertices;
	vao->FillMode = fill_mode;

	// Create Vertex Array Object
	// Should be done after CreateWindow and before any other GL calls
	glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
	glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
	glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

	glBindVertexArray (vao->VertexArrayID); // Bind the VAO
	glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
	glVertexAttribPointer(
						  0,                  // attribute 0. Vertices
						  3,                  // size (x,y,z)
						  GL_FLOAT,           // type
						  GL_FALSE,           // normalized?
						  0,                  // stride
						  (void*)0            // array buffer offset
						  );

	glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
	glVertexAttribPointer(
						  1,                  // attribute 1. Color
						  3,                  // size (r,g,b)
						  GL_FLOAT,           // type
						  GL_FALSE,           // normalized?
						  0,                  // stride
						  (void*)0            // array buffer offset
						  );

	return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
	GLfloat* color_buffer_data = new GLfloat [3*numVertices];
	for (int i=0; i<numVertices; i++) {
		color_buffer_data [3*i] = red;
		color_buffer_data [3*i + 1] = green;
		color_buffer_data [3*i + 2] = blue;
	}

	return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}


void reshapeWindow (GLFWwindow* window, int width, int height);


/*struct VAO* create3DTexturedObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* texture_buffer_data, GLuint textureID, GLenum fill_mode=GL_FILL)
{
	struct VAO* vao = new struct VAO;
	vao->PrimitiveMode = primitive_mode;
	vao->NumVertices = numVertices;
	vao->FillMode = fill_mode;
	vao->TextureID = textureID;

	// Create Vertex Array Object
	// Should be done after CreateWindow and before any other GL calls
	glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
	glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
	glGenBuffers (1, &(vao->TextureBuffer));  // VBO - textures

	glBindVertexArray (vao->VertexArrayID); // Bind the VAO
	glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
	glVertexAttribPointer(
						  0,                  // attribute 0. Vertices
						  3,                  // size (x,y,z)
						  GL_FLOAT,           // type
						  GL_FALSE,           // normalized?
						  0,                  // stride
						  (void*)0            // array buffer offset
						  );

	glBindBuffer (GL_ARRAY_BUFFER, vao->TextureBuffer); // Bind the VBO textures
	glBufferData (GL_ARRAY_BUFFER, 2*numVertices*sizeof(GLfloat), texture_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
	glVertexAttribPointer(
						  2,                  // attribute 2. Textures
						  2,                  // size (s,t)
						  GL_FLOAT,           // type
						  GL_FALSE,           // normalized?
						  0,                  // stride
						  (void*)0            // array buffer offset
						  );

	return vao;
}*/

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
	// Change the Fill Mode for this object
	glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

	// Bind the VAO to use
	glBindVertexArray (vao->VertexArrayID);

	// Enable Vertex Attribute 0 - 3d Vertices
	glEnableVertexAttribArray(0);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

	// Enable Vertex Attribute 1 - Color
	glEnableVertexAttribArray(1);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

	// Draw the geometry !
	glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/*void draw3DTexturedObject (struct VAO* vao)
{
	// Change the Fill Mode for this object
	glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

	// Bind the VAO to use
	glBindVertexArray (vao->VertexArrayID);

	// Enable Vertex Attribute 0 - 3d Vertices
	glEnableVertexAttribArray(0);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

	// Bind Textures using texture units
	glBindTexture(GL_TEXTURE_2D, vao->TextureID);

	// Enable Vertex Attribute 2 - Texture
	glEnableVertexAttribArray(2);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->TextureBuffer);

	// Draw the geometry !
	glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle

	// Unbind Textures to be safe
	glBindTexture(GL_TEXTURE_2D, 0);
}*/

/* Create an OpenGL Texture from an image */
/*
GLuint createTexture (const char* filename)
{
	GLuint TextureID;
	// Generate Texture Buffer
	glGenTextures(1, &TextureID);
	// All upcoming GL_TEXTURE_2D operations now have effect on our texture buffer
	glBindTexture(GL_TEXTURE_2D, TextureID);
	// Set our texture parameters
	// Set texture wrapping to GL_REPEAT
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// Set texture filtering (interpolation)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Load image and create OpenGL texture
	int twidth, theight;
	unsigned char* image = SOIL_load_image(filename, &twidth, &theight, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, twidth, theight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D); // Generate MipMaps to use
	SOIL_free_image_data(image); // Free the data read from file after creating opengl texture
	glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess it up

	return TextureID;
}
*/

int a[10][10];
float playerx=-0.5,playery=2.0,playerz=17.50,initiallx,initiallz,initially;
int tower=1,top=0,adven=0,follow=0,heli=0;
int lasti=-1,lastj=-1,n,incrementflag=0;
float increment=0;
int viewflag=0;
int fallflag=0,setflag=0;
int xset=0,zset=1;
int timeleft=20;
int angle=0;
float changex,changez;
int rd=20;
/**************************
 * Customizable functions *
 **************************/
int width = 1366;
int height = 768;
float triangle_rot_dir = 0;
float rectangle_rot_dir = 0;
bool triangle_rot_status = false;
bool rectangle_rot_status = false;
int gameover=0;
int level=1;
long long int score=0;

float dragx=0,dragy=0;
double mousex,mousey,initialx,initialy;
float zoom=0;
float upp=1.0;
float vchange=0.2;
int jump=0;
int hitx,hitz;

int hitflag=1;

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Function is called first on GLFW_PRESS.

	if (action == GLFW_PRESS&&jump!=1) {
		switch (key) {
			case GLFW_KEY_UP:
				
				if(cubes[9-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)].n==0||cubes[9-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)].n==1)
				fallflag=1;
				setflag=0;
				//cout<<(int)((playerx+0.5)/2)<<" "<<9-(int)((17.5-playerz)/2)<<endl;
				if(zset==1)
				{if(playery>=cubes[8-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)].velocity*2+2)
				playerz-=2;}
				else if(zset==-1)
				{if(playery>=cubes[10-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)].velocity*2+2)
				playerz+=2;}
				else if(xset==1)
				{if(playery>=cubes[9-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)+1].velocity*2+2)
				playerx+=2;}
				else if(xset==-1)
				{if(playery>=cubes[9-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)-1].velocity*2+2)
				playerx-=2;}
				break;
			case GLFW_KEY_DOWN:
				if(cubes[9-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)].n==0||cubes[9-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)].n==1)
				fallflag=1;
				setflag=0;
				if(zset==1)
				{if(playery>=cubes[10-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)].velocity*2+2)
				playerz+=2;}
				else if(zset==-1)
				{if(playery>=cubes[8-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)].velocity*2+2)
				playerz-=2;}
				else if(xset==1)
				{if(playery>=cubes[9-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)-1].velocity*2+2)
				playerx-=2;}
				else if(xset==-1)
				{if(playery>=cubes[9-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)+1].velocity*2+2)
				playerx+=2;}
				break;
			case GLFW_KEY_LEFT:
				/*if(playery>=cubes[9-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)-1].velocity*2+2)
				playerx-=2;
				if(cubes[9-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)].n==0)
				fallflag=1;*/
				setflag=0;
				if(zset==1)
				{xset=-1;zset=0;}
				else if(zset==-1)
				{xset=1;zset=0;}
				else if(xset==1)
				{xset=0;zset=1;}
				else if(xset==-1)
				{xset=0;zset=-1;}
				angle+=90;
				changex=1.0;	
				break;
			case GLFW_KEY_RIGHT:
				/*if(playery>=cubes[9-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)+1].velocity*2+2)
				playerx+=2;
				if(cubes[9-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)].n==0)
				fallflag=1;*/
				setflag=0;
				if(zset==1)
				{xset=1;zset=0;}
				else if(zset==-1)
				{xset=-1;zset=0;}
				else if(xset==1)
				{xset=0;zset=-1;}
				else if(xset==-1)
				{xset=0;zset=1;}
				angle-=90;
				changex=-1.0;
				break;
			case GLFW_KEY_V:
				if(viewflag==0)
				{top=1;tower=0;viewflag=1;}
				else if(viewflag==1)
				{top=0;adven=1;viewflag=2;}
				else if(viewflag==2)
				{follow=1;adven=0;viewflag=3;}
				else if(viewflag==3)
				{heli=1;follow=0;viewflag=4;}
				else if(viewflag==4)
				{tower=1;heli=0;viewflag=0;}
				reshapeWindow(window,width,height);
				break;
			case GLFW_KEY_SPACE:
				if(cubes[9-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)].n!=0&&cubes[9-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)].n!=1)
				jump=1;
				initiallx=playerx;
				initiallz=playerz;
				initially=playery;
			default:
				break;
		}
	}
	else if (action == GLFW_RELEASE) {
		switch (key) {
			case GLFW_KEY_ESCAPE:
				quit(window);
				break;
			
			default:
				break;
		}
	}
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
			quit(window);
			break;
		default:
			break;
	}
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	zoom+=yoffset*0.5;
	reshapeWindow (window, width, height);
	
}



/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
	switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			if (action == GLFW_PRESS){
			
				setflag=0;
				//cout<<(int)((playerx+0.5)/2)<<" "<<9-(int)((17.5-playerz)/2)<<endl;
				if(zset==1)
				{if(playery>=cubes[8-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)].velocity*2+2)
				playerz-=2;}
				else if(zset==-1)
				{if(playery>=cubes[10-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)].velocity*2+2)
				playerz+=2;}
				else if(xset==1)
				{if(playery>=cubes[9-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)+1].velocity*2+2)
				playerx+=2;}
				else if(xset==-1)
				{if(playery>=cubes[9-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)-1].velocity*2+2)
				playerx-=2;}	
			}
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_PRESS) {
                 initialx=mousex;
		 initialy=mousey;
		setflag=0;
				if(zset==1)
				{xset=1;zset=0;}
				else if(zset==-1)
				{xset=-1;zset=0;}
				else if(xset==1)
				{xset=0;zset=-1;}
				else if(xset==-1)
				{xset=0;zset=1;}
				angle-=90;
				changex=-1.0;
				if(cubes[9-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)].n==0)
				fallflag=1;	
		 
            }
            if (action == GLFW_RELEASE) {
            	dragx+=mousex-initialx;
     		dragy+=mousey-initialy;
		//cout<<dragx<<" "<<dragy<<endl;
            }
            break;
	 case GLFW_MOUSE_BUTTON_MIDDLE:
            if (action == GLFW_PRESS) {
            	if(cubes[9-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)].n!=0&&cubes[9-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)].n!=1)
				jump=1;
				initiallx=playerx;
				initiallz=playerz;
				initially=playery;
            	}
		if (action == GLFW_RELEASE) {
			
		}
		default:
			break;
	}
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
	int fbwidth=width, fbheight=height;
	/* With Retina display on Mac OS X, GLFW's FramebufferSize
	 is different from WindowSize */
	glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	 glLoadIdentity ();
	 gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
	// Perspective projection for 3D views
	// Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

	// Ortho projection for 2D views
	Matrices.projection = glm::ortho(-10.0-(adven*(-8.75))-(follow*(-5.0))+zoom*heli, 10.0+(adven*(-7.25))+(follow*(-5.0))-zoom*heli, -5.0-((adven*-8)/2)-((follow*-5)/2)+zoom*heli/2, 5.0+((adven*-8)/2)+((follow*-5)/2)-zoom*heli/2, 0.1, 500.0);
}

VAO *triangle, *rectangle, *rectangle1, *cube, *player, *cube3;


void createCube()
{
  static const GLfloat vertex_buffer_data [] = {
 -1,0,0,
0,0,0,
-1,-2,0,

-1,-2,0,
0,0,0,
0,-2,0,

-1,-2,0,
-1,0,0,
-1,0,-1,

-1,0,-1,
-1,-2,0,
-1,-2,-1,

-1,0,-1,
-1,-2,-1,
0,0,-1,

0,0,-1,
-1,-2,-1,
0,-2,-1,

0,0,-1,
0,0,0,
0,-2,0,

0,-2,0,
0,0,-1,
0,-2,-1,

-1,0,0,
-1,0,-1,
0,0,-1,

0,0,-1,
-1,0,0,
0,0,0,

-1,-2,0,
0,-2,-1,
-1,-2,-1,

0,-2,-1,
-1,-2,0,
0,-2,0

  };

  static const GLfloat color_buffer_data [] = {
    1,1,1, // color 1
    1,1,1, // color 2
    1,1,1, // color 3

    1,1,1, // color 3
    1,1,1, // color 4
    1,1,1,  // color 1

1,1,1, // color 1
    1,1,1, // color 2
    1,1,1, // color 3

    1,1,1, // color 3
    1,1,1, // color 4
    1,1,1,  // color 1

1,1,1, // color 1
    1,1,1, // color 2
    1,1,1, // color 3

    1,1,1, // color 3
    1,1,1, // color 4
    1,1,1,  // color 1

1,1,1, // color 1
    1,1,1, // color 2
    1,1,1, // color 3

    1,1,1, // color 3
    1,1,1, // color 4
    1,1,1,  // color 1

1,1,1, // color 1
    1,1,1, // color 2
    1,1,1, // color 3

    1,1,1, // color 3
    1,1,1, // color 4
    1,1,1,  // color 1

1,1,1, // color 1
    1,1,1, // color 2
    1,1,1, // color 3

    1,1,1, // color 3
    1,1,1, // color 4
    1,1,1,  // color 1

  };

cube3 = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);

}



// Creates the triangle object used in this sample code
void createTriangle ()
{
	/* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

	/* Define vertex array as used in glBegin (GL_TRIANGLES) */
	static const GLfloat vertex_buffer_data [] = {
		0, 1,0, // vertex 0
		-1,-1,0, // vertex 1
		1,-1,0, // vertex 2
	};

	static const GLfloat color_buffer_data [] = {
		1,1,1, // color 0
		1,1,1, // color 1
		1,1,1, // color 2
	};

	// create3DObject creates and returns a handle to a VAO that can be used later
	triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);
}


void createRectangle1 (float v1,float v2,float v3,float v4,float v5,float v6,float v7,float v8,float v9,float v10,float v11,float v12)
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    v1,v2,v3, // vertex 1
    v4,v5,v6, // vertex 2
    v7, v8,v9, // vertex 3

    v7, v8,v9,    // vertex 3
    v10, v11,v12, // vertex 4
    v1,v2,v3  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    1,1,1, // color 1
    1,1,1, // color 2
    1,1,1, // color 3

    1,1,1, // color 3
    1,1,1, // color 4
    1,1,1  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  rectangle1 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}


void createCuboid()
{
  static const GLfloat vertex_buffer_data [] = {
 -1,0,0,
0,0,0,
-1,-2,0,

-1,-2,0,
0,0,0,
0,-2,0,

-1,-2,0,
-1,0,0,
-1,0,-1,

-1,0,-1,
-1,-2,0,
-1,-2,-1,

-1,0,-1,
-1,-2,-1,
0,0,-1,

0,0,-1,
-1,-2,-1,
0,-2,-1,

0,0,-1,
0,0,0,
0,-2,0,

0,-2,0,
0,0,-1,
0,-2,-1,

-1,0,0,
-1,0,-1,
0,0,-1,

0,0,-1,
-1,0,0,
0,0,0,

-1,-2,0,
0,-2,-1,
-1,-2,-1,

0,-2,-1,
-1,-2,0,
0,-2,0

  };

  static const GLfloat color_buffer_data [] = {
    1,1,1, // color 1
    1,1,1, // color 2
    1,1,1, // color 3

    1,1,1, // color 3
    1,1,1, // color 4
    1,1,1,  // color 1

0.223,0.223,1, // color 1
    0.223,0.223,1, // color 2
    0.223,0.223,1, // color 3

0.223,0.223,1, // color 1
    0.223,0.223,1, // color 2
    0.223,0.223,1, // color 3

1,1,1, // color 1
    1,1,1, // color 2
    1,1,1, // color 3

    1,1,1, // color 3
    1,1,1, // color 4
    1,1,1,  // color 1

0.223,0.223,1, // color 1
    0.223,0.223,1, // color 2
    0.223,0.223,1, // color 3

0.223,0.223,1, // color 1
    0.223,0.223,1, // color 2
    0.223,0.223,1, // color 3

0.118,0.565,1, // color 1
    0.118,0.565,1, // color 2
    0.118,0.565,1, // color 3

0.118,0.565,1, // color 1
    0.118,0.565,1, // color 2
    0.118,0.565,1, // color 3

0.118,0.565,1, // color 1
    0.118,0.565,1, // color 2
    0.118,0.565,1, // color 3

0.118,0.565,1, // color 1
    0.118,0.565,1, // color 2
    0.118,0.565,1, // color 3

  };

cube = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);

}




void createPlayer()
{
  static const GLfloat vertex_buffer_data [] = {
 -1,0,0,
0,0,0,
-1,-2,0,

-1,-2,0,
0,0,0,
0,-2,0,

-1,-2,0,
-1,0,0,
-1,0,-1,

-1,0,-1,
-1,-2,0,
-1,-2,-1,

-1,0,-1,
-1,-2,-1,
0,0,-1,

0,0,-1,
-1,-2,-1,
0,-2,-1,

0,0,-1,
0,0,0,
0,-2,0,

0,-2,0,
0,0,-1,
0,-2,-1,

-1,0,0,
-1,0,-1,
0,0,-1,

0,0,-1,
-1,0,0,
0,0,0,

-1,-2,0,
0,-2,-1,
-1,-2,-1,

0,-2,-1,
-1,-2,0,
0,-2,0

  };

  static const GLfloat color_buffer_data [] = {
    0.244,1,0.244, // color 1
    0.244,1,0.244, // color 2
    0.244,1,0.244, // color 3

    1,0.647,0.000001, // color 1
    1,0.647,0.000001, // color 2
    1,0.647,0.000001, // color 3

0.244,1,0.244, // color 1
    0.244,1,0.244, // color 2
    0.244,1,0.244, // color 3

1,0.647,0.000001, // color 1
    1,0.647,0.000001, // color 2
    1,0.647,0.000001, // color 3

0.244,1,0.244, // color 1
    0.244,1,0.244, // color 2
    0.244,1,0.244, // color 3

1,0.647,0.000001, // color 1
    1,0.647,0.000001, // color 2
    1,0.647,0.000001, // color 3

0.244,1,0.244, // color 1
    0.244,1,0.244, // color 2
    0.244,1,0.244, // color 3

1,0.647,0.000001, // color 1
    1,0.647,0.000001, // color 2
    1,0.647,0.000001, // color 3

0.244,1,0.244, // color 1
    0,0,0, // color 2
    0,0,0, // color 3

0,0,0, // color 1
    1,0.647,0.000001, // color 2
    1,0.647,0.000001, // color 3

1,0.647,0.000001, // color 1
    1,0.647,0.000001, // color 2
    1,0.647,0.000001, // color 3

0.244,1,0.244, // color 1
    0.244,1,0.244, // color 2
    0.244,1,0.244 // color 3

  };

player = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);

}



// Creates the rectangle object used in this sample code
/*void createRectangle (GLuint textureID)
{
	// GL3 accepts only Triangles. Quads are not supported
	static const GLfloat vertex_buffer_data [] = {
		-1.2,-1,0, // vertex 1
		1.2,-1,0, // vertex 2
		1.2, 1,0, // vertex 3

		1.2, 1,0, // vertex 3
		-1.2, 1,0, // vertex 4
		-1.2,-1,0  // vertex 1
	};

	static const GLfloat color_buffer_data [] = {
		1,0,0, // color 1
		0,0,1, // color 2
		0,1,0, // color 3

		0,1,0, // color 3
		0.3,0.3,0.3, // color 4
		1,0,0  // color 1
	};

	// Texture coordinates start with (0,0) at top left of the image to (1,1) at bot right
	static const GLfloat texture_buffer_data [] = {
		0,1, // TexCoord 1 - bot left
		1,1, // TexCoord 2 - bot right
		1,0, // TexCoord 3 - top right

		1,0, // TexCoord 3 - top right
		0,0, // TexCoord 4 - top left
		0,1  // TexCoord 1 - bot left
	};

	// create3DTexturedObject creates and returns a handle to a VAO that can be used later
	rectangle = create3DTexturedObject(GL_TRIANGLES, 6, vertex_buffer_data, texture_buffer_data, textureID, GL_FILL);
}
*/
float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
	// clear the color and depth in the frame buffer
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// use the loaded shader program
	// Don't change unless you know what you are doing
	glUseProgram (programID);


	if(zset==1)
	{changex=0;changez=0;}
else if(zset==-1)
	{changex=0;changez=18;}
else if(xset==1)
	{changez=9;changex=9;}
else if(xset==-1)
	{changez=9;changex=-9;}


	// Eye - Location of camera. Don't change unless you are sure!!
	glm::vec3 eye (-250*tower+5*top+(5)*heli+adven*(playerx*0.5)+follow*((playerx+0.5)*0.5), 200*tower+200*top+(2+dragy*0.4)*heli+adven*((playery*0.5))+follow*((playery+1)), 100*tower+5*top+(5+dragy*0.25)*heli+adven*((playerz*0.5)-1)+follow*((playerz)*0.5));
	// Target - Where is the camera looking at.  Don't change unless you are sure!!
	glm::vec3 target (4*tower+5*top+(changex+playerx+0.5)*adven+(changex+playerx+0.5)*follow+(5)*heli, 0*tower+0*top+0.1*adven+0.1*follow+(0)*heli, 5*tower+4*top+((changez+playerz-17.5)*adven)+((changez+playerz-17.5)*follow)+4*heli);
	// Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
	glm::vec3 up (-1*tower+0*top+adven*0+follow*0+(0)*heli, 10*tower+-10*top+adven*1+follow*1+heli*(1), 1*tower+-1500*top+adven*0+follow*0+heli*(0));

	// Compute Camera matrix (view)
	 Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
	//  Don't change unless you are sure!!
	static float c = 0;
	//c++;
	//Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(sinf(c*M_PI/180.0),3*cosf(c*M_PI/180.0),0)); // Fixed camera for 2D (ortho) in XY plane

	// Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
	//  Don't change unless you are sure!!
	glm::mat4 VP = Matrices.projection * Matrices.view;

	// Send our transformation to the currently bound shader, in the "MVP" uniform
	// For each model you render, since the MVP will be different (at least the M part)
	//  Don't change unless you are sure!!
	glm::mat4 MVP;	// MVP = Projection * View * Model

	// Load identity to model matrix
	Matrices.model = glm::mat4(1.0f);

	/* Render your scene */
	glm::mat4 translateTriangle = glm::translate (glm::vec3(-2.0f, 0.0f, 0.0f)); // glTranslatef
	glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
	glm::mat4 triangleTransform = translateTriangle * rotateTriangle;
	Matrices.model *= triangleTransform;
	MVP = VP * Matrices.model; // MVP = p * V * M

	//  Don't change unless you are sure!!
	// Copy MVP to normal shaders
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

	// draw3DObject draws the VAO given to it using current MVP matrix
	//draw3DObject(triangle);


	// Render with texture shaders now
//	glUseProgram(textureProgramID);

	// Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
	// glPopMatrix ();
if(gameover<=1){
for(int i=0;i<10;i++)
{
	for(int j=0;j<10;j++)
	{
		
	//int number=rand()%1000;
	if(cubes[i][j].n!=0&&cubes[i][j].n!=1&&cubes[i][j].n!=2){	
	
	/*Matrices.model = glm::mat4(1.0f);

	glm::mat4 translateRectangle = glm::translate (glm::vec3(j, 0, i));        // glTranslatef
	//glm::mat4 rotateRectangle = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
	Matrices.model *= (translateRectangle);
	MVP = VP * Matrices.model;
	// Copy MVP to texture shaders
//	glUniformMatrix4fv(Matrices.TexMatrixID, 1, GL_FALSE, &MVP[0][0]);

	// Set the texture sampler to access Texture0 memory
//	glUniform1i(glGetUniformLocation(textureProgramID, "texSampler"), 0);
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	// draw3DObject draws the VAO given to it using current MVP matrix
	draw3DObject(rectangle1);*/

	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateCube1 = glm::translate (glm::vec3(j, cubes[i][j].velocity, i));        // glTranslatef
	glm::mat4 rotateCube1 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
	Matrices.model *= (translateCube1 * rotateCube1);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(cube);
	
		}
	else if(cubes[i][j].n==2)
	{
		Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateCube1 = glm::translate (glm::vec3(j, cubes[i][j].velocity, i));        // glTranslatef
	glm::mat4 rotateCube1 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
	Matrices.model *= (translateCube1 * rotateCube1);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(cube);
	}
	}
	
}
}



Matrices.model = glm::mat4(1.0f);
	glm::mat4 scaleCube2 = glm::scale (glm::vec3(0.5, 0.5, 0.5)); 
	glm::mat4 translateCube2 = glm::translate (glm::vec3(playerx, playery, playerz));
	glm::mat4 translateCube3 = glm::translate (glm::vec3(0.5, 1,0.5 ));
	glm::mat4 translateCube3i = glm::translate (glm::vec3(-0.5,-1,-0.5 ));        // glTranslatef
	glm::mat4 rotateCube2 = glm::rotate((float)(angle*M_PI/180.0f), glm::vec3(0,1,0)); // rotate about vector (-1,1,1)
	Matrices.model *= ( scaleCube2 * translateCube2 *translateCube3i * rotateCube2 * translateCube3 );
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(player);


	// Increment angles
	float increments = 1;

	// Render font on screen
	static int fontScale = 0;
	float fontScaleValue = 0.75 + 0.25*sinf(fontScale*M_PI/180.0f);
	glm::vec3 fontColor = getRGBfromHue (fontScale);



	// Use font Shaders for next part of code
	glUseProgram(fontProgramID);
	Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane



/*Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateCube3 = glm::translate (glm::vec3(0, 0,0 ));        // Lives
	glm::mat4 rotateCube3 = glm::rotate((float)(45*M_PI/180.0f), glm::vec3(0,0,1)); 
	Matrices.model *= (translateCube3 * rotateCube3);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(cube3);*/



	// Transform the text
	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateText = glm::translate(glm::vec3(6.25,-4.75,0));
	glm::mat4 scaleText = glm::scale(glm::vec3(0.8,0.8,0.8));
	Matrices.model *= (translateText * scaleText);
	MVP = Matrices.projection * Matrices.view * Matrices.model;
	// send font's MVP and font color to fond shaders
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);
if(gameover<1){
	// Render font
	if(top==1)
	GL3Font.font->Render("TOP VIEW");
	else if(tower==1)
	GL3Font.font->Render("TOWER VIEW");
	else if(adven==1)
	GL3Font.font->Render("ADVENTURER VIEW");
	else if(adven==1)
	GL3Font.font->Render("FOLLOW CAM VIEW");
	else if(heli==1)
	GL3Font.font->Render("HELICOPTER VIEW");
}

Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateText3 = glm::translate(glm::vec3(6.5,3.5,0));
	glm::mat4 scaleText3 = glm::scale(glm::vec3(0.7,0.7,0.7));
	Matrices.model *= (translateText3 * scaleText3);
	MVP = Matrices.projection * Matrices.view * Matrices.model;
	// send font's MVP and font color to fond shaders
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);


	char s[10];
	strcpy(s,"Time: ");
	char buffer[100];
	sprintf(buffer, "%d", timeleft);
	strcat(s, buffer);
	// Render font
	if(gameover==0)
	GL3Font.font->Render(s);



Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateText4 = glm::translate(glm::vec3(-9,4.0,0));
	glm::mat4 scaleText4 = glm::scale(glm::vec3(0.7,0.7,0.7));
	Matrices.model *= (translateText4 * scaleText4);
	MVP = Matrices.projection * Matrices.view * Matrices.model;
	// send font's MVP and font color to fond shaders
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);


	char s1[10];
	strcpy(s1,"LEVEL ");
	char buffer1[100];
	sprintf(buffer1, "%d", level);
	strcat(s1, buffer1);
	// Render font
	if(gameover==0)
	GL3Font.font->Render(s1);



Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateText5 = glm::translate(glm::vec3(6.5,4.0,0));
	glm::mat4 scaleText5 = glm::scale(glm::vec3(0.8,0.8,0.8));
	Matrices.model *= (translateText5 * scaleText5);
	MVP = Matrices.projection * Matrices.view * Matrices.model;
	// send font's MVP and font color to fond shaders
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);


	char s2[100];
	strcpy(s2,"Score: ");
	char buffer2[100];
	sprintf(buffer2, "%lld", score);
	strcat(s2, buffer2);
	// Render font
	if(gameover==0)
	GL3Font.font->Render(s2);


if(gameover>=2){
Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateText6 = glm::translate(glm::vec3(-1.5,0,0));
	glm::mat4 scaleText6 = glm::scale(glm::vec3(0.8,0.8,0.8));
	Matrices.model *= (translateText6 * scaleText6);
	MVP = Matrices.projection * Matrices.view * Matrices.model;
	// send font's MVP and font color to fond shaders
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);
	GL3Font.font->Render("GAME OVER");

if(timeleft<=0){
Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateText6 = glm::translate(glm::vec3(-2.5,-0.5,0));
	glm::mat4 scaleText6 = glm::scale(glm::vec3(0.7,0.7,0.7));
	Matrices.model *= (translateText6 * scaleText6);
	MVP = Matrices.projection * Matrices.view * Matrices.model;
	// send font's MVP and font color to fond shaders
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);
	GL3Font.font->Render("You ran out of time!");
}

else{
Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateText6 = glm::translate(glm::vec3(-2.5,-0.5,0));
	glm::mat4 scaleText6 = glm::scale(glm::vec3(0.7,0.7,0.7));
	Matrices.model *= (translateText6 * scaleText6);
	MVP = Matrices.projection * Matrices.view * Matrices.model;
	// send font's MVP and font color to fond shaders
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);
	GL3Font.font->Render("Sleep with the fishes!");
}

Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateText5 = glm::translate(glm::vec3(-1.5,-1,0));
	glm::mat4 scaleText5 = glm::scale(glm::vec3(0.8,0.8,0.8));
	Matrices.model *= (translateText5 * scaleText5);
	MVP = Matrices.projection * Matrices.view * Matrices.model;
	// send font's MVP and font color to fond shaders
	glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);


	char s3[100];
	strcpy(s3,"Score: ");
	char buffer3[100];
	sprintf(buffer3, "%lld", score);
	strcat(s3, buffer3);
	// Render font
	GL3Font.font->Render(s3);

}
	//camera_rotation_angle++; // Simulating camera rotation
	//triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
	//rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;

	// font size and color changes
	//fontScale = (fontScale + 1) % 360;
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
	GLFWwindow* window; // window desciptor/handle

	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
	glfwSwapInterval( 1 );

	/* --- register callbacks with GLFW --- */

	/* Register function to handle window resizes */
	/* With Retina display on Mac OS X GLFW's FramebufferSize
	 is different from WindowSize */
	glfwSetFramebufferSizeCallback(window, reshapeWindow);
	glfwSetWindowSizeCallback(window, reshapeWindow);

	/* Register function to handle window close */
	glfwSetWindowCloseCallback(window, quit);

	/* Register function to handle keyboard input */
	glfwSetKeyCallback(window, keyboard);      // general keyboard input
	glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

	/* Register function to handle mouse click */
	glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

	return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
	// Load Textures
	// Enable Texture0 as current texture memory
	//glActiveTexture(GL_TEXTURE0);
	// load an image file directly as a new OpenGL texture
	// GLuint texID = SOIL_load_OGL_texture ("beach.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_TEXTURE_REPEATS); // Buggy for OpenGL3
	/*GLuint textureID = createTexture("beach2.png");
	// check for an error during the load process
	if(textureID == 0 )
		cout << "SOIL loading error: '" << SOIL_last_result() << "'" << endl;

	// Create and compile our GLSL program from the texture shaders
	// Get a handle for our "MVP" uniform
	Matrices.TexMatrixID = glGetUniformLocation(textureProgramID, "MVP");


	/* Objects should be created before any other gl function and shaders */
	// Create the models
	createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	createRectangle1 (-1,0,0,0,0,0,0,-2,0,-1,-2,0);
	createCuboid();
	createPlayer();
	createCube();
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL3.vert", "Sample_GL3.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (window, width, height);

	// Background color of the scene
	glClearColor (0.5,0.9,1, 0); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Initialise FTGL stuff
	const char* fontfile = "arial.ttf";
	GL3Font.font = new FTExtrudeFont(fontfile); // 3D extrude style rendering

	if(GL3Font.font->Error())
	{
		cout << "Error: Could not load font `" << fontfile << "'" << endl;
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	// Create and compile our GLSL program from the font shaders
	fontProgramID = LoadShaders( "fontrender.vert", "fontrender.frag" );
	GLint fontVertexCoordAttrib, fontVertexNormalAttrib, fontVertexOffsetUniform;
	fontVertexCoordAttrib = glGetAttribLocation(fontProgramID, "vertexPosition");
	fontVertexNormalAttrib = glGetAttribLocation(fontProgramID, "vertexNormal");
	fontVertexOffsetUniform = glGetUniformLocation(fontProgramID, "pen");
	GL3Font.fontMatrixID = glGetUniformLocation(fontProgramID, "MVP");
	GL3Font.fontColorID = glGetUniformLocation(fontProgramID, "fontColor");

	GL3Font.font->ShaderLocations(fontVertexCoordAttrib, fontVertexNormalAttrib, fontVertexOffsetUniform);
	GL3Font.font->FaceSize(1);
	GL3Font.font->Depth(0);
	GL3Font.font->Outset(0, 0);
	GL3Font.font->CharMap(ft_encoding_unicode);

	cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
	cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
	cout << "VERSION: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

}
float rv=-0.5;

void randomGenerate(int x,float up)
{
	for(int i=0;i<10;i++)
		for(int j=0;j<10;j++)
			{
				int n=rand()%x;
				rv+=0.3;
				if(rv>=up)
				rv=-1*up;
				if((n==0||n==1||n==3)&&((i==0&&j==9)||(i==9&&j==0)))
				n=6;
				if(n==2)
				cubes[i][j].setValues(j,0,i,n,rv,0);
				else
				cubes[i][j].setValues(j,0,i,n,0,0);
			}
/*for(int i=0;i<10;i++)
		{for(int j=0;j<10;j++)
			{
				cout<<a[i][j]<<" ";
			}
	cout<<endl;}*/
}


void fall(int f)
{//cout<<"FALL"<<endl;
	if(f==1)
	{
		if(((playery>cubes[9-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)].velocity*2+2)&&setflag==0&&cubes[9-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)].n==2)||(cubes[9-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)].n==0))
		{playery-=0.2;
		cout<<cubes[9-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)].n<<endl;
		setflag=0;
		}
		/*else if(cubes[9-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)].n!=2)
		{
			if(playery>2.0)
			playery-=0.2;
			else {playery=2.0;cout<<"Here"<<endl;}
			setflag=0;
		}*/
		else if(setflag==1||(playery<cubes[9-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)].velocity*2+2))
		{playery=(cubes[9-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)].velocity)*2+2.0;
		setflag=1;	
		//cout<<"There"<<endl;
		}
	}
}


int high=0;
double yvelocity=1.0;
int main (int argc, char** argv)
{
	
	
	
	randomGenerate(20,upp);
	GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);
	
	double last_update_time = glfwGetTime(), current_time;
	double last_update_time2 = glfwGetTime();
	double last_update_time3 = glfwGetTime();
	/* Draw in loop */
	while (!glfwWindowShouldClose(window)&&gameover<8) {
		//cout<<yvelocity<<endl;
		glfwGetCursorPos(window,&mousex,&mousey);
		mousex=(mousex-1366/2)/85.375;mousey=(mousey-768/2)/96;mousey*=-1;
		// OpenGL Draw commands
		draw();
		glfwSetScrollCallback(window, scroll_callback);
		// Swap Frame Buffer in double buffering
		glfwSwapBuffers(window);

		// Poll for Keyboard and mouse events
		glfwPollEvents();
		if(playerx<-0.5||playerx>17.5||playerz>17.5||playerz<-2.5)
		fallflag=1;
		if((timeleft<=0 || playery<-8)&&gameover==0)
			gameover=1;
		/*fall(fallflag);

		if(cubes[9-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)].n==2)
		{
		if(playery>cubes[9-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)].velocity*2+2.0)
		{
			fallflag=1;
			fall(fallflag);
		}
		}*/
										





	
		if((9-(int)((17.5-playerz)/2))==0&&((int)((playerx+0.5)/2))==9)			//Levelling up
			{score+=level*10*timeleft;
			level++;
			timeleft+=5*level;
			if(rd>10)
			rd-=5;
			else if(rd<=1)
			{rd+=1;vchange+=0.2;}
			else
			rd-=1;
			upp+=0;
			angle=0;zset=1;xset=0;
			randomGenerate(rd,upp);
			playerx=-0.5;playery=2.0;playerz=17.50;}
			

if(cubes[9-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)].n==3)
		{cubes[9-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)].velocity-=0.07;
			//cout<<cubes[9-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)].velocity<<endl;	
}		

					




		// Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
		current_time = glfwGetTime(); // Time in seconds
		if ((current_time - last_update_time) >= 1.0) { // atleast 0.5s elapsed since last frame
			// do something every 0.5 seconds ..
			last_update_time = current_time;
			if(gameover==0)
			timeleft--;
			//cout<<playerx<<" "<<playery<<" "<<playerz<<endl;	
		}

if ((current_time - last_update_time3) >= 0.05) { // atleast 0.5s elapsed since last frame
			// do something every 0.5 seconds ..
			last_update_time3 = current_time;
		if(jump==1)							//Jump
			{
				
				if(high==0&&yvelocity>=0.1)
				{yvelocity-=0.1;
				playery+=yvelocity;
				//cout<<yvelocity<<endl;//cout<<"Here"<<endl;
				}
				else if(yvelocity<0.1)
				{high=1;//cout<<"There"<<endl;
				if(zset==1)
				hitz=-1;
				else if(zset==-1)
				hitz=1;
				else if(xset==1)
				hitx=1;
				else if(xset==-1)
				hitx=-1;
				if(playery<cubes[9+hitz-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)+hitx].velocity*2+2)
					hitflag*=-1;				
				}
				if(high==1)
				{yvelocity+=0.1;}
				if(playery>2.0&&high==1)
				{playery-=yvelocity;}
				else if(high==1&&playery<=2.0)
				{jump=0;yvelocity=1.0;playery=2.0;high=0;
				if(initially>=2.0&&hitflag!=-1){
				if(zset==1)
				playerz=initiallz-4.0;
				else if(zset==-1)
				playerz=initiallz+4.0;
				else if(xset==1)
				playerx=initiallx+4.0;
				else if(xset==-1)
				playerx=initiallx-4.0;
				if(initially-playery>4)
				gameover=1;	}
				else if(hitflag!=-1){
					if(zset==1)
				playerz=initiallz-2.0;
				else if(zset==-1)
				playerz=initiallz+2.0;
				else if(xset==1)
				playerx=initiallx+2.0;
				else if(xset==-1)
				playerx=initiallx-2.0;
				if(initially-playery>4)
				gameover=1;
				}
				}
				if(jump==1&&initially<2.0){	
				if(zset==1)
				playerz-=0.1*hitflag;
				else if(zset==-1)
				playerz+=0.1*hitflag;
				else if(xset==1)
				playerx+=0.1*hitflag;
				else if(xset==-1)
				playerx-=0.1*hitflag;}
				else if(jump==1&&initially>=2.0){	
				if(zset==1)
				playerz-=0.2*hitflag;
				else if(zset==-1)
				playerz+=0.2*hitflag;
				else if(xset==1)
				playerx+=0.2*hitflag;
				else if(xset==-1)
				playerx-=0.2*hitflag;}	
				/*int hitx=0,hitz=0;
				if((((int)((17.5-playerz)/2))<=((17.5-playerz)/2)-0.5))
					hitz=-1;
				else if(((int)((playerx+0.5)/2))>=((playerx+0.5)/2))
					hitx=-1;
				else if(((int)((17.5-playerz)/2))>=((17.5-playerz)/2))
					hitz=1;
				else if(((int)((playerx+0.5)/2))<=((playerx+0.5)/2))
					hitx=1;
				if(playery<cubes[9-(int)((17.5-playerz)/2)+hitz][(int)((playerx+0.5)/2)+hitx].velocity*2+2)
				{
					hitflag=-1;
					high=1;
				}*/
					
			}
}	



if(jump==0){
		if((fallflag==1||((playery>cubes[9-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)].velocity*2+2)&&setflag==0)||(cubes[9-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)].n==0)||(cubes[9-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)].n==1)))
		{playery-=0.2;
		//cout<<cubes[9-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)].n<<endl;
		setflag=0;
		}

		/*else if((fallflag==1||((playery>cubes[9-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)].velocity*2+2)&&setflag==0)||(cubes[9-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)].n==0)||(cubes[9-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)].n==1))&&jump==1)
		{
			
		}*/
		else if(setflag==1||(playery<cubes[9-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)].velocity*2+2))
		{playery=(cubes[9-(int)((17.5-playerz)/2)][(int)((playerx+0.5)/2)].velocity)*2+2.0;
		setflag=1;	
		//cout<<"There"<<endl;
		}
		}




		if ((current_time - last_update_time2) >= 0.5) { // atleast 0.5s elapsed since last frame
			// do something every 0.5 seconds ..
			last_update_time2 = current_time;
			for(int i=0;i<10;i++)
				for(int j=0;j<10;j++)
				{
					
				
			if(cubes[i][j].n==2){
			if(cubes[i][j].velocity<=upp&&cubes[i][j].velocityflag==0)
			{cubes[i][j].velocity+=vchange;
			}
			else if(cubes[i][j].velocity>=-1*upp)
			{cubes[i][j].velocity-=vchange;
			cubes[i][j].velocityflag=1;}
			else
			{cubes[i][j].velocityflag=0;}
			}
			}
			if(gameover>=1)
			gameover+=1;	


				
		}
	}

	glfwTerminate();
	exit(EXIT_SUCCESS);
}
