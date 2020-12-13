
/*
 * Final_Project_CS_330.cpp
 *
 *  Created on: Apr 19, 2020
 *      Author: 1434974_snhu
 */


/*Header Inclusions */
#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/glut.h>
#include <GL/gl.h>


// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// SOIL Image loader Inclusion
#include "SOIL2/SOIL2.h"


/////////////////////////////////////////////////////////////

// new addition in header files
// imgui Header Inclusions
#include <imgui/imgui.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imconfig.h>
#include <imgui/imgui_internal.h>
#include <imgui/imstb_rectpack.h>
#include <imgui/imstb_textedit.h>
#include <imgui/imstb_truetype.h>
#include <imgui/imgui_impl_glfw.h>

// glfw Header Inclusions
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <stdlib.h>
#include <GL/glfw3.h>
//#include <GL/glui.h>


/////////////////////////////////////////////////////////////


using namespace std; // Standard namespace

#define WINDOW_TITLE "Modern OpenGL" // Window title Macro

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version "\n" #Source
#endif


/*Variable declarations for shader, window size initialization, buffer and array objects*/
GLint ramShaderProgram, lampShaderProgram, WindowWidth = 800, WindowHeight = 600;
GLuint VBO, RamVAO, LightVAO, texture;

GLfloat cameraSpeed = 0.0005f; // Movement speed per frame

GLchar currentKey; // Will store key pressed
GLint specialKey; // to store special key


/////////////

GLint WIDTH = 1920, HEIGHT = 1080;
int wireframe = 0;
int segments = 0;
int main_window;



////////////

GLfloat lastMouseX = 400, lastMouseY = 300; // Locks mouse cursor at the center of the screen
GLfloat mouseXOffset, mouseYOffset, yaw = 0.0f, pitch = 0.0f; // mouse offset, yaw, and pitch variables
GLfloat sensitivity = 0.005f; // Used for mouse / camera rotation sensitivity
bool mouseDetected = true; // Initially true when mouse movement is detected
bool leftmousebuttonDetected = false; // Initially false when the mouse movement is detected
bool rightmousebuttonDetected = false; // Initially false when the mouse movement is detected
int view_state = 1; // Initially true when the perspective view renders


// Global vector declarations
glm::vec3 cameraPosition = glm::vec3(0.0f, 3.0f, 0.0f); // Initial camera position.
glm::vec3 CameraUpY = glm::vec3(0.0f, 3.0f, 0.0f); // Temporary y unit vector
glm::vec3 CameraForwardZ = glm::vec3(0.0f, 0.0f, 3.0f); // Temporary z unit vector
glm::vec3 front; // Temporary z unit vector for mouse



// Ram and light color
glm::vec3 ramColor(1.0f, 1.0f, 1.0f); // change the color of the object, now it is white before texturing
glm::vec3 lightColor(0.0f, 0.9f, 0.9f); // color of the light, blue - green

// Light position and scale
glm::vec3 lightPosition(0.8f, 1.5f, -0.5f); // location of the light 0.5, 0.5, -3.0 initial
glm::vec3 lightScale(0.1f); // can scale out or scale in the color of the white box that is a supposed light in the scene



/*Function prototypes*/
void UResizeWindow(int, int);
void URenderGraphics(void);
void UCreateShader(void);
void UCreateBuffers(void);
void UGenerateTexture(void);


void UKeyboardPressed(unsigned char key, int x, int y);
void pressSpecialKey(int key, int x, int y);
void UKeyReleased(unsigned char key, int x, int y);
void UMouseClick(int button, int state, int x, int y);
void UMouseMove(int x, int y);
void UMousePressedMove(int x, int y);


/*Vertex Shader Source Code*/
const GLchar * ramVertexShaderSource = GLSL(330,
	layout (location = 0) in vec3 position; // Vertex data from Vertex Attrib Pointer 0
	layout (location = 1) in vec3 normal; // VAP position 1 for normals
	layout (location = 2) in vec3 textureCoordinate;


	out vec2 mobileTextureCoordinate;
	out vec3 Normal; // For outgoing normals to fragment shader
	out vec3 FragmentPos; // For outgoing color/ pixels to fragment shader

	// Global variables for the transform matrices
	uniform mat4 model;
	uniform mat4 view;
	uniform mat4 projection;

void main(){
		gl_Position = projection * view * model * vec4(position, 1.0f); // transforms vertices to clip coordinates
		mobileTextureCoordinate = vec2(textureCoordinate.x, 1.0f - textureCoordinate.y); // flips the texture horizontal
		FragmentPos = vec3(model * vec4(position, 1.0f)); // Gets fragment / pixel position in world space only (exclude view and position)
		Normal = mat3(transpose(inverse(model))) * normal; // get normal vectors in world space only and exclude normal translation properties
	}
);

/* Ram Fragment Shader Source Code*/
const GLchar * ramFragmentShaderSource = GLSL(330,

		in vec2 mobileTextureCoordinate;

		in vec3 Normal; // For incoming normals

		in vec3 FragmentPos; // For incoming fragment position

		out vec4 cubeColor; //For outgoing ram color to the GPU
		out vec4 gpuTexture; // Variable to pass color data to the GPU

		uniform vec3 ramColor;
		uniform sampler2D uTexture; // Useful when working with multiple textures
		uniform vec3 lightColor;
		uniform vec3 lightPos;
		uniform vec3 viewPosition;

	void main(){


		gpuTexture = texture(uTexture, mobileTextureCoordinate);



		/* Phong lighting model calculations to generate ambient, diffuse and specular components*/

		// Calculate Ambient lighting*/
		float ambientStrength = 0.9f; // Set ambient or global lighting strength
		vec3 ambient = ambientStrength * lightColor; // Generate ambient light color

		// Calculate diffuse lighting
		vec3 norm = normalize(Normal); // Normalize vectors to 1 unit
		vec3 lightDirection = normalize(lightPos - FragmentPos); // Calculate distance (light direction) between light source and fragments/ pixels on
		float impact = max(dot(norm, lightDirection), 0.0); // Calculate diffuse impact by generating dot product of normal and light
		vec3 diffuse = impact * lightColor; // Generate diffuse light color

		// Calculate Specular lighting
		float specularIntensity = 30.0f; // Set specular light strength
		float highlightSize = 128.0f; // Set specular highlight size
		vec3 viewDir = normalize(viewPosition - FragmentPos); // Calculate view direction
		vec3 reflectDir = reflect(-lightDirection, norm); // Calculate reflection vector

		// Calculate specular component
		float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
		vec3 specular = specularIntensity * specularComponent * lightColor;

		// Calculate phong result
		vec3 ramColor = texture(uTexture, mobileTextureCoordinate). xyz; // since the object color comprises of textures in three axes and not the real color
		vec3 phong = (ambient + diffuse + specular) * ramColor;
		cubeColor = vec4(phong, 1.0f); // Send lighting results of the object Ram with the color to GPU


	}
);


/* Lamp Shader Source Code*/
const GLchar * lampVertexShaderSource = GLSL(330,

		layout (location = 0) in vec3 position; // VAP position 0 for vertex position data

		// Uniform / Global variables for the transform matrices
		uniform mat4 model;
		uniform mat4 view;
		uniform mat4 projection;

		void main()
		{
		gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices to clip coordinates

		}
);

/* Fragment Shader Source Code*/
const GLchar * lampFragmentShaderSource = GLSL(330,

		out vec4 color; // For outgoing lamp color (smaller cube) to the GPU
		// Uniform / Global variables for the transform matrices

		void main(){
		color = vec4(1.0f); // Set color to white (1.0f, 1.0f, 1.0f) with alpha 1.0
		}
);




