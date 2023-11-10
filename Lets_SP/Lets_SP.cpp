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
void MouseMotion(int, int);
void Fly(int);
void Create();
void UnderboxMovement(int);
bool CrossCheck();


void convertCoordinate(int x, int y, double& convertedX, double& convertedY) {
	convertedX = (2.0 * x / MAXX) - 1.0;
	convertedY = 1.0 - (2.0 * y / MAXY);
}


std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<> createcoord_x(1.3, 1.5); // 1.3, 1.5
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

bool click;
GLuint vbo_cutting_line[2];
GLfloat cutting_line[2][3] = {
	{5, 5, 0},
	{5, 5, 0},
};
GLfloat cutting_line_RGB[2][3];
GLfloat remember_start[2];
GLfloat remember_end[2];

GLuint vbo_triangle[2];
GLfloat triangle[3][3] = {
	{3,3,0},
	{3,3,0},
	{3,3,0}
};
GLfloat triangle_RGB[3][3];

GLuint vbo_square[2];
GLfloat square[4][3];
GLfloat square_RGB[4][3];

GLfloat point[100][12];
GLfloat RGB[100][12];

int figure_type{ 0 };
GLfloat figure[10][12];

bool isIntersect = false;

int shape_mode;
int shape_count;
int shape_type[10];


struct Point {
	double x, y;
};

struct Segment {
	Point start, end;
};

// 방향을 확인하는 함수
int orientation(Point p, Point q, Point r) {
	double val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);
	if (val == 0) return 0;
	return (val > 0) ? 1 : 2;
}

// 선분 상에 있는지 여부를 확인하는 함수
bool onSegment(Point p, Point q, Point r) {
	if (q.x <= std::max(p.x, r.x) && q.x >= std::min(p.x, r.x) &&
		q.y <= std::max(p.y, r.y) && q.y >= std::min(p.y, r.y))
		return true;
	return false;
}

// 두 선분이 교차하는지 판단하는 함수
bool doIntersect(Segment seg1, Segment seg2) {
	int o1 = orientation(seg1.start, seg1.end, seg2.start);
	int o2 = orientation(seg1.start, seg1.end, seg2.end);
	int o3 = orientation(seg2.start, seg2.end, seg1.start);
	int o4 = orientation(seg2.start, seg2.end, seg1.end);

	// 서로 다른 방향이면 교차함
	if (o1 != o2 && o3 != o4)
		return true;

	// 특수한 경우 처리 (일직선 상에 있거나 일치하는 경우)
	if (o1 == 0 && onSegment(seg1.start, seg2.start, seg1.end)) return true;
	if (o2 == 0 && onSegment(seg1.start, seg2.end, seg1.end)) return true;
	if (o3 == 0 && onSegment(seg2.start, seg1.start, seg2.end)) return true;
	if (o4 == 0 && onSegment(seg2.start, seg1.end, seg2.end)) return true;

	return false;
}

bool doesSegmentIntersectWithTriangle(Segment seg, GLfloat triangle[3][3]) {
	for (int i = 0; i < 3; ++i) {
		Segment triangleSegment = { {triangle[i][0], triangle[i][1]}, {triangle[(i + 1) % 3][0], triangle[(i + 1) % 3][1]} };

		if (doIntersect(seg, triangleSegment))
			return true;
	}

	return false;
}

bool doesSegmentIntersectWithRectangle(Segment seg, GLfloat square[4][3]) {
	for (int i = 0; i < 4; ++i) {
		Segment squareSegment = { {square[i][0], square[i][1]}, {square[(i + 1) % 4][0], square[(i + 1) % 4][1]} };

		if (doIntersect(seg, squareSegment))
			return true;
	}

	return false;
}

bool CrossCheckRectangle() {
	Segment seg1 = { {remember_start[0], remember_start[1]}, {remember_end[0], remember_end[1]} };

	for (int i = 0; i < 4; ++i) {
		Segment seg2 = { {square[i][0], square[i][1]}, {square[(i + 1) % 4][0], square[(i + 1) % 4][1]} };

		if (doesSegmentIntersectWithRectangle(seg1, square)) {
			std::cout << "선분과 사각형이 교차합니다." << std::endl;
			return true;
		}
	}

	std::cout << "선분과 사각형이 교차하지 않습니다." << std::endl;

	return false;
}


