// pyram.c

/*

[ ] Revise generate_points function (there's an error)
[ ] Fix get_char function (The arrangement of points is not symmetrical between face types)

*/

#include <math.h>
#include <signal.h> // for Ctrl+C
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h> // for usleep()

/* Definitions and Constants */
#define WIDTH 170
#define HEIGHT 40
#define PYRAM_TRIANG_FACES 4 // there's 4 triangular faces and 1 square.
#define PYRAM_VETICES 5
#define PI 3.14159

/* Structures */
typedef struct {
	double x, y, z;
} Point;

typedef struct {
	double* data;
	int rows, cols;
} Matrix;

const double near_dst = 40.0f;
const double far_dst = 220.0f;
const double pyram_dst = 100.0f;
const double pyram_width = 50.0f;
const double pyram_edge = 2.0f * pyram_width;
const double pyram_height = 1.15f * pyram_width;

int num_points = 50;
int total_points;

/* Buffers for screen display */
char face_chars[] = {'@', '#', '$', '?', '+', ':'};
double z_buff[HEIGHT][WIDTH];
char screen_buffer[HEIGHT][WIDTH];
Point* pyram_points;

/* Function Prototypes */
void handle_sigint(int sig);
void calculate_globals(void);
void reset_buffs(void);
char get_char(int point_idx);
double mod(double a, double b);
Point* project_xyz(Point* p, Point* center_point);
Point* rotate_XYZ(Point* p, Point* center, double theta_x, double theta_y,
				  double theta_z);
void translate_xyz(Point* p, double tx, double ty, double tz);
Point* generate_pyram_points(Point* vertices,
							 int triang_faces[PYRAM_TRIANG_FACES][3],
							 int base_fase[4], int num_points);
void free_matrix(Matrix* m);
Matrix* new_matrix(int rows, int cols);
Matrix* mat_mul(Matrix* a, Matrix* b);
Matrix* rotate_X(double theta_x);
Matrix* rotate_Y(double theta_y);
Matrix* rotate_Z(double theta_z);

/* Main Function */
int main(void) {
	signal(SIGINT, handle_sigint); // Register the signal handler for SIGINT
	calculate_globals();
	reset_buffs();

	/* Define the pyram vertices */
	Point vertices[PYRAM_VETICES] = {
		{-pyram_width / 2, pyram_height / 2, -pyram_width / 2},
		{pyram_width / 2, pyram_height / 2, -pyram_width / 2},
		{pyram_width / 2, pyram_height / 2, pyram_width / 2},
		{-pyram_width / 2, pyram_height / 2, pyram_width / 2},
		{0, -pyram_height / 2, 0}};

	/* Translate the pyram away from the camera */
	Point center_point = {0, 0, pyram_dst};
	Point cam_point = {0, 0, 0};

	for (int i = 0; i < PYRAM_VETICES; i++) vertices[i].z += pyram_dst;

	/* Define the pyram faces (vertex indices) */
	int triangular_faces[PYRAM_TRIANG_FACES][3] = {
		{0, 1, 4},
		{1, 2, 4},
		{2, 3, 4},
		{3, 0, 4},
	};

	int base_face[4] = {0, 1, 2, 3};

	pyram_points = generate_pyram_points(vertices, triangular_faces, base_face,
										 num_points);

	/* Initial rotation angles */
	double A = 0, B = 0, C = 0;

	/* Main animation loop */
	while (1)
	{
		/* Rotate each pyram point */
		for (int i = 0; i < total_points; i++)
		{
			Point* rotated =
				rotate_XYZ(&pyram_points[i], &center_point, A, B, C);
			pyram_points[i] = *rotated;
			free(rotated);
		}

		/* Clear the screen and reset buffers */
		printf("\033[2J");
		reset_buffs();

		/* Project points and update depth and screen buffers */
		for (int i = 0; i < total_points; i++)
		{
			/* Map projected coordinates to screen indices */
			Point* proj = project_xyz(&pyram_points[i], &cam_point);

			int screen_x = (int) (proj->x) + WIDTH / 2;
			int screen_y = (int) (proj->y) + HEIGHT / 2;

			if (screen_x >= 0 && screen_x < WIDTH && screen_y >= 0 &&
				screen_y < HEIGHT)
			{
				/* Update if the point is closer (smaller z value) */
				if (proj->z < z_buff[screen_y][screen_x])
				{
					z_buff[screen_y][screen_x] = proj->z;
					screen_buffer[screen_y][screen_x] = get_char(i);
				}
			}
			free(proj);
		}

		/* Display the screen buffer */
		for (int i = 0; i < HEIGHT; i++)
		{
			for (int j = 0; j < WIDTH; j++) putchar(screen_buffer[i][j]);
			putchar('\n');
		}

		/* Increment rotation angles */
		double inc = 0.001;

		/* Increment angles avoiding precision overflow */
		/*A = mod(A + inc, 2 * PI);*/
		B = mod(B + inc, 2 * PI);
		/*C = mod(C + inc, 2 * PI);*/

		usleep(80000);
	}

	free(pyram_points);

	return 0;
}