/*Main Program*/
int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(WindowWidth, WindowHeight);
	glutCreateWindow(WINDOW_TITLE);

	glutReshapeFunc(UResizeWindow);

	glewExperimental = GL_TRUE;
			if (glewInit() != GLEW_OK)
			{
				std::cout << "Failed to initialize GLEW" << std::endl;
				return -1;
			}

	UCreateShader();

	UCreateBuffers();

	UGenerateTexture();




	/////////////////////////////////////PLANNED ENHANCEMENT///////////////////////////////////////


	//glfwMakeContextCurrent(window);
	if (!glfwInit()) {exit(EXIT_FAILURE);}

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	// Setup platform/ Renderer bindings

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL", NULL, NULL);

	// Initializing
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 130");
	// Setup Dear ImGui style
	ImGui::StyleColorsDark();


	// loop until the user closes the window
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		glClearColor(0.45f, 0.55f, 0.6f, 1.00f);
		glClear(GL_COLOR_BUFFER_BIT);

		//feed inputs to dear imgui, start new frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui::NewFrame();

		// rendering a simple GUI that offers control to users (over the ram head)
		ImGui::Begin("Ram Head Rotation/ Position");
		ImGui::Text("Hello, press the ALT key and left mouse button to rotate the ram head horizontally"
				"and ALT key and right mouse button to zoom in and out.");

		const float PI = 3.1415927;
		static float rotation = 0.0;
		ImGui::SliderFloat("Rotation", &rotation, 0, 2 * PI);

		static float translation[] = {0.0, 0.0};
		ImGui::SliderFloat2("Position", translation, -1.0, 1.0);
		ImGui::End();

		// Render dear ImGui onto the screen
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glfwSwapBuffers(window);

	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui::DestroyContext();



	/*

	// Trying another type of GUI, but did not really work

	GLUI *glui = GLUI_Master.create_glui("GLUI");

	glui->set_main_gfx_window(main_window);

	// Creating components
	glui->add_checkbox("Wireframe", &wireframe);
	GLUI_Spinner *segment_spinner =
		glui->add_spinner("Segments:", GLUI_SPINNER_INT, &segments);
	segment_spinner->set_int_limits(3, 60);

	*/


	/////////////////////////////////////END OF PLANNED ENHANCEMENT///////////////////////////////////////




	glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Set background color

	glutDisplayFunc(URenderGraphics);

	glutKeyboardFunc(UKeyboardPressed); // detects key press

	glutSpecialFunc(pressSpecialKey); // detects key press for changing views

	glutKeyboardUpFunc(UKeyReleased); // Detects key release

	glutMouseFunc(UMouseClick); // Detects mouse click

	glutPassiveMotionFunc(UMouseMove); // Detects mouse movement

	glutMotionFunc(UMousePressedMove); // Detects mouse press and movement

	glutMainLoop();

	// Destroys Buffer objects once used
	glDeleteVertexArrays(1, &RamVAO);
	glDeleteVertexArrays(1, &LightVAO);
	glDeleteBuffers(1, &VBO);


	return 0;
}


/*Resizes the window*/
void UResizeWindow(int w, int h)
{
	WindowWidth = w;
	WindowHeight = h;
	glViewport(0, 0, WindowWidth, WindowHeight);
}

/*Renders graphics */
void URenderGraphics(void)
{
	glEnable(GL_DEPTH_TEST); // Enable z-depth

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clears the screen

	GLint modelLoc, viewLoc, projLoc, ramColorLoc, lightColorLoc, lightPositionLoc, viewPositionLoc;

	// Use the Ram Shader and activate the Ram Vertex Array Object for rendering and transforming
	glUseProgram(ramShaderProgram);
	glBindVertexArray(RamVAO); // Activate the Vertex Array Object before rendering and transforming them


	// camera movement logic for zooming in and out
	if((rightmousebuttonDetected) && (mouseYOffset > 0))        // positive mouseYOffset means mouse is moving up
		cameraPosition += cameraSpeed * CameraForwardZ;  // Zooming out

	if((rightmousebuttonDetected) && (mouseYOffset < 0))        // negative mouseYOffset means mouse is moving down
		cameraPosition -= cameraSpeed * CameraForwardZ;  // Zooming in

	CameraForwardZ = front;


	// Transforms the object
	glm::mat4 model;
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // Place the object at the center of the viewport
	model = glm::rotate(model, 10.0f, glm::vec3(0.3f, 3.3f, 3.3f)); // Rotate the object 10 degrees on the xyz axes
	model = glm::scale(model, glm::vec3(2.9f, 2.9f, 2.9f)); // Increase the object size by a scale of 2

	// Transforms the camera
	glm::mat4 view;
	//view = glm::lookAt(CameraForwardZ, cameraPosition, CameraUpY); // this navigates vertically and horizontally but does not zoom in or out
	//view = glm::lookAt(cameraPosition - CameraForwardZ, cameraPosition, CameraUpY); // this starts the camera from left in the screen
	view = glm::lookAt(cameraPosition + CameraForwardZ, cameraPosition, CameraUpY); // this starts the camera from right in the screen



	glm::mat4 projection;
	// Creates a perspective projection
	if(view_state == 1){
	projection = glm::perspective(45.0f, (GLfloat)WindowWidth / (GLfloat)WindowHeight, 0.1f, 100.0f);
	}
	if (view_state == 0){
	// Creates an orthographic view
	projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);
	}




	//Reference matrix uniforms from the Ram Shader program
	modelLoc = glGetUniformLocation(ramShaderProgram, "model");
	viewLoc = glGetUniformLocation(ramShaderProgram, "view");
	projLoc = glGetUniformLocation(ramShaderProgram, "projection");

	// Pass matrix data to the Ram Shader program's matrix uniforms
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	/*--------------------------------------------------------*/

	// Reference matrix uniforms from the Ram Shader program for the ram texture, light color, light position, and camera position
	ramColorLoc = glGetUniformLocation(ramShaderProgram, "ramColor");
	lightColorLoc = glGetUniformLocation(ramShaderProgram, "lightColor");
	lightPositionLoc = glGetUniformLocation(ramShaderProgram, "lightPos");
	viewPositionLoc = glGetUniformLocation(ramShaderProgram, "viewPosition");

	// Pass color, light, and camera data to the Ram Shader program's corresponding uniforms
	glUniform3f(ramColorLoc, ramColor.r, ramColor.g, ramColor.b);
	glUniform3f(lightColorLoc, lightColor.r, lightColor.g, lightColor.b);
	glUniform3f(lightPositionLoc, lightPosition.x, lightPosition.y, lightPosition.z);
	glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);


	//Draws the triangles at least 1000 in numbers
	glDrawArrays(GL_TRIANGLES, 0, 858);
	glBindVertexArray(0); // Deactivate the Vertex Array Object



	/****** Use the Lamp Shader and activate the Lamp Vertex Array Object for rendering and transforming ****/
	glUseProgram(lampShaderProgram);
	glBindVertexArray(LightVAO);

	// Transform the smaller ram used as a visual cue for the light source
	model = glm::translate(model, lightPosition);
	model = glm::scale(model, lightScale);

	//Reference matrix uniforms from the Lamp Shader program
	modelLoc = glGetUniformLocation(lampShaderProgram, "model");
	viewLoc = glGetUniformLocation(lampShaderProgram, "view");
	projLoc = glGetUniformLocation(lampShaderProgram, "projection");

	// Pass matrix data to the Lamp Shader program's matrix uniforms
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));


	glBindTexture(GL_TEXTURE_2D, texture);


	glDrawArrays(GL_TRIANGLES, 0, 858);


	glutPostRedisplay(); // Marks the current window to be redisplayed
	glutSwapBuffers(); // Flips the back buffer with the front buffer every frame. Similar to GL Flush

}