bool CrossCheckTriangle() {
	Segment seg1 = { {remember_start[0], remember_start[1]}, {remember_end[0], remember_end[1]} };

	for (int i = 0; i < 3; ++i) {
		Segment seg2 = { {triangle[i][0], triangle[i][1]}, {triangle[(i + 1) % 3][0], triangle[(i + 1) % 3][1]} };

		if (doesSegmentIntersectWithTriangle(seg1, triangle)) {
			std::cout << "선분과 삼각형이 교차합니다." << std::endl;
			return true;
		}
	}

	std::cout << "선분과 삼각형이 교차하지 않습니다." << std::endl;

	return false;
}

void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	// 윈도우 생성
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(500, 500);
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

	InitBuffer();
	glutDisplayFunc(drawScene); // 장면을 다시 그리는데 필요한 루틴들은 모두 이 함수 안에 넣어둔다.
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(Mouse);
	glutMotionFunc(MouseMotion);
	glutTimerFunc(10, UnderboxMovement, 0);
	Create();
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


	// Draw Flying_Figure (Triangle)
	glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle[0]);
	glVertexAttribPointer(PosLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(PosLocation);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle[1]);
	glVertexAttribPointer(ColorLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(ColorLocation);
	glDrawArrays(GL_TRIANGLES, 0, 3);


	// Draw Flying_Figure (Square)
	glBindBuffer(GL_ARRAY_BUFFER, vbo_square[0]);
	glVertexAttribPointer(PosLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(PosLocation);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_square[1]);
	glVertexAttribPointer(ColorLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(ColorLocation);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


	// Drag line check
	if (click) {
		glBindBuffer(GL_ARRAY_BUFFER, vbo_cutting_line[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cutting_line), cutting_line, GL_STATIC_DRAW);
		glVertexAttribPointer(PosLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_cutting_line[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cutting_line_RGB), cutting_line_RGB, GL_STATIC_DRAW);
		glVertexAttribPointer(ColorLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
		glLineWidth(3);
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
	glBufferData(GL_ARRAY_BUFFER, sizeof(underbox), underbox, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_underbox[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(underbox_RGB), underbox_RGB, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_RGB), triangle_RGB, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_square[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(square), square, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_square[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(square_RGB), square_RGB, GL_DYNAMIC_DRAW);
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

	if (state == GLUT_DOWN) {
		if (button == GLUT_LEFT_BUTTON) {
			click = true;

			cutting_line[0][0] = convertedX;
			cutting_line[0][1] = convertedY;
			cutting_line[0][2] = 0.f;
			cutting_line[1][0] = convertedX;
			cutting_line[1][1] = convertedY;
			cutting_line[1][2] = 0.f;

			remember_start[0] = cutting_line[0][0];
			remember_start[1] = cutting_line[0][1];
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

			remember_end[0] = cutting_line[1][0];
			remember_end[1] = cutting_line[1][1];

			if (shape_mode == 1)
				CrossCheckTriangle();
			else
				CrossCheckRectangle();

			//std::cout << triangle[0][0] << '\n';
			//std::cout << triangle[0][1] << '\n';
			//std::cout << triangle[1][0] << '\n';
			//std::cout << triangle[1][1] << '\n';
			//std::cout << triangle[2][0] << '\n';
			//std::cout << triangle[2][1] << '\n';

			/*std::cout << "start0 : " << remember_start[0] << '\n';
			std::cout << "start1 : " << remember_start[1] << '\n';
			std::cout << "end0 : " << remember_end[0] << '\n';
			std::cout << "end1 : " << remember_end[1] << "\n\n\n";*/

		}
	}

	glutPostRedisplay();
}

void MouseMotion(int x, int y)
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
	case 'q':
	case 'Q':
		exit(0);
		break;
	}

	glutPostRedisplay();
}

bool create_flag = true;
void Create()
{
	std::uniform_int_distribution<> mode(1, 2);

	int flag = mode(gen);
	// std::cout << "key : " << flag << '\n';

	switch (flag) {
	case 1:
	{
		shape_mode = 1;

		GLfloat random_point_x = createcoord_x(gen);
		GLfloat random_point_y = createcoord_y(gen);
		GLfloat random_point_4 = createcoord_2(gen);
		triangle[0][0] = random_point_x - random_point_4;
		triangle[0][1] = random_point_y - random_point_4;
		triangle[0][2] = 0.0f;
		triangle[1][0] = random_point_x + random_point_4;
		triangle[1][1] = random_point_y - random_point_4;
		triangle[1][2] = 0.0f;
		triangle[2][0] = random_point_x;
		triangle[2][1] = random_point_y + random_point_4;
		triangle[2][2] = 0.0f;

		triangle_RGB[0][0] = color(gen);
		triangle_RGB[0][1] = color(gen);
		triangle_RGB[0][2] = color(gen);
		triangle_RGB[1][0] = color(gen);
		triangle_RGB[1][1] = color(gen);
		triangle_RGB[1][2] = color(gen);
		triangle_RGB[2][0] = color(gen);
		triangle_RGB[2][1] = color(gen);
		triangle_RGB[2][2] = color(gen);


		glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle[0]);
		glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), triangle, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle[1]);
		glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), triangle_RGB, GL_STATIC_DRAW);
		if (create_flag) {
			glutTimerFunc(100, Fly, 0);
			create_flag = false;
		}
		break;
	}
	case 2:
	{
		shape_mode = 2;

		GLfloat random_point_x = createcoord_x(gen);
		GLfloat random_point_y = createcoord_y(gen);
		GLfloat random_point_2 = createcoord_2(gen);
		square[0][0] = random_point_x - createcoord_2(gen);
		square[0][1] = random_point_y - createcoord_2(gen);
		square[0][2] = 0.0f;
		square[1][0] = random_point_x - createcoord_2(gen);
		square[1][1] = random_point_y + createcoord_2(gen);
		square[1][2] = 0.0f;
		square[2][0] = random_point_x + createcoord_2(gen);
		square[2][1] = random_point_y - createcoord_2(gen);
		square[2][2] = 0.0f;
		square[3][0] = random_point_x + createcoord_2(gen);
		square[3][1] = random_point_y + createcoord_2(gen);
		square[3][2] = 0.0f;

		square_RGB[0][0] = color(gen);
		square_RGB[0][1] = color(gen);
		square_RGB[0][2] = color(gen);
		square_RGB[1][0] = color(gen);
		square_RGB[1][1] = color(gen);
		square_RGB[1][2] = color(gen);
		square_RGB[2][0] = color(gen);
		square_RGB[2][1] = color(gen);
		square_RGB[2][2] = color(gen);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_square[0]);
		glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), square, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_square[1]);
		glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), square_RGB, GL_STATIC_DRAW);
		if (create_flag) {
			glutTimerFunc(100, Fly, 0);
			create_flag = false;
		}
		break;
	}

	glutPostRedisplay();
	}
}

