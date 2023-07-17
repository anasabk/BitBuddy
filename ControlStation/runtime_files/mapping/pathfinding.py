"""
1) Point Cloud to Grid Map
"""

import numpy as np
import cv2
import sys
import math
from transforms3d import quaternions

print("Converting point cloud to grid map...");
sys.stdout.flush()

def get_line_bresenham(start, end):
    """Bresenham's Line Algorithm
    Produces a list of tuples from start and end
    """
    # Setup initial conditions
    x1, y1 = start
    x2, y2 = end
    dx = x2 - x1
    dy = y2 - y1

    # Determine how steep the line is
    is_steep = abs(dy) > abs(dx)

    # Rotate line
    if is_steep:
        x1, y1 = y1, x1
        x2, y2 = y2, x2

    # Swap start and end points if necessary and store swap state
    swapped = False
    if x1 > x2:
        x1, x2 = x2, x1
        y1, y2 = y2, y1
        swapped = True

    # Recalculate differentials
    dx = x2 - x1
    dy = y2 - y1

    # Calculate error
    error = int(dx / 2.0)
    ystep = 1 if y1 < y2 else -1

    # Iterate over bounding box generating points between start and end
    y = y1
    points = []
    for x in range(x1, x2 + 1):
        coord = (y, x) if is_steep else (x, y)
        points.append(coord)
        error -= abs(dy)
        if error < 0:
            y += ystep
            error += dx

    # Reverse the list if the coordinates were swapped
    if swapped:
        points.reverse()
    return points


seq_name = 'bit_buddy'

# inverse of cell size
scale_factor = 25
resize_factor = 20
filter_ground_points = 0
load_counters = 0

point_cloud_fname = '{:s}_map_pts_and_keyframes.txt'.format(seq_name)
keyframe_trajectory_fname = 'KeyFrameTrajectory.txt'
visit_counter_fname = '{:s}_filtered_{:d}_scale_{:d}_visit_counter.txt'.format(
    seq_name, filter_ground_points, scale_factor)
occupied_counter_fname = '{:s}_filtered_{:d}_scale_{:d}_occupied_counter.txt'.format(
    seq_name, filter_ground_points, scale_factor)

print('seq_name: ', seq_name)
print('scale_factor: ', scale_factor)
print('resize_factor: ', resize_factor)
print('filter_ground_points: ', filter_ground_points)
sys.stdout.flush()

counters_loaded = False
if load_counters:
    try:
        print ('Loading counters...')
        sys.stdout.flush()
        visit_counter = np.loadtxt(visit_counter_fname)
        occupied_counter = np.loadtxt(occupied_counter_fname)
        grid_res = visit_counter.shape
        print ('grid_res: ', grid_res)
        counters_loaded = True
    except:
        print ('One or more counter files: {:s}, {:s} could not be found'.format(
            occupied_counter_fname, visit_counter_fname))
        counters_loaded = False
    finally:
        sys.stdout.flush()