/*Creates the Shader program*/
void UCreateShader()
{
	// Ram Vertex shader
	GLint ramVertexShader = glCreateShader(GL_VERTEX_SHADER); // Creates the Vertex shader
	glShaderSource(ramVertexShader, 1, &ramVertexShaderSource, NULL); // Attaches the Vertex shader to the osurce code
	glCompileShader(ramVertexShader); // Compiles the Vertex shader

	// Ram Fragment shader
	GLint ramFragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // Creates the Fragment shader
	glShaderSource(ramFragmentShader, 1, &ramFragmentShaderSource, NULL); // Attaches the Fragment shader to the source code
	glCompileShader(ramFragmentShader); // Compiles the Fragment shader

	// Ram Shader program
	ramShaderProgram = glCreateProgram(); // Creates the Shader program and returns an id
	glAttachShader(ramShaderProgram, ramVertexShader); // Attach Vertex shader to the Shader program
	glAttachShader(ramShaderProgram, ramFragmentShader);; // Attach Fragment shader to the Shader program
	glLinkProgram(ramShaderProgram); // Link Vertex and Fragment shaders to Shader program

	// Delete the Vertex and Fragment shaders once linked
	glDeleteShader(ramVertexShader);
	glDeleteShader(ramFragmentShader);



	// Lamp Vertex shader
	GLint lampVertexShader = glCreateShader(GL_VERTEX_SHADER); // Creates the Vertex shader
	glShaderSource(lampVertexShader, 1, &lampVertexShaderSource, NULL); // Attaches the Vertex shader to the osurce code
	glCompileShader(lampVertexShader); // Compiles the Vertex shader

	// Lamp Fragment shader
	GLint lampFragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // Creates the Fragment shader
	glShaderSource(lampFragmentShader, 1, &lampFragmentShaderSource, NULL); // Attaches the Fragment shader to the source code
	glCompileShader(lampFragmentShader); // Compiles the Fragment shader

	// Lamp Shader program
	lampShaderProgram = glCreateProgram(); // Creates the Shader program and returns an id
	glAttachShader(lampShaderProgram, lampVertexShader); // Attach Vertex shader to the Shader program
	glAttachShader(lampShaderProgram, lampFragmentShader);; // Attach Fragment shader to the Shader program
	glLinkProgram(lampShaderProgram); // Link Vertex and Fragment shaders to Shader program

	// Delete the lamp shaders once linked
	glDeleteShader(lampVertexShader);
	glDeleteShader(lampFragmentShader);


}

