import math
import matplotlib.pyplot as plt
import matplotlib.image as mpimg
import numpy as np
from scipy.spatial import KDTree
import time

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


def main():
    print(__file__ + " start!!")


    # ROBOTUN İLK KONUMU.
    # sx, sy = get_robot_pos()
    sx = 530.0  # [m]
    sy = 440.0  # [m]



    grid_size = 2.0  # [m]
    robot_radius = 1.0  # [m]

    ox, oy = [], []
    

    img = mpimg.imread("image.pgm")
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


    

    def onclick(event):
        if event.button == 1:  # Check if left mouse button is clicked
            sx, sy = robot_pos[0]
            gx = event.xdata
            gy = event.ydata
            print(f"Clicked at coordinates: x={x}, y={y}")
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




    # Register the event handler function
    cid = fig.canvas.mpl_connect('button_press_event', onclick)

    # Set grid and equal aspect ratio
    ax.grid(True)
    ax.axis("equal")

    # Display the plot
    plt.show()




if __name__ == '__main__':
    main()
