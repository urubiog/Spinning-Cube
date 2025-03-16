// cube.c

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // for usleep()

/* Definitions and Constants */
#define WIDTH 170
#define HEIGHT 56
#define CUBE_FACES 6
#define CUBE_VERTICES 8
#define PI 3.14159

const double near_dst = 40.0f;
const double far_dst = 120.0f;
const double cube_dst = 75.0f;
const double cube_width = 35.0f;

/* Buffers for screen display */
char face_chars[] = {'@', '#', '$', '?', '+', ':'};
double z_buff[HEIGHT][WIDTH];
char screen_buffer[HEIGHT][WIDTH];

/* Structures */
typedef struct {
	double x, y, z;
} Point;

typedef struct {
	double* data;
	int rows, cols;
} Matrix;

/* Function Prototypes */
void reset_buffs(void);
char get_char(int point_idx, int total_points);
double mod(double a, double b);
Point* project_xyz(Point* p, Point* center_point);
Point* rotate_XYZ(Point* p, double theta_x, double theta_y, double theta_z, Point* center_point);
void translate_xyz(Point* p, double tx, double ty, double tz);
Point* generate_cube_points(Point* vertices, int faces[CUBE_FACES][4], int num_points);
void free_matrix(Matrix* m);
Matrix* new_matrix(int rows, int cols);
Matrix* mat_mul(Matrix* a, Matrix* b);
Matrix* rotate_X(double theta_x);
Matrix* rotate_Y(double theta_y);
Matrix* rotate_Z(double theta_z);