if not counters_loaded:
    # read keyframes
    keyframe_trajectory_data = open(keyframe_trajectory_fname, 'r').readlines()
    keyframe_timestamps = []
    keyframe_locations = []
    keyframe_quaternions = []
    for line in keyframe_trajectory_data:
        line_tokens = line.strip().split()
        timestamp = float(line_tokens[0])
        keyframe_x = float(line_tokens[1]) * scale_factor
        keyframe_y = float(line_tokens[2]) * scale_factor
        keyframe_z = float(line_tokens[3]) * scale_factor
        keyframe_timestamps.append(timestamp)
        keyframe_locations.append([keyframe_x, keyframe_y, keyframe_z])
        keyframe_quaternions.append([float(line_tokens[4]), float(line_tokens[5]),
                                     float(line_tokens[6]), float(line_tokens[7])])

    keyframe_locations_dict = dict(zip(keyframe_timestamps, keyframe_locations))
    keyframe_quaternions_dict = dict(zip(keyframe_timestamps, keyframe_quaternions))
    keyframe_locations = np.array(keyframe_locations)
    keyframe_timestamps = np.array(keyframe_timestamps)
    n_keyframes = keyframe_locations.shape[0]

    # read point cloud
    point_cloud_data = open(point_cloud_fname, 'r').readlines()
    point_locations = []
    point_timestamps = []
    n_lines = len(point_cloud_data)
    n_points = 0
    is_ground_point = []
    for line in point_cloud_data:
        line_tokens = line.strip().split()
        point_x = float(line_tokens[0]) * scale_factor
        point_y = float(line_tokens[1]) * scale_factor
        point_z = float(line_tokens[2]) * scale_factor

        timestamps = []

        for i in range(3, len(line_tokens)):
            timestamps.append(float(line_tokens[i]))
        if len(timestamps) == 0:
            raise Exception('Point {:d} has no keyframes'.format(n_points))

        is_ground_point.append(False)

        if filter_ground_points:
            key_frame_id = 0
            keyframe_quaternion = None
            keyframe_location = None
            while key_frame_id < len(timestamps):
                try:
                    keyframe_quaternion = np.array(keyframe_quaternions_dict[timestamps[key_frame_id]])
                    keyframe_location = keyframe_locations_dict[timestamps[key_frame_id]]
                    break
                except KeyError:
                    key_frame_id += 1
            if keyframe_quaternion is None:
                raise Exception('No valid keyframes found for point {:d}'.format(n_points))
            # normalize quaternion
            keyframe_quaternion /= np.linalg.norm(keyframe_quaternion)
            keyframe_rotation = quaternions.quat2mat(keyframe_quaternion)
            keyframe_translation = np.matrix(keyframe_location).transpose()
            transform_mat = np.zeros([4, 4], dtype=np.float64)
            transform_mat[0:3, 0:3] = keyframe_rotation.transpose()
            transform_mat[3, 0:3] = (np.matrix(-keyframe_rotation.transpose()) * keyframe_translation).ravel()
            transform_mat[3, 3] = 1
            point_location = np.matrix([point_x, point_y, point_z, 1]).transpose()
            transformed_point_location = np.matrix(transform_mat) * point_location
            # homogeneous to non homogeneous coordinates
            transformed_point_height = transformed_point_location[1] / transformed_point_location[3]
            if transformed_point_height < 0:
                is_ground_point[-1] = True

        point_locations.append([point_x, point_y, point_z])
        point_timestamps.append(timestamps)
        n_points += 1

    point_locations = np.array(point_locations)
    print('n_keyframes: ', n_keyframes)
    print('n_points: ', n_points)
    if filter_ground_points:
        n_ground_points = np.count_nonzero(np.array(is_ground_point))
        print('n_ground_points: ', n_ground_points)

    sys.stdout.flush()

    # MAX DIST DEĞERİNİ AYARLAYARAK NE KADAR UZAKTAKİ NOKTALARIN SİLİNECEĞİNİ SEÇİN.
    max_dist = 10
    far_points = []
    pl_max_x = 0
    pl_max_z = 0
    pl_min_x = 0
    pl_min_z = 0
    
    np.savetxt("point_locations.txt", point_locations, fmt='%d')


    for i in range (len(point_locations)):
        cur_min_dist = 999999
        for j in range (i, len(point_locations)):
            if(i == j):
                continue

            dist = math.sqrt((point_locations[i][0] - point_locations[j][0])**2 + 
                             (point_locations[i][1] - point_locations[j][1])**2 + 
                             (point_locations[i][2] - point_locations[j][2])**2)
            

            if(dist < cur_min_dist):
                cur_min_dist = dist
        
        if(cur_min_dist > max_dist):
            far_points.append(i)

        else:
            if(point_locations[i][0] < pl_min_x):
                pl_min_x = point_locations[i][0]
            elif(point_locations[i][0] > pl_max_x):
                pl_max_x = point_locations[i][0]
            if(point_locations[i][2] < pl_min_z):
                pl_min_z = point_locations[i][2]
            elif(point_locations[i][2] > pl_max_z):
                pl_max_z = point_locations[i][2]


    print("pl_max_x: ", pl_max_x)
    print("pl_min_x: ", pl_min_x)
    print("pl_max_z: ", pl_max_z)
    print("pl_min_z: ", pl_min_z)
    sys.stdout.flush()
    

    for i in range(len(far_points)):
        index = far_points[i]
        if(point_locations[index][0] < pl_max_x + max_dist and point_locations[index][0] > pl_min_x - max_dist and
           point_locations[index][2] < pl_max_z + max_dist and point_locations[index][2] > pl_min_z - max_dist):
            continue
        
        print("deleted point on coordinates: ", point_locations[index][0], " - ", point_locations[index][2])
        sys.stdout.flush()
        point_locations = np.delete(point_locations, far_points[i], 0)
        n_points -= 1
        for j in range (i + 1, len(far_points)):
            far_points[j] -= 1


    kf_min_x = np.floor(np.min(keyframe_locations[:, 0]))
    kf_min_z = np.floor(np.min(keyframe_locations[:, 2]))
    kf_max_x = np.ceil(np.max(keyframe_locations[:, 0]))
    kf_max_z = np.ceil(np.max(keyframe_locations[:, 2]))

    pc_min_x = np.floor(np.min(point_locations[:, 0]))
    pc_min_z = np.floor(np.min(point_locations[:, 2]))
    pc_max_x = np.ceil(np.max(point_locations[:, 0]))
    pc_max_z = np.ceil(np.max(point_locations[:, 2]))

    np.savetxt("point_locations2.txt", point_locations, fmt='%d')


    grid_min_x = min(kf_min_x, pc_min_x)
    grid_min_z = min(kf_min_z, pc_min_z)
    grid_max_x = max(kf_max_x, pc_max_x)
    grid_max_z = max(kf_max_z, pc_max_z)

    print('grid_max_x: ', grid_max_x)
    print('grid_min_x: ', grid_min_x)
    print('grid_max_z: ', grid_max_z)
    print('grid_min_z: ', grid_min_z)



    grid_res = [int(grid_max_x - grid_min_x), int(grid_max_z - grid_min_z)]
    print('grid_ress: ', grid_res)

    visit_counter = np.zeros(grid_res, dtype=np.int32)
    occupied_counter = np.zeros(grid_res, dtype=np.int32)

    print('grid extends from ({:f}, {:f}) to ({:f}, {:f})'.format(
        grid_min_x, grid_min_z, grid_max_x, grid_max_z))

    grid_cell_size_x = (grid_max_x - grid_min_x) / float(grid_res[0])
    grid_cell_size_z = (grid_max_z - grid_min_z) / float(grid_res[1])

    norm_factor_x = float(grid_res[0] - 1) / float(grid_max_x - grid_min_x)
    norm_factor_z = float(grid_res[1] - 1) / float(grid_max_z - grid_min_z)
    print('norm_factor_x: ', norm_factor_x)
    print('norm_factor_z: ', norm_factor_z)

    sys.stdout.flush()

    for point_id in range(n_points):
        point_location = point_locations[point_id]
        for timestamp in point_timestamps[point_id]:
            try:
                keyframe_location = keyframe_locations_dict[timestamp]
            except KeyError:
                continue
            keyframe_x = int(keyframe_location[0])
            keyframe_z = int(keyframe_location[2])
            point_x = int(point_location[0])
            point_z = int(point_location[2])
            ray_points = get_line_bresenham([keyframe_x, keyframe_z], [point_x, point_z])
            n_ray_pts = len(ray_points)

            for ray_point_id in range(n_ray_pts - 1):
                ray_point_x_norm = int(np.floor((ray_points[ray_point_id][0] - grid_min_x) * norm_factor_x))
                ray_point_z_norm = int(np.floor((ray_points[ray_point_id][1] - grid_min_z) * norm_factor_z))
                
                try:
                    # visit_counter[start_x:end_x, start_z:end_z] += 1
                    visit_counter[ray_point_x_norm, ray_point_z_norm] += 1
                except IndexError:
                    print('Out of bound point: ({:d}, {:d}) -> ({:f}, {:f})'.format(
                        ray_points[ray_point_id][0], ray_points[ray_point_id][1],
                        ray_point_x_norm, ray_point_z_norm))
                    sys.stdout.flush()
                    sys.exit(0)
            ray_point_x_norm = int(np.floor((ray_points[-1][0] - grid_min_x) * norm_factor_x))
            ray_point_z_norm = int(np.floor((ray_points[-1][1] - grid_min_z) * norm_factor_z))
            
            try:
                if is_ground_point[point_id]:
                    visit_counter[ray_point_x_norm, ray_point_z_norm] += 1
                else:
                    # occupied_counter[start_x:end_x, start_z:end_z] += 1
                    occupied_counter[ray_point_x_norm, ray_point_z_norm] += 1
            except IndexError:
                print('Out of bound point: ({:d}, {:d}) -> ({:f}, {:f})'.format(
                    ray_points[-1][0], ray_points[-1][1], ray_point_x_norm, ray_point_z_norm))
                sys.stdout.flush()
                sys.exit(0)
        if (point_id + 1) % 1000 == 0:
            print('Done {:d} points of {:d}'.format(point_id + 1, n_points))
            sys.stdout.flush()

    print('Saving counters to {:s} and {:s}'.format(occupied_counter_fname, visit_counter_fname))
    sys.stdout.flush()
    np.savetxt(occupied_counter_fname, occupied_counter, fmt='%d')
    np.savetxt(visit_counter_fname, visit_counter, fmt='%d')

