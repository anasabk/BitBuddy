import numpy as np
import cv2

# Define intrinsic camera parameters
focal_length = 500
image_width = 640
image_height = 480
intrinsic_matrix = np.array([
	[focal_length, 0, image_width/2],
	[0, focal_length, image_height/2],
	[0, 0, 1]
])

# Define extrinsic camera parameters
rvec = np.array([0, 0, 0], dtype=np.float32)
tvec = np.array([0, 0, 100], dtype=np.float32)

# Generate 3D points on a paraboloid
u_range = np.linspace(-1, 1, num=20)
v_range = np.linspace(-1, 1, num=20)
u, v = np.meshgrid(u_range, v_range)
x = u
y = v
z = u**2 + v**2

points_3d = np.stack([x, y, z], axis=-1).reshape(-1, 3)

# Create a video capture object
cap = cv2.VideoCapture(0)  # 0 represents the default camera

while True:
	# Capture frame from the camera
	ret, frame = cap.read()
	
	# Project 3D points onto 2D plane
	points_2d, _ = cv2.projectPoints(points_3d,
									rvec, tvec.reshape(-1, 1),
									intrinsic_matrix,
									None)

	# Plot 2D points on the captured frame
	for point in points_2d.astype(int):
		frame = cv2.circle(frame, tuple(point[0]), 2, (0, 0, 255), -1)

	# Display the frame
	cv2.imshow('Camera View', frame)

	# Check for key press
	if cv2.waitKey(1) & 0xFF == ord('q'):
		break

# Release the video capture object and close windows
cap.release()
cv2.destroyAllWindows()