void handle_sigint(int sig) {
	printf("\nExiting...\n");

	free(pyram_points);

	exit(0);
}

void calculate_globals(void) {
	total_points = num_points * num_points * (PYRAM_TRIANG_FACES + 1);
}

/* Returns a character based on the point index */
char get_char(int point_idx) {
	int num_chars = sizeof(face_chars) / sizeof(char);

	int index =
		(int) (((double) point_idx / (double) total_points) * num_chars);

	return face_chars[index];
}

/* Resets the depth and screen buffers */
void reset_buffs(void) {
	for (int i = 0; i < HEIGHT; i++)
	{
		for (int j = 0; j < WIDTH; j++)
		{
			z_buff[i][j] = far_dst;
			screen_buffer[i][j] = ' ';
		}
	}
}

/* Computes the modulus operator between two numbers */
double mod(double a, double b) { return a - b * (int) (a / b); }

/* Creates and returns a new matrix with given dimensions */
Matrix* new_matrix(int rows, int cols) {
	Matrix* m = malloc(sizeof(Matrix));

	m->rows = rows;
	m->cols = cols;
	m->data = calloc(rows * cols, sizeof(double));

	return m;
}

/* Frees the memory used by a matrix */
void free_matrix(Matrix* m) {
	if (m)
	{
		free(m->data);
		free(m);
	}
}

/* Matrix multiplication: result = a * b */
Matrix* mat_mul(Matrix* a, Matrix* b) {
	if (a->cols != b->rows) return NULL;
	Matrix* r = new_matrix(a->rows, b->cols);
	for (int i = 0; i < a->rows; i++)
	{
		for (int j = 0; j < b->cols; j++)
		{
			double sum = 0;
			for (int k = 0; k < a->cols; k++)
				sum += a->data[i * a->cols + k] * b->data[k * b->cols + j];
			r->data[i * r->cols + j] = sum;
		}
	}
	return r;
}

/* Rotation around the X axis */
Matrix* rotate_X(double theta) {
	Matrix* R = new_matrix(3, 3);

	double c = cos(theta), s = sin(theta);

	R->data[0] = 1;
	R->data[1] = 0;
	R->data[2] = 0;

	R->data[3] = 0;
	R->data[4] = c;
	R->data[5] = -s;

	R->data[6] = 0;
	R->data[7] = s;
	R->data[8] = c;

	return R;
}

/* Rotation around the Y axis */
Matrix* rotate_Y(double theta) {
	Matrix* R = new_matrix(3, 3);

	double c = cos(theta), s = sin(theta);

	R->data[0] = c;
	R->data[1] = 0;
	R->data[2] = s;

	R->data[3] = 0;
	R->data[4] = 1;
	R->data[5] = 0;

	R->data[6] = -s;
	R->data[7] = 0;
	R->data[8] = c;

	return R;
}

/* Rotation around the Z axis */
Matrix* rotate_Z(double theta) {
	Matrix* R = new_matrix(3, 3);

	double c = cos(theta), s = sin(theta);

	R->data[0] = c;
	R->data[1] = -s;
	R->data[2] = 0;

	R->data[3] = s;
	R->data[4] = c;
	R->data[5] = 0;

	R->data[6] = 0;
	R->data[7] = 0;
	R->data[8] = 1;

	return R;
}