/*Creates the Buffer and Array Objects*/
void UCreateBuffers()
{

	// Position and color data
		GLfloat vertices[] = {
				// left portion neck bottom		// Texture Coordinates			// Normals on the axis
								 1.20f, 0.85f, 0.25f,		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.20f, 0.64f, 0.0f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.11f, 0.63f, 0.11f,		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.11f, 0.63f, 0.11f,		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.13f, 0.77f, 0.29f,		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.20f, 0.85f, 0.25f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 // right portion neck bottom
								 1.20f, 0.64f, 0.0f, 		0.0f, 0.0f,	   -1.0f, 0.0f, 0.0f,
								 1.20f, 0.85f, 0.25f,		1.0f, 0.0f,	   -1.0f, 0.0f, 0.0f,
								 1.29f, 0.63f, 0.11f,		1.0f, 1.0f,	   -1.0f, 0.0f, 0.0f,
								 1.29f, 0.63f, 0.11f,		1.0f, 1.0f,	   -1.0f, 0.0f, 0.0f,
								 1.27f, 0.77f, 0.29f,		0.0f, 1.0f,	   -1.0f, 0.0f, 0.0f,
								 1.20f, 0.85f, 0.25f, 		0.0f, 0.0f,	   -1.0f, 0.0f, 0.0f,

								 //////////////////////


								 // left portion neck bottom
								 1.13f, 0.77f, 0.29f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.20f, 0.85f, 0.25f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.20f, 0.96f, 0.48f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.20f, 0.96f, 0.48f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.14f, 0.87f, 0.47f,		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.13f, 0.77f, 0.29f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 // right portion neck bottom
								 1.27f, 0.77f, 0.29f, 		0.0f, 0.0f,	   -1.0f, 0.0f, 0.0f,
								 1.20f, 0.85f, 0.25f, 		1.0f, 0.0f,	   -1.0f, 0.0f, 0.0f,
								 1.20f, 0.96f, 0.48f, 		1.0f, 1.0f,	   -1.0f, 0.0f, 0.0f,
								 1.20f, 0.96f, 0.48f, 		1.0f, 1.0f,	   -1.0f, 0.0f, 0.0f,
								 1.26f, 0.87f, 0.47f,		0.0f, 1.0f,    -1.0f, 0.0f, 0.0f,
								 1.27f, 0.77f, 0.29f, 		0.0f, 0.0f,    -1.0f, 0.0f, 0.0f,

								 //////////////////////


								 // left portion neck mid section
								 1.14f, 0.87f, 0.47f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.20f, 0.96f, 0.48f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.20f, 1.07f, 0.66f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.20f, 1.07f, 0.66f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.14f, 0.96f, 0.67f,		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.14f, 0.87f, 0.47f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 // right portion neck mid section
								 1.26f, 0.87f, 0.47f, 		0.0f, 0.0f,	   -1.0f, 0.0f, 0.0f,
								 1.20f, 0.96f, 0.48f, 		1.0f, 0.0f,	   -1.0f, 0.0f, 0.0f,
								 1.20f, 1.07f, 0.66f, 		1.0f, 1.0f,	   -1.0f, 0.0f, 0.0f,
								 1.20f, 1.07f, 0.66f, 		1.0f, 1.0f,    -1.0f, 0.0f, 0.0f,
								 1.26f, 0.96f, 0.67f,		0.0f, 1.0f,	   -1.0f, 0.0f, 0.0f,
								 1.26f, 0.87f, 0.47f, 		0.0f, 0.0f,	   -1.0f, 0.0f, 0.0f,

								 //////////////////////

								 // left portion neck top
								 1.07f, 0.59f, 0.27f,		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.11f, 0.63f, 0.11f,		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.13f, 0.77f, 0.29f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.13f, 0.77f, 0.29f,		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.14f, 0.87f, 0.47f,		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.07f, 0.59f, 0.27f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 // right portion neck top
								 1.33f, 0.59f, 0.27f,		0.0f, 0.0f,	   -1.0f, 0.0f, 0.0f,
								 1.29f, 0.63f, 0.11f,		1.0f, 0.0f,	   -1.0f, 0.0f, 0.0f,
								 1.27f, 0.77f, 0.29f, 		1.0f, 1.0f,	   -1.0f, 0.0f, 0.0f,
								 1.27f, 0.77f, 0.29f,		1.0f, 1.0f,	   -1.0f, 0.0f, 0.0f,
								 1.26f, 0.87f, 0.47f,		0.0f, 1.0f,    -1.0f, 0.0f, 0.0f,
								 1.33f, 0.59f, 0.27f, 		0.0f, 0.0f,	   -1.0f, 0.0f, 0.0f,

								 //////////////////////

								 // left neck
								 1.07f, 0.59f, 0.27f,		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.14f, 0.87f, 0.47f,		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.09f, 0.76f, 0.60f,		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.09f, 0.76f, 0.60f,		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.04f, 0.57f, 0.55f,		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.07f, 0.59f, 0.27f,		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 // right neck
								 1.33f, 0.59f, 0.27f,		0.0f, 0.0f,	   -1.0f, 0.0f, 0.0f,
								 1.26f, 0.87f, 0.47f,		1.0f, 0.0f,    -1.0f, 0.0f, 0.0f,
								 1.31f, 0.76f, 0.60f,		1.0f, 1.0f,	   -1.0f, 0.0f, 0.0f,
								 1.31f, 0.76f, 0.60f,		1.0f, 1.0f,	   -1.0f, 0.0f, 0.0f,
								 1.36f, 0.57f, 0.55f,		0.0f, 1.0f,	   -1.0f, 0.0f, 0.0f,
								 1.33f, 0.59f, 0.27f,		0.0f, 0.0f,	   -1.0f, 0.0f, 0.0f,

								 //////////////////////


								 // left neck
								 1.14f, 0.87f, 0.47f,		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.14f, 0.96f, 0.67f,		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.09f, 0.76f, 0.60f,		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.09f, 0.76f, 0.60f,		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.14f, 0.96f, 0.67f,		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.03f, 0.87f, 0.75f,		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 // right neck
								 1.26f, 0.87f, 0.47f,		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.26f, 0.96f, 0.67f,		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.31f, 0.76f, 0.60f,		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.31f, 0.76f, 0.60f,		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.26f, 0.96f, 0.67f,		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.37f, 0.87f, 0.75f,		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,

								 //////////////////////

								 // left cheek
								 1.14f, 0.96f, 0.67f,		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.20f, 1.07f, 0.66f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.06f, 1.09f, 0.71f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.06f, 1.09f, 0.71f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.03f, 0.87f, 0.75f,		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.14f, 0.96f, 0.67f,		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 // right cheek
								 1.26f, 0.96f, 0.67f,		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.20f, 1.07f, 0.66f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.34f, 1.09f, 0.71f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.34f, 1.09f, 0.71f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.37f, 0.87f, 0.75f,		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.26f, 0.96f, 0.67f,		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,

								 //////////////////////

								 1.04f, 0.57f, 0.55f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.09f, 0.76f, 0.60f,		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.02f, 0.70f, 0.81f,		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.02f, 0.70f, 0.81f,		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.02f, 0.59f, 0.76f,		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.04f, 0.57f, 0.55f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,


								 1.36f, 0.57f, 0.55f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.31f, 0.76f, 0.60f,		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.38f, 0.70f, 0.81f,		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.38f, 0.70f, 0.81f,		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.38f, 0.59f, 0.76f,		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.36f, 0.57f, 0.55f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,

								 //////////////////////
								 1.10f, 1.26f, 0.65f,		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.14f, 0.96f, 0.67f,		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.06f, 1.09f, 0.71f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.06f, 1.09f, 0.71f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.07f, 1.29f, 0.70f,		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.10f, 1.26f, 0.65f,		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.30f, 1.26f, 0.65f,		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.26f, 0.96f, 0.67f,		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.34f, 1.09f, 0.71f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.34f, 1.09f, 0.71f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.33f, 1.29f, 0.70f,		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.30f, 1.26f, 0.65f,		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,



								 //////////////////////

				                 // head part
								 1.07f, 1.16f, 1.13f,		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.20f, 1.20f, 1.07f,		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.20f, 1.10f, 1.17f,		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.20f, 1.10f, 1.17f,		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.07f, 1.16f, 1.13f,		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.07f, 1.16f, 1.13f,		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.33f, 1.16f, 1.13f,		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.20f, 1.20f, 1.07f,		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.20f, 1.10f, 1.17f,		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.20f, 1.10f, 1.17f,		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.33f, 1.16f, 1.13f,		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.33f, 1.16f, 1.13f,		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,

								 //////////////////////
								 1.06f, 1.09f, 0.71f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.04f, 1.13f, 0.87f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.07f, 1.34f, 0.75f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.07f, 1.34f, 0.75f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.07f, 1.29f, 0.70f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.06f, 1.09f, 0.71f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.34f, 1.09f, 0.71f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.36f, 1.13f, 0.87f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.33f, 1.34f, 0.75f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.33f, 1.34f, 0.75f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.33f, 1.29f, 0.70f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.34f, 1.09f, 0.71f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 //liliac check
								 1.03f, 0.87f, 0.75f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.09f, 0.76f, 0.60f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.02f, 0.70f, 0.81f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.02f, 0.70f, 0.81f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.97f, 0.89f, 0.97f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.03f, 0.87f, 0.75f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.37f, 0.87f, 0.75f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.31f, 0.76f, 0.60f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.38f, 0.70f, 0.81f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.38f, 0.70f, 0.81f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.43f, 0.89f, 0.97f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.37f, 0.87f, 0.75f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////


								 1.03f, 0.87f, 0.75f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.06f, 1.09f, 0.71f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.01f, 1.01f, 0.89f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.01f, 1.01f, 0.89f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.97f, 0.89f, 0.97f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.03f, 0.87f, 0.75f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.37f, 0.87f, 0.75f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.34f, 1.09f, 0.71f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.39f, 1.01f, 0.89f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.39f, 1.01f, 0.89f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.43f, 0.89f, 0.97f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.37f, 0.87f, 0.75f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 1.01f, 1.01f, 0.89f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.06f, 1.09f, 0.71f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.04f, 1.13f, 0.87f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.04f, 1.13f, 0.87f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.97f, 1.01f, 0.98f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.01f, 1.01f, 0.89f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.39f, 1.01f, 0.89f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.34f, 1.09f, 0.71f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.36f, 1.13f, 0.87f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.36f, 1.13f, 0.87f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.43f, 1.01f, 0.98f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.39f, 1.01f, 0.89f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 0.97f, 0.89f, 0.97f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.01f, 1.01f, 0.89f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.97f, 1.01f, 0.98f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.97f, 1.01f, 0.98f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.97f, 0.90f, 1.02f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.97f, 0.89f, 0.97f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.43f, 0.89f, 0.97f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.39f, 1.01f, 0.89f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.43f, 1.01f, 0.98f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.43f, 1.01f, 0.98f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.43f, 0.90f, 1.02f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.43f, 0.89f, 0.97f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 0.97f, 0.90f, 1.02f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.97f, 1.01f, 0.98f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.95f, 1.11f, 1.05f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.95f, 1.11f, 1.05f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.96f, 0.97f, 1.11f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.97f, 0.90f, 1.02f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.43f, 0.90f, 1.02f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.43f, 1.01f, 0.98f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.45f, 1.11f, 1.05f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.45f, 1.11f, 1.05f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.44f, 0.97f, 1.11f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.43f, 0.90f, 1.02f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 // under eye
								 0.97f, 1.01f, 0.98f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.01f, 1.12f, 0.95f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.04f, 1.13f, 0.87f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.04f, 1.13f, 0.87f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.09f, 1.23f, 0.95f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.01f, 1.12f, 0.95f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.43f, 1.01f, 0.98f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.39f, 1.12f, 0.95f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.36f, 1.13f, 0.87f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.36f, 1.13f, 0.87f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.31f, 1.23f, 0.95f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.39f, 1.12f, 0.95f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,



								 //////////////////////

								 1.04f, 1.13f, 0.87f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.07f, 1.34f, 0.75f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.10f, 1.35f, 0.84f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.10f, 1.35f, 0.84f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.09f, 1.23f, 0.95f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.04f, 1.13f, 0.87f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.36f, 1.13f, 0.87f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.33f, 1.34f, 0.75f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.30f, 1.35f, 0.84f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.30f, 1.35f, 0.84f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.31f, 1.23f, 0.95f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.36f, 1.13f, 0.87f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 1.09f, 1.23f, 0.95f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.10f, 1.35f, 0.84f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.20f, 1.36f, 0.91f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.20f, 1.36f, 0.91f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.20f, 1.26f, 0.99f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.09f, 1.23f, 0.95f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.31f, 1.23f, 0.95f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.30f, 1.35f, 0.84f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.20f, 1.36f, 0.91f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.20f, 1.36f, 0.91f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.20f, 1.26f, 0.99f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.31f, 1.23f, 0.95f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 1.04f, 1.17f, 1.03f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.09f, 1.23f, 0.95f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.20f, 1.26f, 0.99f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.20f, 1.26f, 0.99f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.20f, 1.20f, 1.07f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.04f, 1.17f, 1.03f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.36f, 1.17f, 1.03f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.31f, 1.23f, 0.95f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.20f, 1.26f, 0.99f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.20f, 1.26f, 0.99f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.20f, 1.20f, 1.07f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.36f, 1.17f, 1.03f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,

								 //////////////////////
								 1.04f, 1.17f, 1.03f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.20f, 1.20f, 1.07f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.07f, 1.05f, 1.13f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.07f, 1.05f, 1.13f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.96f, 0.97f, 1.11f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.04f, 1.17f, 1.03f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.36f, 1.17f, 1.03f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.20f, 1.20f, 1.07f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.33f, 1.05f, 1.13f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.33f, 1.05f, 1.13f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.44f, 0.97f, 1.11f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.36f, 1.17f, 1.03f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 1.02f, 0.59f, 0.76f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.02f, 0.70f, 0.81f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.95f, 0.70f, 0.97f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.95f, 0.70f, 0.97f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.01f, 0.64f, 0.93f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.02f, 0.59f, 0.76f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.38f, 0.59f, 0.76f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.38f, 0.70f, 0.81f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.45f, 0.70f, 0.97f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.45f, 0.70f, 0.97f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.39f, 0.64f, 0.93f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.38f, 0.59f, 0.76f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 // side of ear orange color
								 1.02f, 0.70f, 0.81f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.97f, 0.89f, 0.97f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.97f, 0.80f, 0.98f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.97f, 0.80f, 0.98f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.95f, 0.70f, 0.97f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.02f, 0.70f, 0.81f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.38f, 0.70f, 0.81f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.43f, 0.89f, 0.97f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.43f, 0.80f, 0.98f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.43f, 0.80f, 0.98f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.45f, 0.70f, 0.97f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.38f, 0.70f, 0.81f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,

								 //////////////////////
								 // the other side of ear
								 0.96f, 0.97f, 1.11f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.97f, 0.90f, 1.02f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.95f, 0.77f, 1.07f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.95f, 0.77f, 1.07f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.99f, 0.88f, 1.15f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.96f, 0.97f, 1.11f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.44f, 0.97f, 1.11f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.43f, 0.90f, 1.02f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.45f, 0.77f, 1.07f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.45f, 0.77f, 1.07f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.41f, 0.88f, 1.15f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.44f, 0.97f, 1.11f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,

								 //////////////////////
								 //base of  the horn, green
								 0.99f, 0.88f, 1.15f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.96f, 0.97f, 1.11f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.07f, 1.05f, 1.13f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.07f, 1.05f, 1.13f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.02f, 0.94f, 1.18f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.99f, 0.88f, 1.15f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.41f, 0.88f, 1.15f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.44f, 0.97f, 1.11f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.33f, 1.05f, 1.13f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.33f, 1.05f, 1.13f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.38f, 0.94f, 1.18f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.41f, 0.88f, 1.15f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,

								 //////////////////////
								 // behind ear, three triangles
								 0.95f, 0.70f, 0.97f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.97f, 0.80f, 0.98f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.95f, 0.84f, 1.04f, 		0.5f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.95f, 0.84f, 1.04f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.95f, 0.77f, 1.07f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.95f, 0.70f, 0.97f, 		0.5f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.95f, 0.70f, 0.97f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.01f, 0.64f, 0.93f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.95f, 0.77f, 1.07f, 		0.5f, 1.0f,		1.0f, 0.0f, 0.0f,

								 1.45f, 0.70f, 0.97f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.43f, 0.80f, 0.98f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.45f, 0.84f, 1.04f, 		0.5f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.45f, 0.84f, 1.04f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.45f, 0.77f, 1.07f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.45f, 0.70f, 0.97f, 		0.5f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.45f, 0.70f, 0.97f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.39f, 0.64f, 0.93f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.45f, 0.77f, 1.07f, 		0.5f, 1.0f,		-1.0f, 0.0f, 0.0f,



								 //////////////////////
								 // ear green
								 0.97f, 0.90f, 1.02f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.88f, 0.83f, 1.10f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.92f, 0.86f, 0.99f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.92f, 0.86f, 0.99f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.97f, 0.89f, 0.97f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.97f, 0.90f, 1.02f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.43f, 0.90f, 1.02f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.52f, 0.83f, 1.10f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.48f, 0.86f, 0.99f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.48f, 0.86f, 0.99f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.43f, 0.89f, 0.97f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.43f, 0.90f, 1.02f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,

								 //////////////////////
								 0.97f, 0.80f, 0.98f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.97f, 0.89f, 0.97f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.92f, 0.86f, 0.99f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.92f, 0.86f, 0.99f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.86f, 0.73f, 1.00f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.97f, 0.80f, 0.98f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.43f, 0.80f, 0.98f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.43f, 0.89f, 0.97f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.48f, 0.86f, 0.99f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.48f, 0.86f, 0.99f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.54f, 0.73f, 1.00f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.43f, 0.80f, 0.98f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 0.88f, 0.83f, 1.10f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.97f, 0.90f, 1.02f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.95f, 0.84f, 1.04f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.95f, 0.84f, 1.04f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.84f, 0.67f, 1.09f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.88f, 0.83f, 1.10f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.52f, 0.83f, 1.10f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.43f, 0.90f, 1.02f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.45f, 0.84f, 1.04f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.45f, 0.84f, 1.04f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.56f, 0.67f, 1.09f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.52f, 0.83f, 1.10f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 0.84f, 0.67f, 1.09f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.95f, 0.84f, 1.04f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.97f, 0.80f, 0.98f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.97f, 0.80f, 0.98f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.86f, 0.73f, 1.00f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.84f, 0.67f, 1.09f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.56f, 0.67f, 1.09f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.45f, 0.84f, 1.04f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.43f, 0.80f, 0.98f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.43f, 0.80f, 0.98f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.54f, 0.73f, 1.00f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.56f, 0.67f, 1.09f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,

								 //////////////////////


								 // head skull in between part
								 1.16f, 0.99f, 1.25f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.20f, 0.99f, 1.25f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.20f, 1.14f, 1.17f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.20f, 1.14f, 1.17f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.07f, 1.05f, 1.13f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.16f, 0.99f, 1.25f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.24f, 0.99f, 1.25f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.20f, 0.99f, 1.25f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.20f, 1.14f, 1.17f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.20f, 1.14f, 1.17f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.33f, 1.05f, 1.13f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.24f, 0.99f, 1.25f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 // middle temple
								 1.20f, 1.14f, 1.17f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.07f, 1.05f, 1.13f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.20f, 1.20f, 1.07f, 		0.5f, 1.0f,		1.0f, 0.0f, 0.0f,

								 1.20f, 1.14f, 1.17f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.33f, 1.05f, 1.13f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.20f, 1.20f, 1.07f, 		0.5f, 1.0f,		-1.0f, 0.0f, 0.0f,


								 //////////////////////

								 // horn starting yellow
								 1.16f, 0.99f, 1.25f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.00f, 0.94f, 1.37f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.94f, 1.04f, 1.34f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.94f, 1.04f, 1.34f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.07f, 1.05f, 1.13f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.16f, 0.99f, 1.25f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.24f, 0.99f, 1.25f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.40f, 0.94f, 1.37f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.46f, 1.04f, 1.34f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.46f, 1.04f, 1.34f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.33f, 1.05f, 1.13f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.24f, 0.99f, 1.25f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,

								 //////////////////////

								 // horn red
								 0.94f, 1.04f, 1.34f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.07f, 1.05f, 1.13f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.02f, 0.94f, 1.18f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.02f, 0.94f, 1.18f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.86f, 0.71f, 1.18f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.94f, 1.04f, 1.34f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.46f, 1.04f, 1.34f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.33f, 1.05f, 1.13f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.38f, 0.94f, 1.18f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.38f, 0.94f, 1.18f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.54f, 0.71f, 1.18f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.46f, 1.04f, 1.34f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 // small triangle below
								 1.02f, 0.94f, 1.18f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.96f, 0.88f, 1.15f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.86f, 0.71f, 1.18f, 		0.5f, 1.0f,		1.0f, 0.0f, 0.0f,

								 1.38f, 0.94f, 1.18f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.44f, 0.88f, 1.15f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.54f, 0.71f, 1.18f, 		0.5f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////


								 // horn 2nd segment blue
								 1.00f, 0.94f, 1.37f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.85f, 0.66f, 1.43f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.76f, 0.66f, 1.40f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.76f, 0.66f, 1.40f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.94f, 1.04f, 1.34f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.00f, 0.94f, 1.37f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.40f, 0.94f, 1.37f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.55f, 0.66f, 1.43f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.64f, 0.66f, 1.40f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.64f, 0.66f, 1.40f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.46f, 1.04f, 1.34f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.40f, 0.94f, 1.37f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 0.94f, 1.04f, 1.34f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.76f, 0.66f, 1.40f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.72f, 0.71f, 1.18f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.72f, 0.71f, 1.18f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.86f, 0.71f, 1.18f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.94f, 1.04f, 1.34f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.46f, 1.04f, 1.34f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.64f, 0.66f, 1.40f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.68f, 0.71f, 1.18f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.68f, 0.71f, 1.18f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.54f, 0.71f, 1.18f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.46f, 1.04f, 1.34f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 // brown
								 0.76f, 0.66f, 1.40f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.62f, 0.46f, 1.20f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.68f, 0.62f, 1.08f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.68f, 0.62f, 1.08f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.72f, 0.71f, 1.18f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.76f, 0.66f, 1.40f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.64f, 0.66f, 1.40f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.78f, 0.46f, 1.20f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.72f, 0.62f, 1.08f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.72f, 0.62f, 1.08f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.68f, 0.71f, 1.18f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.64f, 0.66f, 1.40f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,

								 //////////////////////
								 // baby blue
								 0.62f, 0.46f, 1.20f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.57f, 0.41f, 0.94f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.66f, 0.60f, 0.96f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.66f, 0.60f, 0.96f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.68f, 0.62f, 1.08f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.62f, 0.46f, 1.20f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.78f, 0.46f, 1.20f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.83f, 0.41f, 0.94f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.74f, 0.60f, 0.96f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.74f, 0.60f, 0.96f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.72f, 0.62f, 1.08f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.78f, 0.46f, 1.20f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 // orange
								 0.57f, 0.41f, 0.94f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.66f, 0.60f, 0.96f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.64f, 0.62f, 0.86f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.64f, 0.62f, 0.86f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.54f, 0.54f, 0.75f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.57f, 0.41f, 0.94f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.83f, 0.41f, 0.94f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.74f, 0.60f, 0.96f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.76f, 0.62f, 0.86f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.76f, 0.62f, 0.86f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.86f, 0.54f, 0.75f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.83f, 0.41f, 0.94f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 // red
								 0.54f, 0.54f, 0.75f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.64f, 0.62f, 0.86f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.63f, 0.75f, 0.80f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.63f, 0.75f, 0.80f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.53f, 0.71f, 0.69f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.54f, 0.54f, 0.75f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.86f, 0.54f, 0.75f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.76f, 0.62f, 0.86f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.77f, 0.75f, 0.80f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.77f, 0.75f, 0.80f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.87f, 0.71f, 0.69f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.86f, 0.54f, 0.75f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,

								 //////////////////////

								 // horn bends here green
								 0.63f, 0.75f, 0.80f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.51f, 0.93f, 0.82f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.49f, 0.93f, 0.72f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.49f, 0.93f, 0.72f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.53f, 0.71f, 0.69f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.63f, 0.75f, 0.80f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.77f, 0.75f, 0.80f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.89f, 0.93f, 0.82f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.90f, 0.93f, 0.72f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.90f, 0.93f, 0.72f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.87f, 0.71f, 0.69f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.77f, 0.75f, 0.80f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 //pointed horn
								 0.51f, 0.93f, 0.82f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.49f, 0.93f, 0.72f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.31f, 1.12f, 0.98f, 		0.5f, 1.0f,		1.0f, 0.0f, 0.0f,

								 1.89f, 0.93f, 0.82f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.90f, 0.93f, 0.72f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 2.09f, 1.12f, 0.98f, 		0.5f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 // 2nd layer of horn seen from side elevation
								 // starting from top
								 0.85f, 0.66f, 1.43f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.76f, 0.66f, 1.40f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.62f, 0.46f, 1.20f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.62f, 0.46f, 1.20f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.68f, 0.39f, 1.19f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.85f, 0.66f, 1.43f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,


								 1.55f, 0.66f, 1.43f, 		0.0f, 0.0f,		-1.0f, 0.0f, .0f,
								 1.64f, 0.66f, 1.40f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.78f, 0.46f, 1.20f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.78f, 0.46f, 1.20f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.72f, 0.39f, 1.19f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.55f, 0.66f, 1.43f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 0.68f, 0.39f, 1.19f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.62f, 0.46f, 1.20f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.57f, 0.41f, 0.94f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.57f, 0.41f, 0.94f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.61f, 0.36f, 0.94f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.68f, 0.39f, 1.19f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.72f, 0.39f, 1.19f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.78f, 0.46f, 1.20f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.83f, 0.41f, 0.94f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.83f, 0.41f, 0.94f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.79f, 0.36f, 0.94f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.72f, 0.39f, 1.19f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 0.61f, 0.36f, 0.94f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.57f, 0.41f, 0.94f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.54f, 0.54f, 0.75f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.54f, 0.54f, 0.75f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.57f, 0.51f, 0.69f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.61f, 0.36f, 0.94f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.79f, 0.36f, 0.94f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.83f, 0.41f, 0.94f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.86f, 0.54f, 0.75f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.86f, 0.54f, 0.75f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.83f, 0.51f, 0.69f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.79f, 0.36f, 0.94f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 // complicated bend here
								 0.54f, 0.54f, 0.75f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.57f, 0.51f, 0.69f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.56f, 0.71f, 0.63f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.56f, 0.71f, 0.63f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.57f, 0.51f, 0.69f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.54f, 0.54f, 0.75f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.86f, 0.54f, 0.75f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.83f, 0.51f, 0.69f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.76f, 0.71f, 0.63f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.76f, 0.71f, 0.63f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.83f, 0.51f, 0.69f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.86f, 0.54f, 0.75f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 0.53f, 0.71f, 0.69f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.49f, 0.93f, 0.72f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.45f, 0.96f, 0.72f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.45f, 0.96f, 0.72f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.56f, 0.71f, 0.62f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.53f, 0.71f, 0.69f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.87f, 0.71f, 0.69f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.90f, 0.93f, 0.72f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.95f, 0.96f, 0.72f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.95f, 0.96f, 0.72f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.76f, 0.71f, 0.62f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.87f, 0.71f, 0.69f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 // pointed horn
								 0.45f, 0.96f, 0.72f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.49f, 0.93f, 0.72f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.31f, 1.12f, 0.98f, 		0.5f, 1.0f,		1.0f, 0.0f, 0.0f,

								 1.95f, 0.96f, 0.72f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.90f, 0.93f, 0.72f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 2.09f, 1.12f, 0.98f, 		0.5f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 // horn inside/ underneath from front elevation starts here
								 0.95f, 0.91f, 1.23f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.83f, 0.70f, 1.23f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.72f, 0.71f, 1.18f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.72f, 0.71f, 1.18f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.86f, 0.94f, 1.18f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.95f, 0.91f, 1.23f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.45f, 0.91f, 1.23f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.57f, 0.70f, 1.23f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.68f, 0.71f, 1.18f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.68f, 0.71f, 1.18f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.54f, 0.94f, 1.18f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.45f, 0.91f, 1.23f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,

								 //////////////////////

								 0.72f, 0.71f, 1.18f, 		0.0f, 0.0f,		1.0f, 1.0f, 0.0f,
								 0.68f, 0.62f, 1.08f, 		1.0f, 0.0f,		1.0f, 1.0f, 0.0f,
								 0.79f, 0.58f, 1.08f, 		1.0f, 1.0f,		1.0f, 1.0f, 0.0f,
								 0.79f, 0.58f, 1.08f, 		1.0f, 1.0f,		1.0f, 1.0f, 0.0f,
								 0.83f, 0.70f, 1.23f, 		0.0f, 1.0f,		1.0f, 1.0f, 0.0f,
								 0.72f, 0.71f, 1.18f, 		0.0f, 0.0f,		1.0f, 1.0f, 0.0f,

								 1.68f, 0.71f, 1.18f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.72f, 0.62f, 1.08f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.61f, 0.58f, 1.08f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.61f, 0.58f, 1.08f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.57f, 0.70f, 1.23f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.68f, 0.71f, 1.18f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 0.68f, 0.62f, 1.08f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.79f, 0.58f, 1.08f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.76f, 0.56f, 0.96f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.76f, 0.56f, 0.96f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.79f, 0.58f, 1.08f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.68f, 0.62f, 1.08f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.72f, 0.62f, 1.08f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.61f, 0.58f, 1.08f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.64f, 0.56f, 0.96f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.64f, 0.56f, 0.96f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.61f, 0.58f, 1.08f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.72f, 0.62f, 1.08f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 0.66f, 0.60f, 0.96f, 		0.0f, 0.0f,		1.0f, 1.0f, 0.0f,
								 0.64f, 0.62f, 0.86f, 		1.0f, 0.0f,		1.0f, 1.0f, 0.0f,
								 0.74f, 0.60f, 0.82f, 		1.0f, 1.0f,		1.0f, 1.0f, 0.0f,
								 0.74f, 0.60f, 0.82f, 		1.0f, 1.0f,		1.0f, 1.0f, 0.0f,
								 0.76f, 0.56f, 0.96f, 		0.0f, 1.0f,		1.0f, 1.0f, 0.0f,
								 0.68f, 0.60f, 0.82f, 		0.0f, 0.0f,		1.0f, 1.0f, 0.0f,

								 1.74f, 0.60f, 0.96f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.76f, 0.62f, 0.86f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.66f, 0.60f, 0.82f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.66f, 0.60f, 0.82f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.64f, 0.56f, 0.96f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.72f, 0.60f, 0.82f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////


								 0.64f, 0.62f, 0.86f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.63f, 0.75f, 0.80f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.71f, 0.75f, 0.77f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.71f, 0.75f, 0.77f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.74f, 0.60f, 0.82f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.64f, 0.62f, 0.86f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.76f, 0.62f, 0.86f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.77f, 0.75f, 0.80f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.69f, 0.75f, 0.77f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.69f, 0.75f, 0.77f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.66f, 0.60f, 0.82f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.76f, 0.62f, 0.86f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 // bent, is confusing here
								 0.63f, 0.75f, 0.80f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.63f, 0.93f, 0.82f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.73f, 0.95f, 0.81f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.73f, 0.95f, 0.81f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.71f, 0.75f, 0.77f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.63f, 0.75f, 0.80f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.77f, 0.75f, 0.80f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.77f, 0.93f, 0.82f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.67f, 0.95f, 0.81f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.67f, 0.95f, 0.81f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.69f, 0.75f, 0.77f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.77f, 0.75f, 0.80f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 //pointed
								 0.63f, 0.93f, 0.82f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.73f, 0.95f, 0.81f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.31f, 1.12f, 0.98f, 		0.5f, 1.0f,		1.0f, 0.0f, 0.0f,

								 1.77f, 0.93f, 0.82f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.67f, 0.95f, 0.81f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 2.09f, 1.12f, 0.98f, 		0.5f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 // horn back elevation starting from top horn, red
								 1.00f, 0.94f, 1.37f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.85f, 0.66f, 1.43f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.83f, 0.70f, 1.23f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.83f, 0.70f, 1.23f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.95f, 0.91f, 1.23f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.00f, 0.94f, 1.37f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.40f, 0.94f, 1.37f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.55f, 0.66f, 1.43f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.57f, 0.70f, 1.23f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.57f, 0.70f, 1.23f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.45f, 0.91f, 1.23f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.40f, 0.94f, 1.37f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,

								 //////////////////////

								 0.85f, 0.66f, 1.43f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.68f, 0.39f, 1.19f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.68f, 0.58f, 1.08f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.68f, 0.58f, 1.08f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.83f, 0.70f, 1.23f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.85f, 0.66f, 1.43f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.55f, 0.66f, 1.43f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.72f, 0.39f, 1.19f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.72f, 0.58f, 1.08f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.72f, 0.58f, 1.08f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.57f, 0.70f, 1.23f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.55f, 0.66f, 1.43f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 0.68f, 0.39f, 1.19f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.61f, 0.36f, 0.94f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.76f, 0.56f, 0.96f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.76f, 0.56f, 0.96f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.79f, 0.58f, 1.08f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.68f, 0.39f, 1.19f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.72f, 0.39f, 1.19f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.79f, 0.36f, 0.94f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.64f, 0.56f, 0.96f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.64f, 0.56f, 0.96f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.61f, 0.58f, 1.08f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.72f, 0.39f, 1.19f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 0.61f, 0.36f, 0.94f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.53f, 0.51f, 0.69f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.74f, 0.60f, 0.82f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.74f, 0.60f, 0.82f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.76f, 0.56f, 0.96f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.61f, 0.36f, 0.94f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.79f, 0.36f, 0.94f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.87f, 0.51f, 0.69f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.66f, 0.60f, 0.82f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.66f, 0.60f, 0.82f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.64f, 0.56f, 0.96f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.79f, 0.36f, 0.94f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 0.53f, 0.51f, 0.69f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.74f, 0.60f, 0.82f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.71f, 0.75f, 0.77f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.71f, 0.75f, 0.77f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.64f, 0.71f, 0.63f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.53f, 0.51f, 0.69f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.87f, 0.51f, 0.69f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.66f, 0.60f, 0.82f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.69f, 0.75f, 0.77f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.69f, 0.75f, 0.77f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.76f, 0.71f, 0.63f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.87f, 0.51f, 0.69f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 0.64f, 0.71f, 0.63f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.71f, 0.75f, 0.77f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.63f, 0.95f, 0.81f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.63f, 0.95f, 0.81f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.71f, 0.75f, 0.77f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 0.64f, 0.71f, 0.63f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.76f, 0.71f, 0.63f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.69f, 0.75f, 0.77f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.77f, 0.95f, 0.81f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.77f, 0.95f, 0.81f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.69f, 0.75f, 0.77f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.76f, 0.71f, 0.63f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 // pointed horn
								 0.63f, 0.95f, 0.81f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.45f, 0.96f, 0.72f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 0.31f, 1.12f, 0.98f, 		0.5f, 1.0f,		1.0f, 0.0f, 0.0f,

								 1.77f, 0.95f, 0.81f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.95f, 0.96f, 0.72f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 2.09f, 1.12f, 0.98f, 		0.5f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 // nose detailings
								 1.10f, 1.35f, 0.84f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.20f, 1.36f, 0.91f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.20f, 1.47f, 0.79f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.20f, 1.47f, 0.79f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.14f, 1.46f, 0.76f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.10f, 1.35f, 0.84f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.30f, 1.35f, 0.84f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.20f, 1.36f, 0.91f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.20f, 1.47f, 0.79f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.20f, 1.47f, 0.79f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.26f, 1.46f, 0.76f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.30f, 1.35f, 0.84f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 1.10f, 1.35f, 0.84f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.14f, 1.46f, 0.76f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.12f, 1.46f, 0.71f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.12f, 1.46f, 0.71f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.07f, 1.34f, 0.75f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.10f, 1.35f, 0.84f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.30f, 1.35f, 0.84f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.26f, 1.46f, 0.76f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.28f, 1.46f, 0.71f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.28f, 1.46f, 0.71f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.33f, 1.34f, 0.75f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.30f, 1.35f, 0.84f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////


								 1.07f, 1.34f, 0.75f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.12f, 1.46f, 0.71f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.14f, 1.45f, 0.68f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.14f, 1.45f, 0.68f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.07f, 1.29f, 0.70f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.07f, 1.34f, 0.75f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.33f, 1.34f, 0.75f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.28f, 1.46f, 0.71f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.26f, 1.45f, 0.68f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.26f, 1.45f, 0.68f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.33f, 1.29f, 0.70f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.33f, 1.34f, 0.75f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 // under lips
								 1.07f, 1.29f, 0.70f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.11f, 1.41f, 0.63f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.20f, 1.40f, 0.61f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.20f, 1.40f, 0.61f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.08f, 1.26f, 0.65f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.07f, 1.29f, 0.70f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.33f, 1.29f, 0.70f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.29f, 1.41f, 0.63f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.20f, 1.40f, 0.61f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.20f, 1.40f, 0.61f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.32f, 1.26f, 0.65f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.33f, 1.29f, 0.70f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 // beside nose triangle
								 1.20f, 1.47f, 0.79f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.14f, 1.46f, 0.76f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.20f, 1.48f, 0.71f, 		0.5f, 1.0f,		1.0f, 0.0f, 0.0f,

								 1.20f, 1.47f, 0.79f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.26f, 1.46f, 0.76f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.20f, 1.48f, 0.71f, 		0.5f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 // next to nose three triangles
								 1.12f, 1.46f, 0.71f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.19f, 1.48f, 0.71f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.14f, 1.45f, 0.68f, 		0.5f, 1.0f,		.0f, 0.0f, 0.0f,

								 1.28f, 1.46f, 0.71f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.21f, 1.48f, 0.71f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.26f, 1.45f, 0.68f, 		0.5f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 1.14f, 1.45f, 0.68f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.20f, 1.48f, 0.71f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.19f, 1.48f, 0.71f, 		0.5f, 1.0f,		1.0f, 0.0f, 0.0f,

								 1.26f, 1.45f, 0.68f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.20f, 1.48f, 0.71f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.21f, 1.48f, 0.71f, 		0.5f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 1.14f, 1.45f, 0.68f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.20f, 1.48f, 0.68f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.20f, 1.48f, 0.71f, 		0.5f, 1.0f,		1.0f, 0.0f, 0.0f,

								 1.26f, 1.45f, 0.68f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.20f, 1.48f, 0.68f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.20f, 1.48f, 0.71f, 		0.5f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 // lips
								 1.07f, 1.29f, 0.70f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.14f, 1.45f, 0.68f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.20f, 1.43f, 0.66f, 		0.5f, 1.0f,		1.0f, 0.0f, 0.0f,

								 1.33f, 1.29f, 0.70f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.26f, 1.45f, 0.68f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.20f, 1.43f, 0.66f, 		0.5f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 1.07f, 1.29f, 0.70f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.20f, 1.43f, 0.66f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.14f, 1.42f, 0.65f, 		0.5f, 1.0f,		1.0f, 0.0f, 0.0f,

								 1.33f, 1.29f, 0.70f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.20f, 1.43f, 0.66f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.26f, 1.42f, 0.65f, 		0.5f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 1.07f, 1.29f, 0.70f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.14f, 1.42f, 0.65f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.14f, 1.41f, 0.63f, 		0.5f, 1.0f,		1.0f, 0.0f, 0.0f,

								 1.33f, 1.29f, 0.70f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.26f, 1.42f, 0.65f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.26f, 1.41f, 0.63f, 		0.5f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////


								 // front lips
								 1.20f, 1.48f, 0.68f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.14f, 1.45f, 0.68f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.20f, 1.43f, 0.66f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.20f, 1.43f, 0.66f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.20f, 1.46f, 0.67f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.20f, 1.48f, 0.68f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.20f, 1.48f, 0.68f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.26f, 1.45f, 0.68f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.20f, 1.43f, 0.66f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.20f, 1.43f, 0.66f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.20f, 1.46f, 0.67f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.20f, 1.48f, 0.68f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 1.20f, 1.46f, 0.67f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.20f, 1.43f, 0.66f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.14f, 1.42f, 0.65f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.14f, 1.42f, 0.65f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.20f, 1.45f, 0.65f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.20f, 1.46f, 0.67f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.20f, 1.46f, 0.67f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.20f, 1.43f, 0.66f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.26f, 1.42f, 0.65f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.26f, 1.42f, 0.65f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.20f, 1.45f, 0.65f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.20f, 1.46f, 0.67f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 1.20f, 1.45f, 0.65f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.14f, 1.42f, 0.65f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.14f, 1.41f, 0.63f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.14f, 1.41f, 0.63f, 		1.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.20f, 1.45f, 0.64f, 		0.0f, 1.0f,		1.0f, 0.0f, 0.0f,
								 1.20f, 1.45f, 0.65f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,

								 1.20f, 1.45f, 0.65f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.26f, 1.42f, 0.65f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.26f, 1.41f, 0.63f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.26f, 1.41f, 0.63f, 		1.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.20f, 1.45f, 0.64f, 		0.0f, 1.0f,		-1.0f, 0.0f, 0.0f,
								 1.20f, 1.45f, 0.65f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 //////////////////////

								 1.20f, 1.45f, 0.64f, 		0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.14f, 1.41f, 0.63f, 		1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
								 1.20f, 1.40f, 0.61f, 		0.5f, 1.0f,		1.0f, 0.0f, 0.0f,

								 1.20f, 1.45f, 0.64f, 		0.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.26f, 1.41f, 0.63f, 		1.0f, 0.0f,		-1.0f, 0.0f, 0.0f,
								 1.20f, 1.40f, 0.61f, 		0.5f, 1.0f,		-1.0f, 0.0f, 0.0f
								 //////////////////////
								 // some detailings still trying to figure out

				};


	// Generate buffer ids
	glGenVertexArrays(1, &RamVAO);
	glGenBuffers(1, &VBO);

	// Activate the Vertex array Object before binding and setting any VBOs and Vertex Attribute Pointers.
	glBindVertexArray(RamVAO);

	// Activate the VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // Copy vertices to VBO

	// Set attribute pointer 0 to hold Position data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0); // enables vertex attribute

	// Set attribute pointer 1 to hold Normal data
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1); // Enables vertex attribute

	// Set attribute pointer 1 to hold Color data
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2); // Enables vertex attribute

	glBindVertexArray(0); // Deactivates the  Ram VAO which is good practice


	// Generate buffer ids for lamp(smaller ram)
	glGenVertexArrays(1, &LightVAO); // Vertex Array Object for Ram vertex copies to serve as a light source

	// Activate the Vertex Array Object before binding and setting any VBOs and Vertex Attribute Pointers
	glBindVertexArray(LightVAO);

	// Referencing the same VBO for its vertices
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// Set attribute pointer 0 to hold Position data (used for the lamp)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0); // enables vertex attribute
	glBindVertexArray(0); // Deactivates the VAO which is good practice


}


