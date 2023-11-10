#define  _CRT_SECURE_NO_WARNINGS
#include <iostream>

#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <stdlib.h>
#include <random>

#define MAXX 1000
#define MAXY 700

void make_vertexShaders();
void make_fragmentShaders();
void make_shaderProgram();
char* filetobuf(const char* file);
GLvoid InitBuffer();

void Mouse(int, int, int, int);
GLvoid drawScene();
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
void Motion(int, int);
void Fly(int);
void Create(int);

void convertCoordinate(int x, int y, double& convertedX, double& convertedY) {
	convertedX = (2.0 * x / MAXX) - 1.0;
	convertedY = 1.0 - (2.0 * y / MAXY);
}


std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<> createcoord_x(1.0, 1.5);
std::uniform_real_distribution<> createcoord_y(-0.5, 0.5);
std::uniform_real_distribution<> createcoord_2(0.1, 0.2);
// GLfloat tri_size = createcoord(gen);
std::uniform_real_distribution<> color(0, 1);


GLchar* vertexSource, * fragmentSource; //--- 소스코드 저장 변수
GLuint vertexShader, fragmentShader; //--- 세이더 객체
GLuint shaderProgramID;
GLuint vao, vbo[2];

GLuint vbo_line[2];
GLfloat line[2][6] = {
	{0, 1, 0, 0,-1, 0},
	{1, 0, 0,-1, 0, 0}
};
GLfloat line_RGB[2][6] = {
	{0, 0, 1, 0, 0, 1},
	{1, 0, 0, 1, 0, 0}
};


bool click;
GLuint vbo_cutting_line[2];
GLfloat cutting_line[2][3];
GLfloat cutting_line_RGB[2][3];

GLuint vbo_triangle[2];
GLfloat triangle[3][3];
GLfloat triangle_RGB[3][3];

GLuint vbo_square[2];
GLfloat squrae[4][3];
GLfloat square_RGB[4][3];

GLuint vbo_underbox[2];
GLfloat underbox[4][3] = {
	{-0.3f,-0.8f, 0.f},
	{-0.3f,-0.9f, 0.f},
	{ 0.3f,-0.9f, 0.f},
	{ 0.3f,-0.8f, 0.f}
};
GLfloat underbox_RGB[4][3] = {
	{0.0f, 0.0f, 1.0f},
	{0.0f, 0.0f, 1.0f},
	{0.0f, 0.0f, 1.0f},
	{0.0f, 0.0f, 1.0f}
};

GLfloat point[100][12];
GLfloat RGB[100][12];

int figure_type{ 0 };
GLfloat figure[10][12];



int shape_mode;
int shape_count;
int shape_type[10];

void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{

	// 윈도우 생성
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(MAXX, MAXY);
	glutCreateWindow("Let's SP");
	// GLEW 초기화
	glewExperimental = GL_TRUE;
	glewInit();

	//--- 세이더 읽어와서 세이더 프로그램 만들기
	make_shaderProgram();
	glUseProgram(shaderProgramID);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(2, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, 4 * 9 * sizeof(GLfloat), figure, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, 4 * 9 * sizeof(GLfloat), RGB, GL_STATIC_DRAW);

	InitBuffer();
	glutDisplayFunc(drawScene); // 장면을 다시 그리는데 필요한 루틴들은 모두 이 함수 안에 넣어둔다.
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(Mouse);
	glutMotionFunc(Motion);
	glutTimerFunc(1000, Create, 0);
	glutMainLoop();
}