/* Applies full rotation (X, Y, Z) to a point relative to a center */
Point* rotate_XYZ(Point* p, Point* center, double theta_x, double theta_y,
				  double theta_z) {
	double rx = p->x - center->x;
	double ry = p->y - center->y;
	double rz = p->z - center->z;

	Matrix* vec = new_matrix(3, 1);
	
    vec->data[0] = rx;
	vec->data[1] = ry;
	vec->data[2] = rz;

	Matrix* Rx = rotate_X(theta_x);
	Matrix* Ry = rotate_Y(theta_y);
	Matrix* Rz = rotate_Z(theta_z);

	Matrix* RyRz = mat_mul(Ry, Rz);
	Matrix* R = mat_mul(Rx, RyRz);
	Matrix* result = mat_mul(R, vec);

	Point* rotated = (Point*) malloc(sizeof(Point));
	
    rotated->x = result->data[0] + center->x;
	rotated->y = result->data[1] + center->y;
	rotated->z = result->data[2] + center->z;

	free_matrix(Rx);
	free_matrix(Ry);
	free_matrix(Rz);
	free_matrix(RyRz);
	free_matrix(R);
	free_matrix(vec);
	free_matrix(result);

	return rotated;
}

/* Translates a point in space */
void translate_xyz(Point* p, double tx, double ty, double tz) {
	p->x += tx;
	p->y += ty;
	p->z += tz;
}

/* Generates points for the pyram faces using interpolation */
Point* generate_pyram_points(Point* vertices,
							 int triang_faces[PYRAM_TRIANG_FACES][3],
							 int base_face[4], int num_points) {
	Point* points = malloc(sizeof(Point) * total_points);
	int point_idx = 0;

	// Triangular faces
	for (int f = 0; f < PYRAM_TRIANG_FACES; f++)
	{
		Point v0 = vertices[triang_faces[f][0]];
		Point v1 = vertices[triang_faces[f][1]];
		Point v2 = vertices[triang_faces[f][2]];

		for (int i = 0; i < num_points; i++)
		{
			for (int j = 0; j < num_points - i; j++)
			{
				double a = (double) i / num_points;
				double b = (double) j / num_points;
				double c = 1.0 - a - b;

				points[point_idx].x = a * v0.x + b * v1.x + c * v2.x;
				points[point_idx].y = a * v0.y + b * v1.y + c * v2.y;
				points[point_idx].z = a * v0.z + b * v1.z + c * v2.z;

				point_idx++;
			}
		}
	}

	// Base face
	Point v0 = vertices[base_face[0]];
	Point v1 = vertices[base_face[1]];
	Point v2 = vertices[base_face[2]];
	Point v3 = vertices[base_face[3]];

	for (int i = 0; i < num_points; i++)
	{
		for (int j = 0; j < num_points; j++)
		{
			double a = (double) i / num_points;
			double b = (double) j / num_points;

			Point p1 = {v0.x + a * (v1.x - v0.x), v0.y + a * (v1.y - v0.y),
						v0.z + a * (v1.z - v0.z)};
			Point p2 = {v3.x + a * (v2.x - v3.x), v3.y + a * (v2.y - v3.y),
						v3.z + a * (v2.z - v3.z)};

			points[point_idx].x = p1.x + b * (p2.x - p1.x);
			points[point_idx].y = p1.y + b * (p2.y - p1.y);
			points[point_idx].z = p1.z + b * (p2.z - p1.z);

			point_idx++;
		}
	}

	return points;
}

/* Projects a point using homogeneous coordinates */
Point* project_xyz(Point* p, Point* center_point) {
	/* Create a homogeneous vector: (x - cx, y - cy, z - cz, 1) */
	double vec[4] = {p->x - center_point->x, p->y - center_point->y,
					 p->z - center_point->z, 1.0};

	Matrix* vec_m = new_matrix(4, 1);

	for (int i = 0; i < 4; i++) vec_m->data[i] = vec[i];

	/* Define the projection matrix */
	Matrix* proj = new_matrix(4, 4);

	proj->data[0] = near_dst;
	proj->data[1] = 0;
	proj->data[2] = 0;
	proj->data[3] = 0;

	proj->data[4] = 0;
	proj->data[5] = near_dst;
	proj->data[6] = 0;
	proj->data[7] = 0;

	proj->data[8] = 0;
	proj->data[9] = 0;
	proj->data[10] = far_dst + near_dst;
	proj->data[11] = -far_dst * near_dst;

	proj->data[12] = 0;
	proj->data[13] = 0;
	proj->data[14] = 1;
	proj->data[15] = 0;

	Matrix* result = mat_mul(proj, vec_m);
	double w = result->data[3];

	/* Dehomogenize to get the projected point */
	Point* projected = malloc(sizeof(Point));

	projected->x = result->data[0] / w;
	projected->y = result->data[1] / w;
	projected->z = result->data[2] / w;

	free_matrix(vec_m);
	free_matrix(proj);
	free_matrix(result);

	return projected;
}
