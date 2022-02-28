#define GLFW_INCLUDE_ES2 1
#define GLFW_DLL 1
//#define GLFW_EXPOSE_NATIVE_WIN32 1
//#define GLFW_EXPOSE_NATIVE_EGL 1

#include <GLES2/gl2.h>
#include <EGL/egl.h>

#include <GLFW/glfw3.h>
//#include <GLFW/glfw3native.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <fstream> 
#include "angle_util/Matrix.h"
#include "angle_util/geometry_utils.h"
#include "bitmap.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define TEXTURE_COUNT 2

GLint GprogramID = -1;
GLuint GtextureID[TEXTURE_COUNT];

GLFWwindow* window;

Matrix4 gPerspectiveMatrix;
Matrix4 gViewMatrix;

static void error_callback(int error, const char* description)
{
  fputs(description, stderr);
}

GLuint LoadShader(GLenum type, const char *shaderSrc )
{
   GLuint shader;
   GLint compiled;
   
   // Create the shader object
   shader = glCreateShader ( type );

   if ( shader == 0 )
   	return 0;

   // Load the shader source
   glShaderSource ( shader, 1, &shaderSrc, NULL );
   
   // Compile the shader
   glCompileShader ( shader );

   // Check the compile status
   glGetShaderiv ( shader, GL_COMPILE_STATUS, &compiled );

   if ( !compiled ) 
   {
      GLint infoLen = 0;

      glGetShaderiv ( shader, GL_INFO_LOG_LENGTH, &infoLen );
      
      if ( infoLen > 1 )
      {
		 char infoLog[4096];
         glGetShaderInfoLog ( shader, infoLen, NULL, infoLog );
         printf ( "Error compiling shader:\n%s\n", infoLog );            
      }

      glDeleteShader ( shader );
      return 0;
   }

   return shader;
}

GLuint LoadShaderFromFile(GLenum shaderType, std::string path)
{
    GLuint shaderID = 0;
    std::string shaderString;
    std::ifstream sourceFile( path.c_str() );

    if( sourceFile )
    {
        shaderString.assign( ( std::istreambuf_iterator< char >( sourceFile ) ), std::istreambuf_iterator< char >() );
        const GLchar* shaderSource = shaderString.c_str();

		return LoadShader(shaderType, shaderSource);
    }
    else
        printf( "Unable to open file %s\n", path.c_str() );

    return shaderID;
}


void loadTexture(const char* path, GLuint textureID)
{
	CBitmap bitmap(path);

	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 

	// bilinear filtering.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bitmap.GetWidth(), bitmap.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap.GetBits());
}

int Init ( void )
{
   GLuint vertexShader;
   GLuint fragmentShader;
   GLuint programObject;
   GLint linked;

   //load textures
   glGenTextures(TEXTURE_COUNT, GtextureID);
   loadTexture("../media/rocks.bmp", GtextureID[0]);
   loadTexture("../media/background.bmp", GtextureID[1]);
   //====

   fragmentShader = LoadShaderFromFile(GL_VERTEX_SHADER, "../vertexShader1.vert" );
   vertexShader = LoadShaderFromFile(GL_FRAGMENT_SHADER, "../fragmentShader1.frag" );

   // Create the program object
   programObject = glCreateProgram ( );
   
   if ( programObject == 0 )
      return 0;

   glAttachShader ( programObject, vertexShader );
   glAttachShader ( programObject, fragmentShader );

   glBindAttribLocation ( programObject, 0, "vPosition" );
   glBindAttribLocation ( programObject, 1, "vColor" );
   glBindAttribLocation ( programObject, 2, "vTexCoord" );

   // Link the program
   glLinkProgram ( programObject );

   // Check the link status
   glGetProgramiv ( programObject, GL_LINK_STATUS, &linked );

   if ( !linked ) 
   {
      GLint infoLen = 0;

      glGetProgramiv ( programObject, GL_INFO_LOG_LENGTH, &infoLen );
      
      if ( infoLen > 1 )
      {
		 char infoLog[1024];
         glGetProgramInfoLog ( programObject, infoLen, NULL, infoLog );
         printf ( "Error linking program:\n%s\n", infoLog );            
      }

      glDeleteProgram ( programObject );
      return 0;
   }

   // Store the program object
   GprogramID = programObject;


   glClearColor ( 0.0f, 0.0f, 0.0f, 0.0f );
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   // Initialize matrices
   gPerspectiveMatrix = Matrix4::perspective(60.0f,
											(float)WINDOW_WIDTH/(float)WINDOW_HEIGHT,
                                             0.5f, 300.0f);

   gViewMatrix = Matrix4::translate(Vector3(0.0f, 0.0f, -2.0f));


   return 1;
}