free_thresh = 0.55
occupied_thresh = 0.50

grid_map = np.zeros(grid_res, dtype=np.float32)
grid_map_thresh = np.zeros(grid_res, dtype=np.uint8)
for x in range(grid_res[0]):
    for z in range(grid_res[1]):
        if visit_counter[x, z] == 0 or occupied_counter[x, z] == 0:
            grid_map[x, z] = 0.5
        else:
            grid_map[x, z] = 1 - occupied_counter[x, z] / visit_counter[x, z]
        if grid_map[x, z] >= free_thresh:
            grid_map_thresh[x, z] = 255
        elif occupied_thresh <= grid_map[x, z] < free_thresh:
            grid_map_thresh[x, z] = 128
        else:
            grid_map_thresh[x, z] = 0

if resize_factor != 1:
    grid_res_resized = (grid_res[0] * resize_factor, grid_res[1] * resize_factor)
    print('grid_res_resized: ', grid_res_resized)
    sys.stdout.flush()
    grid_map_resized = cv2.resize(grid_map_thresh, grid_res_resized)
else:
    grid_map_resized = grid_map_thresh




"""
2) A* Pathfinding
"""

import math
import matplotlib.pyplot as plt
import matplotlib.image as mpimg
import numpy as np
from scipy.spatial import KDTree
import time
import socket
from threading import Thread