GLvoid drawScene() //--- 콜백 함수: 그리기 콜백 함수
{
	GLfloat rColor, gColor, bColor;
	rColor = gColor = bColor = 1.0;
	glClearColor(rColor, gColor, bColor, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(shaderProgramID);

	glBindVertexArray(vao);
	int PosLocation = glGetAttribLocation(shaderProgramID, "in_Position"); //	: 0  Shader의 'layout (location = 0)' 부분
	int ColorLocation = glGetAttribLocation(shaderProgramID, "in_Color"); //	: 1

	glEnableVertexAttribArray(PosLocation); // Enable 필수! 사용하겠단 의미
	glEnableVertexAttribArray(ColorLocation);

	// Draw XY_Line
	for (int i = 0; i < 2; ++i) {
		glBindBuffer(GL_ARRAY_BUFFER, vbo_line[0]);
		glVertexAttribPointer(PosLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)(i * 6 * sizeof(GLfloat)));
		glEnableVertexAttribArray(PosLocation);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_line[1]);
		glVertexAttribPointer(ColorLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)(i * 6 * sizeof(GLfloat)));
		glEnableVertexAttribArray(ColorLocation);
		glDrawArrays(GL_LINES, 0, 2);
	}


	// Draw underbox
	glBindBuffer(GL_ARRAY_BUFFER, vbo_underbox[0]);
	glVertexAttribPointer(PosLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(PosLocation);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_underbox[1]);
	glVertexAttribPointer(ColorLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(ColorLocation);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	// Draw Flying_Figure
	for (int i = 0; i < shape_count; ++i) {
		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glVertexAttribPointer(PosLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)(i * 12 * sizeof(GLfloat)));
		glEnableVertexAttribArray(PosLocation);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		glVertexAttribPointer(ColorLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)(i * 12 * sizeof(GLfloat)));
		glEnableVertexAttribArray(ColorLocation);
		if (shape_type[i] == 1)
			glDrawArrays(GL_TRIANGLES, 0, 3);
		else if (shape_type[i] == 2)
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}


	if (click) {
		glBindBuffer(GL_ARRAY_BUFFER, vbo_cutting_line[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cutting_line), cutting_line, GL_STATIC_DRAW);
		glVertexAttribPointer(PosLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_cutting_line[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cutting_line_RGB), cutting_line_RGB, GL_STATIC_DRAW);
		glVertexAttribPointer(ColorLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
		glLineWidth(2);
		glDrawArrays(GL_LINES, 0, 2);
	}


	glDisableVertexAttribArray(PosLocation); // Disable 필수!
	glDisableVertexAttribArray(ColorLocation);

	glutSwapBuffers(); // 화면에 출력하기
}