void UnderboxMovement(int value)
{
	GLfloat gravity = 0.007f;
	static bool moving_direction = true;	// true : move left, false : move right


	if (moving_direction == true) {
		underbox[0][0] -= gravity;
		underbox[1][0] -= gravity;
		underbox[2][0] -= gravity;
		underbox[3][0] -= gravity;
		if (underbox[0][0] < -1.0f)
			moving_direction = false;
	}
	if (moving_direction == false) {
		underbox[0][0] += gravity;
		underbox[1][0] += gravity;
		underbox[2][0] += gravity;
		underbox[3][0] += gravity;
		if (underbox[2][0] > 1.0f)
			moving_direction = true;
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo_underbox[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(underbox), underbox, GL_DYNAMIC_DRAW);
	glutPostRedisplay();

	glutTimerFunc(10, UnderboxMovement, value);
}


void Fly(int value)
{
	// std::cout << shape_mode << '\n';
	GLfloat gravity = -0.002f;
	GLfloat left_gravity = 0.007f;

	if (shape_mode == 1) {
		triangle[0][0] -= left_gravity;
		triangle[0][1] -= gravity;
		triangle[0][2] = 0.0f;
		triangle[1][0] -= left_gravity;
		triangle[1][1] -= gravity;
		triangle[1][2] = 0.0f;
		triangle[2][0] -= left_gravity;
		triangle[2][1] -= gravity;
		triangle[2][2] = 0.0f;

		glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle[0]);
		glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), triangle, GL_DYNAMIC_DRAW);
	}
	else if (shape_mode == 2) {
		square[0][0] -= left_gravity;
		square[0][1] -= gravity;
		square[0][2] = 0.0f;
		square[1][0] -= left_gravity;
		square[1][1] -= gravity;
		square[1][2] = 0.0f;
		square[2][0] -= left_gravity;
		square[2][1] -= gravity;
		square[2][2] = 0.0f;
		square[3][0] -= left_gravity;
		square[3][1] -= gravity;
		square[3][2] = 0.0f;

		glBindBuffer(GL_ARRAY_BUFFER, vbo_square[0]);
		glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), square, GL_DYNAMIC_DRAW);
	}

	if (triangle[0][0] < -1.5 && shape_mode == 1) {
		Create();
	}
	else if (square[0][0] < -1.5 && shape_mode == 2) {
		Create();
	}


	// 화면 다시 그리기 요청
	glutPostRedisplay();

	// 일정 간격으로 Fly 함수를 반복 호출
	glutTimerFunc(10, Fly, 0);
}