print("Running A* pathfinding...");
sys.stdout.flush()

show_animation = False


class AStarPlanner:
    def __init__(self, ox, oy, resolution, rr):
        self.resolution = resolution
        self.rr = rr
        self.min_x, self.min_y = 0, 0
        self.max_x, self.max_y = 0, 0
        self.obstacle_map = None
        self.x_width, self.y_width = 0, 0
        self.motion = self.get_motion_model()
        self.obstacle_kd_tree = KDTree(list(zip(ox, oy)))
        self.calc_obstacle_map(ox, oy)

    class Node:
        def __init__(self, x, y, cost, parent_index):
            self.x = x
            self.y = y
            self.cost = cost
            self.parent_index = parent_index

        def __str__(self):
            return str(self.x) + "," + str(self.y) + "," + str(self.cost) + "," + str(self.parent_index)

    def planning(self, sx, sy, gx, gy):
        start_node = self.Node(self.calc_xy_index(sx, self.min_x), self.calc_xy_index(sy, self.min_y), 0.0, -1)
        goal_node = self.Node(self.calc_xy_index(gx, self.min_x), self.calc_xy_index(gy, self.min_y), 0.0, -1)

        open_set, closed_set = dict(), dict()
        open_set[self.calc_grid_index(start_node)] = start_node

        

        while True:
            #print(len(open_set))
            if len(open_set) == 0:
                print("Open set is empty..")
                sys.stdout.flush()
                break

            c_id = min(open_set, key=lambda o: open_set[o].cost + self.calc_heuristic(goal_node, open_set[o]))
            current = open_set[c_id]

            if show_animation:
                plt.plot(self.calc_grid_position(current.x, self.min_x), self.calc_grid_position(current.y, self.min_y),
                         "xc")
                plt.gcf().canvas.mpl_connect('key_release_event',
                                             lambda event: [exit(0) if event.key == 'escape' else None])
                if len(closed_set.keys()) % 10 == 0:
                    plt.pause(0.001)

            if current.x == goal_node.x and current.y == goal_node.y:
                print("Goal found")
                sys.stdout.flush()
                goal_node.parent_index = current.parent_index
                goal_node.cost = current.cost
                break

            del open_set[c_id]
            closed_set[c_id] = current

            for i, _ in enumerate(self.motion):
                node = self.Node(current.x + self.motion[i][0], current.y + self.motion[i][1],
                                 current.cost + self.motion[i][2], c_id)
                n_id = self.calc_grid_index(node)

                if not self.verify_node(node):
                    continue

                if n_id in closed_set:
                    continue

                if n_id not in open_set:
                    open_set[n_id] = node
                else:
                    if open_set[n_id].cost > node.cost:
                        open_set[n_id] = node

        rx, ry = self.calc_final_path(goal_node, closed_set)

        return rx, ry

    def calc_final_path(self, goal_node, closed_set):
        rx, ry = [self.calc_grid_position(goal_node.x, self.min_x)], [self.calc_grid_position(goal_node.y, self.min_y)]
        parent_index = goal_node.parent_index
        while parent_index != -1:
            n = closed_set[parent_index]
            rx.append(self.calc_grid_position(n.x, self.min_x))
            ry.append(self.calc_grid_position(n.y, self.min_y))
            parent_index = n.parent_index

        return rx, ry

    @staticmethod
    def calc_heuristic(n1, n2):
        w = 1.0
        d = w * math.hypot(n1.x - n2.x, n1.y - n2.y)
        return d

    def calc_grid_position(self, index, min_position):
        pos = index * self.resolution + min_position
        return pos

    def calc_xy_index(self, position, min_pos):
        return round((position - min_pos) / self.resolution)

    def calc_grid_index(self, node):
        return (node.y - self.min_y) * self.x_width + (node.x - self.min_x)

    def verify_node(self, node):
        px = self.calc_grid_position(node.x, self.min_x)
        py = self.calc_grid_position(node.y, self.min_y)

        if px < self.min_x or py < self.min_y or px >= self.max_x or py >= self.max_y:
            return False

        # Query the KDTree for points within the radius
        query_result = self.obstacle_kd_tree.query_ball_point([px, py], self.rr)
        if len(query_result) > 0:
            return False

        return True



    ## BU KISIM ÇOK UZUN SÜRÜYOR
    def calc_obstacle_map(self, ox, oy):
        self.min_x = round(min(ox))
        self.min_y = round(min(oy))
        self.max_x = round(max(ox))
        self.max_y = round(max(oy))
        print("min_x:", self.min_x)
        print("min_y:", self.min_y)
        print("max_x:", self.max_x)
        print("max_y:", self.max_y)

        self.x_width = round((self.max_x - self.min_x) / self.resolution) + 1
        self.y_width = round((self.max_y - self.min_y) / self.resolution) + 1
        print("x_width:", self.x_width)
        print("y_width:", self.y_width)

        sys.stdout.flush()

        # self.obstacle_map = np.zeros((self.x_width, self.y_width), dtype=bool)
        # for ix in range(self.x_width):
        #     print("calc_obstacle_map: ", ix, "/", self.x_width, end="\r")
        #     x = self.calc_grid_position(ix, self.min_x)
        #     for iy in range(self.y_width):
        #         y = self.calc_grid_position(iy, self.min_y)
        #         for iox, ioy in zip(ox, oy):
        #             d = math.hypot(iox - x, ioy - y)
        #             if d <= self.rr:
        #                 self.obstacle_map[ix][iy] = True
        #                 break

    @staticmethod
    def get_motion_model():
        motion = [[1, 0, 1],
                  [0, 1, 1],
                  [-1, 0, 1],
                  [0, -1, 1],
                  [-1, -1, math.sqrt(2)],
                  [-1, 1, math.sqrt(2)],
                  [1, -1, math.sqrt(2)],
                  [1, 1, math.sqrt(2)]]

        return motion


