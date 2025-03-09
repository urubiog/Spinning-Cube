"""
Specification in Python.
"""

import numpy as np

WIDTH, HEIGHT = 170, 44
Near_dst = 40
Far_dst = 120
Cube_dst = 75
Cube_width = 35
face_chars = ["@", "#", "$", "?", "+", ":"]
    
z_buffer = np.full((HEIGHT, WIDTH), Far_dst, np.float64)
screen_buffer = np.full((HEIGHT, WIDTH), " ", dtype=str)

def project_xyz(x, y, z, center_point):
    x, y, z = x - center_point[0], y - center_point[1], z - center_point[2]
    
    Proj_m = np.array(
        [
            [Near_dst, 0, 0, 0],
            [0, Near_dst, 0, 0],
            [0, 0, Far_dst + Near_dst, -Far_dst * Near_dst],
            [0, 0, 1, 0],
        ]
    )

    proj = Proj_m.dot(np.array([x, y, z, 1]))
    proj /= proj[-1]
    proj = proj[:3]

    proj += center_point
    
    return proj

def rotate_X(theta_x):
    return np.array(
        [
            [1, 0, 0],
            [0, np.cos(theta_x), -np.sin(theta_x)],
            [0, np.sin(theta_x), np.cos(theta_x)],
        ]
    )

def rotate_Y(theta_y):
    return np.array(
        [
            [np.cos(theta_y), 0, np.sin(theta_y)],
            [0, 1, 0],
            [-np.sin(theta_y), 0, np.cos(theta_y)],
        ]
    )

def rotate_Z(theta_z):
    return np.array(
        [
            [np.cos(theta_z), -np.sin(theta_z), 0],
            [np.sin(theta_z), np.cos(theta_z), 0],
            [0, 0, 1],
        ]
    )

def rotate_xyz(point, theta_x, theta_y, theta_z, center_point=np.array([0, 0, 0])):
    relative_point = point - center_point

    R_total = rotate_Z(theta_z) @ rotate_Y(theta_y) @ rotate_X(theta_x)

    return R_total @ relative_point + center_point

def translate_xyz(x, y, z, tx, ty, tz):
    return np.array([x + tx, y + ty, z + tz])

vertices = [
    np.array([-Cube_width / 2, -Cube_width / 2, -Cube_width / 2]),
    np.array([Cube_width / 2, -Cube_width / 2, -Cube_width / 2]),
    np.array([Cube_width / 2, Cube_width / 2, -Cube_width / 2]),
    np.array([-Cube_width / 2, Cube_width / 2, -Cube_width / 2]),
    np.array([-Cube_width / 2, -Cube_width / 2, Cube_width / 2]),
    np.array([Cube_width / 2, -Cube_width / 2, Cube_width / 2]),
    np.array([Cube_width / 2, Cube_width / 2, Cube_width / 2]),
    np.array([-Cube_width / 2, Cube_width / 2, Cube_width / 2]),
]

center_point = np.array([0, 0, Cube_dst])
cam_point = np.zeros(3)

for i in range(len(vertices)):
    vertices[i] = translate_xyz(*vertices[i], *center_point)

faces = [
    [0, 1, 2, 3],
    [4, 5, 6, 7],
    [0, 3, 7, 4],
    [1, 2, 6, 5],
    [0, 1, 5, 4],
    [3, 2, 6, 7],
]

def generate_cube_points(vertices, faces, num_points):
    points = []
    face_indices = []
    
    for face_idx, face in enumerate(faces):
        v0, v1, v2, v3 = [vertices[i] for i in face]
        
        for i in range(num_points):
            for j in range(num_points):
                u, v = i / (num_points - 1), j / (num_points - 1)
                point = (
                    (1 - u) * (1 - v) * v0
                    + u * (1 - v) * v1
                    + (1 - u) * v * v3
                    + u * v * v2
                )
                points.append(point)
                face_indices.append(face_idx)

    return np.array(points), np.array(face_indices)

cube_points, face_indices = generate_cube_points(vertices, faces, num_points=50)

def main():
    A, B, C = 5, 5, 5
    
    while True:
        for j in range(len(cube_points)):
            cube_points[j] = rotate_xyz(cube_points[j], A, B, C, center_point)

        print("\033[2J")
        z_buffer.fill(Far_dst)
        screen_buffer.fill(" ")
        
        for i in range(len(cube_points)):
            proj = project_xyz(*cube_points[i], *cam_point)
            proj_x = int(proj[0]) + int(WIDTH / 2)
            proj_y = int(proj[1]) + int(HEIGHT / 2)

            if 0 <= proj_x < WIDTH and 0 <= proj_y < HEIGHT:
                if z_buffer[proj_y, proj_x] > proj[2]:
                    z_buffer[proj_y, proj_x] = proj[2]
                    screen_buffer[proj_y, proj_x] = face_chars[face_indices[i]]
            
        for i in range(HEIGHT):
            print("".join(screen_buffer[i]))
        
        A += 5e-4
        B += 1e-4
        C += 1e-4

if __name__ == "__main__":
    main()