void UpdateCamera(void)
{
	static float yaw = 0.0f;
	static float pitch = 0.0f;
	static float distance = 3.0f;

	if(glfwGetKey(window, 'A')) pitch -= 1.0f;
	if(glfwGetKey(window, 'D')) pitch += 1.0f;
	if(glfwGetKey(window, 'W')) yaw -= 1.0f;
	if(glfwGetKey(window, 'S')) yaw += 1.0f;

	if(glfwGetKey(window, 'R'))
	{
		distance -= 0.06f;
		if(distance < 1.0f)
			distance = 1.0f;
	}
	if(glfwGetKey(window, 'F')) distance += 0.06f;

	gViewMatrix =	Matrix4::translate(Vector3(0.0f, 0.0f, -distance)) * 
					Matrix4::rotate(yaw, Vector3(1.0f, 0.0f, 0.0f)) *
					Matrix4::rotate(pitch, Vector3(0.0f, 1.0f, 0.0f));
}

void DrawSquare(float originX, float originY, float width, float height)
{
   static GLfloat vVertices[] = {(originX - width / 2.0f),  (originY + height / 2.0f), 0.0f,
								  (originX - width / 2.0f), (originY - height / 2.0f), 0.0f,
								  (originX + width / 2.0f), (originY - height / 2.0f),  0.0f,
								  (originX + width / 2.0f),  (originY - height / 2.0f), 0.0f,
								  (originX + width / 2.0f), (originY + height / 2.0f), 0.0f,
								  (originX - width / 2.0f), (originY + height / 2.0f),  0.0f};
					  
   static GLfloat vColors[] = {1.0f,  0.0f, 0.0f, 1.0f,
								0.0f, 1.0f, 0.0f, 1.0f,
								0.0f, 0.0f,  1.0f, 1.0f,
								0.0f,  0.0f, 1.0f, 1.0f,
								1.0f, 1.0f, 0.0f, 1.0f,
								1.0f, 0.0f,  0.0f, 1.0f};

   static GLfloat vTexCoords[] = {0.0f,  1.0f,
									0.0f, 0.0f,
									1.0f, 0.0f,
									1.0f,  0.0f,
									1.0f, 1.0f,
									0.0f, 1.0f};


   glBindTexture(GL_TEXTURE_2D, GtextureID[0]);




   // Use the program object
   glUseProgram(GprogramID);

   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vVertices);
   glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, vColors);
   glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, vTexCoords);

   glEnableVertexAttribArray(0);
   glEnableVertexAttribArray(1);
   glEnableVertexAttribArray(2);


   glDrawArrays(GL_TRIANGLES, 0, 6);


   glDisableVertexAttribArray(0);
   glDisableVertexAttribArray(1);
   glDisableVertexAttribArray(2);
}

void Draw(void)
{
	// Set the viewport
   glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	UpdateCamera();
	
	// set the sampler2D varying variable to the first texture unit(index 0)
	glUniform1i(glGetUniformLocation(GprogramID, "sampler2d"), 0);

	//modify Factor 1 varying variable
	static float factor1 = 0.0f;
	factor1 += 0.01f;
	GLint factor1Loc = glGetUniformLocation(GprogramID, "Factor1");
	if(factor1Loc != -1)
	{
	   glUniform1f(factor1Loc, factor1);
	}
	//===================


	static float modelRotation = 0.0f;
	modelRotation += 0.5f;
	if(modelRotation > 360.0f)
		modelRotation -= 360.0f;

	Matrix4 modelMatrix, mvpMatrix;
    modelMatrix = Matrix4::translate(Vector3(-1.0, 0.0, 0.0)) * Matrix4::rotate(-modelRotation, Vector3(0.0f, 1.0f, 0.0f));
					
    mvpMatrix = gPerspectiveMatrix * gViewMatrix * modelMatrix;
    glUniformMatrix4fv(glGetUniformLocation(GprogramID, "uMvpMatrix"), 1, GL_FALSE, mvpMatrix.data);
	DrawSquare(0.0, 0.0, 1.0, 1.0);

    Matrix4 modelMatrix1, mvpMatrix1;
    modelMatrix1 = Matrix4::translate(Vector3(1.0, 0.0, 0.0)) * Matrix4::rotate(-modelRotation, Vector3(1.0, 0.0, 0.0));

    mvpMatrix1 = gPerspectiveMatrix * gViewMatrix * modelMatrix1;
    glUniformMatrix4fv(glGetUniformLocation(GprogramID, "uMvpMatrix"), 1, GL_FALSE, mvpMatrix1.data);
    DrawSquare(0.0, 0.0, 1.0, 1.0);
}

int main(void)
{
  glfwSetErrorCallback(error_callback);

  // Initialize GLFW library
  if (!glfwInit())
    return -1;

  glfwDefaultWindowHints();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

  // Create and open a window
  window = glfwCreateWindow(WINDOW_WIDTH,
                            WINDOW_HEIGHT,
                            "Hello World",
                            NULL,
                            NULL);

  if(!window)
  {
    glfwTerminate();
    printf("glfwCreateWindow Error\n");
    exit(1);
  }

  glfwMakeContextCurrent(window);

  Init();

  // Repeat
  while(!glfwWindowShouldClose(window))
  {
	  Draw();
	  glfwSwapBuffers(window);
	  glfwPollEvents();

	  if(glfwGetKey(window, GLFW_KEY_ESCAPE))
			break;
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