GLvoid InitBuffer()
{
	//---VAO 객체 생성 및 바인딩
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	// vertex data 저장을 위한 VBO 생성 및 바인딩
	glGenBuffers(2, vbo_line);
	glGenBuffers(2, vbo_triangle);
	glGenBuffers(2, vbo_square);
	glGenBuffers(2, vbo_cutting_line);
	glGenBuffers(2, vbo_underbox);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_line[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_line[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(line_RGB), line_RGB, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_underbox[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(underbox), underbox, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_underbox[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(underbox_RGB), underbox_RGB, GL_STATIC_DRAW);


	//glGenBuffers(1, &vbo);
	//glBindBuffer(GL_ARRAY_BUFFER, vbo);
	//// vertex data 데이터 입력
	//glBufferData(GL_ARRAY_BUFFER, sizeof vertexData), vertexData, GL_STATIC_DRAW);
	//// 위치 속성 : 속성 위치 0
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	//glEnableVertexAttribArray(0);
	//// 색상 속성 : 속성 위치 1
	//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	//glEnableVertexAttribArray(1);
}

GLvoid Reshape(int w, int h) //--- 콜백 함수: 다시 그리기 콜백 함수
{
	glViewport(0, 0, w, h);
}
void make_vertexShaders()
{
	vertexSource = filetobuf("vertex.glsl");
	//--- 버텍스 세이더 객체 만들기
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	//--- 세이더 코드를 세이더 객체에 넣기
	glShaderSource(vertexShader, 1, (const GLchar**)&vertexSource, 0);
	//--- 버텍스 세이더 컴파일하기
	glCompileShader(vertexShader);
	//--- 컴파일이 제대로 되지 않은 경우: 에러 체크
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
		std::cout << "ERROR: vertex shader 컴파일 실패\n" << errorLog << std::endl;
		return;
	}
}
void make_fragmentShaders()
{
	fragmentSource = filetobuf("fragment.glsl");
	//--- 프래그먼트 세이더 객체 만들기
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	//--- 세이더 코드를 세이더 객체에 넣기
	glShaderSource(fragmentShader, 1, (const GLchar**)&fragmentSource, 0);
	//--- 프래그먼트 세이더 컴파일
	glCompileShader(fragmentShader);
	//--- 컴파일이 제대로 되지 않은 경우: 컴파일 에러 체크
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		std::cerr << "ERROR: fragment shader 컴파일 실패\n" << errorLog << std::endl;
		return;
	}
}
void make_shaderProgram()
{
	make_vertexShaders(); //--- 버텍스 세이더 만들기
	make_fragmentShaders(); //--- 프래그먼트 세이더 만들기
	//-- shader Program
	shaderProgramID = glCreateProgram();
	glAttachShader(shaderProgramID, vertexShader);
	glAttachShader(shaderProgramID, fragmentShader);
	glLinkProgram(shaderProgramID);
	//--- 세이더 삭제하기
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	//--- Shader Program 사용하기
	glUseProgram(shaderProgramID);
}
char* filetobuf(const char* file)
{
	FILE* fptr;
	long length;
	char* buf;
	fptr = fopen(file, "rb"); // Open file for reading
	if (!fptr) // Return NULL on failure
		return NULL;
	fseek(fptr, 0, SEEK_END); // Seek to the end of the file
	length = ftell(fptr); // Find out how many bytes into the file we are
	buf = (char*)malloc(length + 1); // Allocate a buffer for the entire length of the file and a null terminator
	fseek(fptr, 0, SEEK_SET); // Go back to the beginning of the file
	fread(buf, length, 1, fptr); // Read the contents of the file in to the buffer
	fclose(fptr); // Close the file
	buf[length] = 0; // Null terminator
	return buf; // Return the buffer
}

void Mouse(int button, int state, int x, int y)
{
	GLdouble convertedX, convertedY;
	convertCoordinate(x, y, convertedX, convertedY);

	if (state == GLUT_DOWN)	{
		if (button == GLUT_LEFT_BUTTON) {
			click = true;

			cutting_line[0][0] = convertedX;
			cutting_line[0][1] = convertedY;
			cutting_line[0][2] = 0.f;
			cutting_line[1][0] = convertedX;
			cutting_line[1][1] = convertedY;
			cutting_line[1][2] = 0.f;
		}
	}
	else if (state == GLUT_UP) {
		if (button == GLUT_LEFT_BUTTON) {
			click = false;

			cutting_line[0][0] = convertedX;
			cutting_line[0][1] = convertedY;
			cutting_line[0][2] = 0.f;
			cutting_line[1][0] = convertedX;
			cutting_line[1][1] = convertedY;
			cutting_line[1][2] = 0.f;
		}
	}

	glutPostRedisplay();
}

void Motion(int x, int y)
{
	GLdouble convertedX, convertedY;
	convertCoordinate(x, y, convertedX, convertedY);

	if (click) {
		cutting_line[1][0] = convertedX;
		cutting_line[1][1] = convertedY;
		cutting_line[1][2] = 0.f;
	}
	glutPostRedisplay();
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
		switch (key) {
		case '1':
		{
			shape_mode = 1;

			shape_type[shape_count] = shape_mode;

			double random_point_x = createcoord_x(gen);
			double random_point_y = createcoord_y(gen);
			double random_point_4 = createcoord_2(gen);
			point[shape_count][0] = random_point_x - random_point_4;
			point[shape_count][1] = random_point_y - random_point_4;
			point[shape_count][2] = 0.0f;
			point[shape_count][3] = random_point_x + random_point_4;
			point[shape_count][4] = random_point_y - random_point_4;
			point[shape_count][5] = 0.0f;
			point[shape_count][6] = random_point_x;
			point[shape_count][7] = random_point_y + random_point_4;
			point[shape_count][8] = 0.0f;
			for (int i = 0; i < 3; ++i) {
				RGB[shape_count][i] = color(gen);
			}
			for (int i = 0; i < 3; ++i) {
				RGB[shape_count][i + 6] = RGB[shape_count][i + 3] = RGB[shape_count][i];
			}
			++shape_count;
			for (int i = 0; i < shape_count; ++i)
				glutTimerFunc(100, Fly, i);
			break;
		}
		case '2':
		{
			shape_mode = 2;

			shape_type[shape_count] = shape_mode;

			double random_point_x = createcoord_x(gen);
			double random_point_y = createcoord_y(gen);
			double random_point_2 = createcoord_2(gen);
			point[shape_count][0] = random_point_x - createcoord_2(gen);
			point[shape_count][1] = random_point_y - createcoord_2(gen);
			point[shape_count][2] = 0.0f;
			point[shape_count][3] = random_point_x - createcoord_2(gen);
			point[shape_count][4] = random_point_y + createcoord_2(gen);
			point[shape_count][5] = 0.0f;
			point[shape_count][6] = random_point_x + createcoord_2(gen);
			point[shape_count][7] = random_point_y - createcoord_2(gen);
			point[shape_count][8] = 0.0f;
			point[shape_count][9] = random_point_x + createcoord_2(gen);
			point[shape_count][10] = random_point_y + createcoord_2(gen);
			point[shape_count][11] = 0.0f;
			for (int i = 0; i < 3; ++i) {
				RGB[shape_count][i] = color(gen);
			}
			for (int i = 0; i < 3; ++i) {
				RGB[shape_count][i + 9] = RGB[shape_count][i + 6] = RGB[shape_count][i + 3] = RGB[shape_count][i];
			}
			++shape_count;
			for (int i = 0; i < shape_count; ++i)
				glutTimerFunc(100, Fly, i);
			break;
		}
		case 'q':
		case 'Q':
			exit(0);
			break;
		}
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, shape_count * 12 * sizeof(GLfloat), point, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, shape_count * 12 * sizeof(GLfloat), RGB, GL_STATIC_DRAW);
	
	glutPostRedisplay();
}

