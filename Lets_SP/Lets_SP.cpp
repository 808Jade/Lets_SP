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
GLfloat line[2][6];
GLfloat line_RGB[2][6];

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


	{
		line[0][1] = 1;
		line[0][4] = -1;
		line[1][0] = 1;
		line[1][3] = -1;
		line_RGB[0][2] = 1;
		line_RGB[0][5] = 1;
		line_RGB[1][0] = 1;
		line_RGB[1][3] = 1;
	}
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(2, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, 4 * 9 * sizeof(GLfloat), figure, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, 4 * 9 * sizeof(GLfloat), RGB, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_line[0]);
	glGenBuffers(2, vbo_line);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_line[0]);
	glBufferData(GL_ARRAY_BUFFER, 2 * 6 * sizeof(GLfloat), line, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_line[1]);
	glBufferData(GL_ARRAY_BUFFER, 2 * 6 * sizeof(GLfloat), line_RGB, GL_STATIC_DRAW);

	glUseProgram(shaderProgramID);

	//--- 세이더 읽어와서 세이더 프로그램 만들기
	make_shaderProgram();

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


	glDisableVertexAttribArray(PosLocation); // Disable 필수!
	glDisableVertexAttribArray(ColorLocation);

	glutSwapBuffers(); // 화면에 출력하기
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

	glutPostRedisplay();
}

void Motion(int x, int y)
{
	GLdouble convertedX, convertedY;
	convertCoordinate(x, y, convertedX, convertedY);
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
	int key = mode(gen);
	std::cout << "key : " << key << '\n';
	if (shape_count < 10) {
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

		glutTimerFunc(1000, Create, value + 1);
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

