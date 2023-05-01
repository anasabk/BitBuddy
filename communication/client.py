import cv2
import socket
import numpy as np

# Create a socket and connect to the server
server_ip = "ip address"  # Replace this with the server's IP address
server_port = 8080

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((server_ip, server_port))

# Receive and display video stream
buf = b''
while True:
    # Receive data from the server
    data = sock.recv(1024)
    if not data:
        break
    buf += data

    # Check if there is enough data for a JPEG frame
    start = buf.find(b'\xff\xd8')
    end = buf.find(b'\xff\xd9')
    if start != -1 and end != -1:
        # Extract JPEG frame
        jpg_data = buf[start:end + 2]
        buf = buf[end + 2:]

        # Decode JPEG data and display the frame
        frame = cv2.imdecode(np.frombuffer(jpg_data, dtype=np.uint8), cv2.IMREAD_COLOR)
        cv2.imshow('Video Stream', frame)

        # Exit if the user presses the 'q' key
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

# Close the connection and window
sock.close()
cv2.destroyAllWindows()