/* Implements the UKeyboard function*/
void UKeyboardPressed(unsigned char key, GLint x, GLint y)
{

	switch(key){
			case GLUT_ACTIVE_ALT:

				currentKey = key;
				int modifier;
				modifier = glutGetModifiers();
				if (modifier == GLUT_ACTIVE_ALT){
					cout << "You pressed ALT!"<<endl;
				}
				break;

			default:
				cout << "This is not an ALT key"<<endl;
	}
}


/* Implements the UKeyboard function*/
void pressSpecialKey(int key, int x, int y)
{

	switch(key){

			case GLUT_KEY_LEFT: // for orthographic view
				cout << "You are watching the orthographic view.";
				view_state = 0;
				break;

			case GLUT_KEY_RIGHT: // for perspective view
				cout << "You are watching the perspective view.";
				view_state = 1;
				break;

	}
}



/*Implements the UKeyreleased function*/
void UKeyReleased(unsigned char key, GLint x, GLint y)
{
	cout<<"Key released!"<<endl;
	currentKey = '0';
}


/*Implements the UMouseClick function*/
void UMouseClick(int button, int state, int x, int y ) {

	specialKey = glutGetModifiers(); //GLUT function to detect if ALT key is pressed

		if((button == GLUT_LEFT_BUTTON) &&  // detects if left mouse click and ALT key are pressed
		   (state == GLUT_DOWN) &&
		   (specialKey == GLUT_ACTIVE_ALT)) {
			leftmousebuttonDetected = true;
		}

		if((button == GLUT_LEFT_BUTTON) && (state == GLUT_UP)) {   // detects if left mouse released
			leftmousebuttonDetected = false;
			mouseDetected = true;
		}

		if((button == GLUT_RIGHT_BUTTON) &&  // detects if right mouse click and ALT key pressed
		   (state == GLUT_DOWN) &&
		   (specialKey == GLUT_ACTIVE_ALT)) {
			rightmousebuttonDetected = true;
		}

		if((button == GLUT_RIGHT_BUTTON) && (state == GLUT_UP)) {   // detects if right mouse released
			rightmousebuttonDetected = false;
			mouseDetected = true;
		}

}


