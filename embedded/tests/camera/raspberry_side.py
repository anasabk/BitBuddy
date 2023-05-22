from flask import Flask, Response
import cv2

app = Flask(_name_)

camera = cv2.VideoCapture(0)  # Raspberry Pi kamerayı aç

def gen_frames():
    while True:
        success, frame = camera.read()  # Frame oku
        if not success:
            break
        else:
            ret, buffer = cv2.imencode('.jpg', frame)
            frame = buffer.tobytes()
            yield (b'--frame\r\n'
                   b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')

@app.route('/video_feed')
def video_feed():
    return Response(gen_frames(), mimetype='multipart/x-mixed-replace; boundary=frame')

if _name_ == '_main_':
    app.run(host='0.0.0.0', port=8080)