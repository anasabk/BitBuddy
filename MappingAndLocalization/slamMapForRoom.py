import cv2
import numpy as np

# Kamera parametreleri
fx, fy, cx, cy = 525.0, 525.0, 319.5, 239.5
K = np.array([[fx, 0, cx], [0, fy, cy], [0, 0, 1]])

def detect_features(img):
    orb = cv2.ORB_create(nfeatures=1000)
    keypoints = orb.detect(img, None)
    keypoints = cv2.KeyPoint_convert(keypoints)
    return np.array(keypoints, dtype=np.float32)

def track_features(old_img, new_img, old_points):
    lk_params = dict(winSize=(21, 21), maxLevel=3, criteria=(cv2.TERM_CRITERIA_EPS | cv2.TERM_CRITERIA_COUNT, 30, 0.01))
    old_points = old_points.reshape(-1, 1, 2)
    new_points, status, _ = cv2.calcOpticalFlowPyrLK(old_img, new_img, old_points, None, **lk_params)
    if status is not None:
        new_points = new_points[status == 1]
        old_points = old_points[status == 1]
    else:
        new_points = np.array([], dtype=np.float32).reshape(0, 2)
        old_points = np.array([], dtype=np.float32).reshape(0, 2)
    return new_points.reshape(-1, 2), old_points.reshape(-1, 2)


def main():
    cap = cv2.VideoCapture(0)

    # İlk görüntüyü oku ve özellikleri algıla
    ret, old_img = cap.read()
    old_img_gray = cv2.cvtColor(old_img, cv2.COLOR_BGR2GRAY)
    old_points = detect_features(old_img_gray)

    while True:
        ret, new_img = cap.read()
        new_img_gray = cv2.cvtColor(new_img, cv2.COLOR_BGR2GRAY)

        # Özellikleri takip et ve yeni özellikler algıla
        new_points, old_points = track_features(old_img_gray, new_img_gray, old_points)
        detected_points = detect_features(new_img_gray)
        if detected_points.size > 0:
            old_points = np.vstack((old_points, detected_points))

        # Özellikleri çiz ve görüntüyü göster
        for point in new_points:
            x, y = point.ravel()
            cv2.circle(new_img, (int(x), int(y)), 5, (0, 255, 0), -1)
        
        cv2.imshow("MonoSLAM", new_img)

        # Çıkış için "q" tuşuna basın
        if cv2.waitKey(1) & 0xFF == ord("q"):
            break

        # Geçerli görüntüyü ve özellikleri güncelle
        old_img_gray = new_img_gray.copy()

    cap.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()