print(__file__ + " start!!")
sys.stdout.flush()


# ROBOTUN İLK KONUMU.
# sx, sy = get_robot_pos()
sx = 530.0  # [m]
sy = 440.0  # [m]



grid_size = 2.0  # [m]
robot_radius = 1.0  # [m]

ox, oy = [], []


img = grid_map_resized
img_height, img_width = img.shape[:2]

# Haritanın kullanmak istediğiniz alanının kordinatları
min_x_range = 0
min_y_range = 0
max_x_range = img_width
max_y_range = img_height



ox, oy = [], []
for i in range(min_x_range, max_x_range):
    ox.append(i)
    oy.append(min_y_range)
for i in range(min_y_range, max_y_range):
    ox.append(max_x_range)
    oy.append(i)
for i in range(min_x_range, max_x_range):
    ox.append(i)
    oy.append(max_y_range)
for i in range(min_y_range, max_y_range):
    ox.append(min_x_range)
    oy.append(i)


# Calculate the obstacle positions based on the image
# Haritadaki engel noktalarını tanıtma kısmı. 128 keşfedilmemiş noktaların, 0 engellerin, 255 boşlukların değeri. 
# img[y, x] <= 200 kısmındaki değerin değiştirilmesi gerekebilir. 
for x in range(min_x_range, max_x_range):
    for y in range(min_y_range, max_y_range):
        if (img[y, x] <= 200):
            #print("obstacle on: ", x, " ", y)
            ox.append(x)
            oy.append(y)