/* Main Function */
int main(void) {
	reset_buffs();

	/* Define the cube vertices */
	Point vertices[CUBE_VERTICES] = {
		{-cube_width / 2, -cube_width / 2, -cube_width / 2},
		{cube_width / 2, -cube_width / 2, -cube_width / 2},
		{cube_width / 2, cube_width / 2, -cube_width / 2},
		{-cube_width / 2, cube_width / 2, -cube_width / 2},
		{-cube_width / 2, -cube_width / 2, cube_width / 2},
		{cube_width / 2, -cube_width / 2, cube_width / 2},
		{cube_width / 2, cube_width / 2, cube_width / 2},
		{-cube_width / 2, cube_width / 2, cube_width / 2}};

	/* Translate the cube away from the camera */
	Point center_point = {0, 0, cube_dst};
	Point cam_point = {0, 0, 0};

	for (int i = 0; i < CUBE_VERTICES; i++) vertices[i].z += cube_dst;

	/* Define the cube faces (vertex indices) */
	int faces[CUBE_FACES][4] = {
        {0, 1, 2, 3}, 
        {4, 5, 6, 7}, 
        {0, 3, 7, 4},
        {1, 2, 6, 5}, 
        {0, 1, 5, 4}, 
        {3, 2, 6, 7}
    };

	/* Generate the points for each face */
	int num_points = 50;
	int total_points = num_points * num_points * CUBE_FACES;

	Point* cube_points = generate_cube_points(vertices, faces, num_points);

	/* Initial rotation angles */
	double A = 0, B = 0, C = 0;

	/* Main animation loop */
	while (1)
	{
		/* Rotate each cube point */
		for (int i = 0; i < total_points; i++)
		{
			Point* rotated = rotate_XYZ(&cube_points[i], A, B, C, &center_point);
			cube_points[i] = *rotated;
			free(rotated);
		}

		/* Clear the screen and reset buffers */
		printf("\033[2J");
		reset_buffs();

		/* Project points and update depth and screen buffers */
		for (int i = 0; i < total_points; i++)
		{
            /* Map projected coordinates to screen indices */
			Point* proj = project_xyz(&cube_points[i], &cam_point);

			int screen_x = (int) (proj->x) + WIDTH / 2;
			int screen_y = (int) (proj->y) + HEIGHT / 2;

			if (screen_x >= 0 && screen_x < WIDTH && screen_y >= 0 &&
				screen_y < HEIGHT)
			{
				/* Update if the point is closer (smaller z value) */
				if (proj->z < z_buff[screen_y][screen_x])
				{
					z_buff[screen_y][screen_x] = proj->z;
					screen_buffer[screen_y][screen_x] =
						get_char(i, total_points);
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
		A = mod(A + inc, 2 * PI);
		B = mod(B + inc, 2 * PI);
		C = mod(C + inc, 2 * PI);

        usleep(40000);
	}

	free(cube_points);
	return 0;
}

/* Returns a character based on the point index */
char get_char(int point_idx, int total_points) {
	int num_chars = sizeof(face_chars) / sizeof(char);
	int index =
		(int) (((double) point_idx / (double) total_points) * num_chars);
	if (index >= num_chars) index = num_chars - 1;
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
double mod(double a, double b) {
    return a - b * (int)(a / b);
}

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
Matrix* rotate_X(double theta_x) {
	Matrix* m = new_matrix(3, 3);
	m->data[0] = 1;
	m->data[1] = 0;
	m->data[2] = 0;
	m->data[3] = 0;
	m->data[4] = cos(theta_x);
	m->data[5] = -sin(theta_x);
	m->data[6] = 0;
	m->data[7] = sin(theta_x);
	m->data[8] = cos(theta_x);
	return m;
}

/* Rotation around the Y axis */
Matrix* rotate_Y(double theta_y) {
	Matrix* m = new_matrix(3, 3);
	m->data[0] = cos(theta_y);
	m->data[1] = 0;
	m->data[2] = sin(theta_y);
	m->data[3] = 0;
	m->data[4] = 1;
	m->data[5] = 0;
	m->data[6] = -sin(theta_y);
	m->data[7] = 0;
	m->data[8] = cos(theta_y);
	return m;
}

/* Rotation around the Z axis */
Matrix* rotate_Z(double theta_z) {
	Matrix* m = new_matrix(3, 3);
	m->data[0] = cos(theta_z);
	m->data[1] = -sin(theta_z);
	m->data[2] = 0;
	m->data[3] = sin(theta_z);
	m->data[4] = cos(theta_z);
	m->data[5] = 0;
	m->data[6] = 0;
	m->data[7] = 0;
	m->data[8] = 1;
	return m;
}

/* Applies full rotation (X, Y, Z) to a point relative to a center */
Point* rotate_XYZ(Point* p, double theta_x, double theta_y, double theta_z,
				  Point* center_point) {
	/* Calculate relative coordinates */
	double rx = p->x - center_point->x;
	double ry = p->y - center_point->y;
	double rz = p->z - center_point->z;
	Matrix* vec = new_matrix(3, 1);
	vec->data[0] = rx;
	vec->data[1] = ry;
	vec->data[2] = rz;

	/* Create rotation matrices */
	Matrix* Rx = rotate_X(theta_x);
	Matrix* Ry = rotate_Y(theta_y);
	Matrix* Rz = rotate_Z(theta_z);

	/* Compose rotations: R = Rx * (Ry * Rz) */
	Matrix* RyRz = mat_mul(Ry, Rz);
	Matrix* R = mat_mul(Rx, RyRz);

	/* Multiply rotation matrix by the vector: result = R * vec */
	Matrix* result = mat_mul(R, vec);

	/* Reconstruct the absolute point */
	Point* rotated = malloc(sizeof(Point));
	rotated->x = result->data[0] + center_point->x;
	rotated->y = result->data[1] + center_point->y;
	rotated->z = result->data[2] + center_point->z;

	/* Free temporary matrices */
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

/* Generates points for the cube faces using interpolation */
Point* generate_cube_points(Point* vertices, int faces[CUBE_FACES][4],
							int num_points) {
	int total_points = num_points * num_points * CUBE_FACES;
	Point* points = malloc(sizeof(Point) * total_points);
	int point_idx = 0;
	for (int face_idx = 0; face_idx < CUBE_FACES; face_idx++)
	{
		int v0_idx = faces[face_idx][0];
		int v1_idx = faces[face_idx][1];
		int v2_idx = faces[face_idx][2];
		int v3_idx = faces[face_idx][3];

		Point v0 = vertices[v0_idx];
		Point v1 = vertices[v1_idx];
		Point v2 = vertices[v2_idx];
		Point v3 = vertices[v3_idx];

		for (int i = 0; i < num_points; i++)
		{
			for (int j = 0; j < num_points; j++)
			{
				double u = (double) i / (num_points - 1);
				double v = (double) j / (num_points - 1);
				points[point_idx].x = (1 - u) * (1 - v) * v0.x +
									  u * (1 - v) * v1.x + u * v * v2.x +
									  (1 - u) * v * v3.x;
				points[point_idx].y = (1 - u) * (1 - v) * v0.y +
									  u * (1 - v) * v1.y + u * v * v2.y +
									  (1 - u) * v * v3.y;
				points[point_idx].z = (1 - u) * (1 - v) * v0.z +
									  u * (1 - v) * v1.z + u * v * v2.z +
									  (1 - u) * v * v3.z;
				point_idx++;
			}
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
	/* First row */
	proj->data[0] = near_dst;
	proj->data[1] = 0;
	proj->data[2] = 0;
	proj->data[3] = 0;
	/* Second row */
	proj->data[4] = 0;
	proj->data[5] = near_dst;
	proj->data[6] = 0;
	proj->data[7] = 0;
	/* Third row */
	proj->data[8] = 0;
	proj->data[9] = 0;
	proj->data[10] = far_dst + near_dst;
	proj->data[11] = -far_dst * near_dst;
	/* Fourth row */
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
