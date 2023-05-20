import os
import cv2
import time
import shutil
from  datetime import datetime

if os.path.exists("images"):
    shutil.rmtree("images")
else:
    print("images not found.")

if os.path.exists("time.txt"):
    os.remove("time.txt")
else:
    print("time.txt not found.")


def save_image(path, counter, image):
    image_path = os.path.join(path, f"image{counter}.jpg")

    cv2.imwrite(image_path, image)


def cam_to_image(path):
    fisrt_time = datetime.now()

    time_txt = open("time.txt", "w")

    cam = cv2.VideoCapture(0)

    if not cam.isOpened():
        print("cam not open.")
        return

    counter = 0

    while True:

        ret, frame = cam.read()

        if not ret:
            print("image not retrieved.")
            break

        cv2.imshow("cam", frame)

        if cv2.waitKey(1) & 0xFF == ord("q"):
            break

        save_image(path, counter, frame)
        counter += 1

        now = time.time_ns()
        time_txt.write(f"{now}\n")

    time_txt.close()
    cam.release()
    cv2.destroyAllWindows()
path = "images"

if not os.path.exists(path):
    os.makedirs(path)

cam_to_image(path)