void Create(int value)
{
	std::uniform_int_distribution<> mode(1, 2);
	int flag = mode(gen);
	std::cout << "key : " << flag << '\n';
	if (shape_count < 10) {
		switch (flag) {
		case '1':
		{
			shape_mode = 1;

			shape_type[shape_count] = shape_mode;

			double random_point_x = createcoord_x(gen);
			double random_point_y = createcoord_y(gen);
			double random_point_4 = createcoord_2(gen);
			point[shape_count][0] = random_point_x - random_point_4;
			point[shape_count][1] = random_point_y - random_point_4;
			point[shape_count][2] = 0.0f;
			point[shape_count][3] = random_point_x + random_point_4;
			point[shape_count][4] = random_point_y - random_point_4;
			point[shape_count][5] = 0.0f;
			point[shape_count][6] = random_point_x;
			point[shape_count][7] = random_point_y + random_point_4;
			point[shape_count][8] = 0.0f;
			for (int i = 0; i < 3; ++i) {
				RGB[shape_count][i] = color(gen);
			}
			for (int i = 0; i < 3; ++i) {
				RGB[shape_count][i + 6] = RGB[shape_count][i + 3] = RGB[shape_count][i];
			}
			++shape_count;
			for (int i = 0; i < 10; ++i)
				glutTimerFunc(100, Fly, i);
			break;
		}
		case '2':
		{
			shape_mode = 2;

			shape_type[shape_count] = shape_mode;

			double random_point_x = createcoord_x(gen);
			double random_point_y = createcoord_y(gen);
			double random_point_2 = createcoord_2(gen);
			point[shape_count][0] = random_point_x - createcoord_2(gen);
			point[shape_count][1] = random_point_y - createcoord_2(gen);
			point[shape_count][2] = 0.0f;
			point[shape_count][3] = random_point_x - createcoord_2(gen);
			point[shape_count][4] = random_point_y + createcoord_2(gen);
			point[shape_count][5] = 0.0f;
			point[shape_count][6] = random_point_x + createcoord_2(gen);
			point[shape_count][7] = random_point_y - createcoord_2(gen);
			point[shape_count][8] = 0.0f;
			point[shape_count][9] = random_point_x + createcoord_2(gen);
			point[shape_count][10] = random_point_y + createcoord_2(gen);
			point[shape_count][11] = 0.0f;
			for (int i = 0; i < 3; ++i) {
				RGB[shape_count][i] = color(gen);
			}
			for (int i = 0; i < 3; ++i) {
				RGB[shape_count][i + 9] = RGB[shape_count][i + 6] = RGB[shape_count][i + 3] = RGB[shape_count][i];
			}
			++shape_count;
			for (int i = 0; i < 10; ++i)
				glutTimerFunc(100, Fly, i);
			break;
		}
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, shape_count * 12 * sizeof(GLfloat), point, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, shape_count * 12 * sizeof(GLfloat), RGB, GL_STATIC_DRAW);
		glutPostRedisplay();
		}
	}
}

void Fly(int value)
{
	GLfloat gravity = -0.2f;	
	GLfloat left_gravity = 0.007f;

	// gravity += 0.005;
	point[value][1] -= 0.005f * gravity;
	point[value][4] -= 0.005f * gravity;
	point[value][7] -= 0.005f * gravity;
	point[value][10] -= 0.005f* gravity;

	point[value][0] -= left_gravity;
	point[value][3] -= left_gravity;
	point[value][6] -= left_gravity;
	point[value][9] -= left_gravity;
	// 화면 다시 그리기 요청
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, shape_count * 12 * sizeof(GLfloat), point, GL_DYNAMIC_DRAW);
	glutPostRedisplay();

	// 일정 간격으로 Fly 함수를 반복 호출
	glutTimerFunc(10, Fly, value);
}