/*Implements the UMousePrssedMove function*/
void UMouseMove(int x, int y)
{
	cout<<"Mouse is moving at " << "(" << x << "," << y << ")" << " while not pressed!"<<endl;
}


/* Implements the UMouseMove function*/
void UMousePressedMove(int x, int y)
{

// When the left mouse button is clicked along with ALT key
	if (leftmousebuttonDetected){

		// Immediately replaces center locked coordinates with new mouse coordinates
		if(mouseDetected)
		{
			lastMouseX = x;
			lastMouseY = y;

			mouseDetected = false;
		}

		// Gets the direction the mouse was moved in x and y
		mouseXOffset = x - lastMouseX;
		mouseYOffset = lastMouseY - y; // Inverted Y

		// Updates with new mouse coordinates
		lastMouseX = x;
		lastMouseY = y;

		// Applies sensitivity to mouse direction
		mouseXOffset *= sensitivity;
		mouseYOffset *= sensitivity;

		// Accumulates the yaw and pitch variables
		yaw += mouseXOffset;
		pitch += mouseYOffset;

		//Orbits around the center
		front.x = 10.f * cos(yaw);
		front.y = 10.0f * sin(pitch);
		front.z = sin(yaw) * cos(pitch) * 10.f;

}

// when the right mouse button is clicked along with ALT key
		if (rightmousebuttonDetected) {

			// Immediately replaces center locked coordinates with new mouse coordinates
			if(mouseDetected)
			{
				//lastMouseX = x;
				lastMouseY = y;
				mouseDetected = false;
			}

			mouseYOffset = lastMouseY - y; // Used to determine direction of mouse on y axis
			// Updates with new mouse coordinates
			lastMouseY = y;

		}
}


/* Generate and load the texture */
void UGenerateTexture() {
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		int width, height;

		unsigned char* image = SOIL_load_image("multi-toned-grungy.jpg", &width, &height, 0, SOIL_LOAD_RGB); // Loads texture file

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		glGenerateMipmap(GL_TEXTURE_2D);
		SOIL_free_image_data(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

}