a_star = AStarPlanner(ox, oy, grid_size, robot_radius)


# Create the plot
fig, ax = plt.subplots()

# Display the image
ax.imshow(img)



# Plot the clicked points as scatter markers
robot_icon = ax.scatter([], [], marker='*', color='red')
robot_pos = []
robot_pos.append((sx, sy))
robot_icon.set_offsets(robot_pos)


dest_icon = ax.scatter([], [], marker='*', color='blue')
dest_pos = []


line_objects = []


def click(gx, gy):
    print(f"Clicked at coordinates: x={gx}, y={gy}")
    sys.stdout.flush()

    sx, sy = robot_pos[0]

    dest_pos.clear()
    dest_pos.append((gx, gy))
    dest_icon.set_offsets(dest_pos)

    for line in line_objects:
        line.remove()
    line_objects.clear()

    rx, ry = a_star.planning(sx, sy, gx, gy)
    line, = plt.plot(rx, ry, "-r")
    line_objects.append(line)
    plt.draw()



    ## GERÇEK ROBOTTA BÖYLE BİR ALGORİTMA KULLANILABİLİR
    # if(len(rx) > 0):
    #     while True:
    #         robot_pos_x, robot_pos_y = get_robot_pos()
    #         time.sleep(0.5)
    #         robot_pos.clear()
    #         robot_pos.append((robot_pos_x, robot_pos_y))
    #         sx = robot_pos_x
    #         sy = robot_pos_y
    #         robot_icon.set_offsets(robot_pos)
    #         plt.draw()
    #         plt.pause(0.1)
    #         if(destination_reached):
    #             break
        

    ## ÖRNEK ANİMASYON
    if(len(rx) > 0):
        for i in range(len(rx) - 1, 0, -10):
            robot_pos.clear()
            robot_pos.append((rx[i], ry[i]))
            sx = rx[i]
            sy = ry[i]
            robot_icon.set_offsets(robot_pos)
            plt.draw()
            plt.pause(0.1)
            time.sleep(0.5)
        if(len(rx) > 1):
            robot_icon.set_offsets(dest_pos)


    plt.draw()
    print("Movement completed.")
    sys.stdout.flush()


def onclick(event):
    click(event.xdata, event.ydata)


sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(("127.0.0.1", 8088))


def receive_clicks():
    while True:
        gx = int.from_bytes(sock.recv(4), 'little', signed=True)
        gy = int.from_bytes(sock.recv(4), 'little', signed=True)
        width = int.from_bytes(sock.recv(4), 'little', signed=True)
        height = int.from_bytes(sock.recv(4), 'little', signed=True)
        gx = (gx / width) * max_x_range
        gy = (gy / height) * max_y_range
        click(gx, gy)
        

Thread(target=receive_clicks).start()


def ondraw(event):
    frame = cv2.cvtColor(np.asarray(fig.canvas.buffer_rgba()), cv2.COLOR_RGBA2RGB)
    buffer = cv2.imencode(".jpg", frame, [cv2.IMWRITE_JPEG_QUALITY, 50])[1].tobytes()
    sock.sendall(buffer)


# Register the event handler function
fig.canvas.mpl_connect('button_press_event', onclick)
fig.canvas.mpl_connect('draw_event', ondraw)

# Set grid and equal aspect ratio
ax.grid(True)
ax.axis("equal")

# Display the plot
plt.subplots_adjust(0, 0, 1, 1)
plt.show()

