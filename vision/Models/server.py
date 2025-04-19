import os
import zmq
import struct
import cv2
import numpy as np
import socket
import json
import time
from easyOCR import performOCR

DISCOVERY_PORT = 5005 # UDP discovery port

def loadConfig():
    config_path = os.path.join(os.path.dirname(__file__), "../config.json")
    with open(config_path, "r") as f:
        return json.load(f)

def getLocalIP():
    """Get the actual network IP of this machine (not 127.0.0.1)."""
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.connect(("8.8.8.8", 80))  # Connect to an external server
        ip = sock.getsockname()[0]
        sock.close()
        return ip
    except Exception as e:
        print(f"Failed to get local IP: {e}")
        return "127.0.0.1"

def waitForPiDiscovery(discoveryPort):
    """Wait for a Raspberry Pi discovery request, then send the IP."""
    serverIP = getLocalIP()
    print(f"Waiting for Raspberry Pi discovery on UDP {discoveryPort}...")

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(("0.0.0.0", discoveryPort))  # Listen on all interfaces

    while True:
        try:
            data, clientAddress = sock.recvfrom(1024)  # Receive broadcast
            message = data.decode("utf-8").strip()

            if message == "DISCOVER_SERVER":
                print(f"Received discovery request from {clientAddress[0]}")
                sock.sendto(serverIP.encode(), clientAddress)  # Send server IP back
                sock.close()
                return
        except Exception as e:
            print(f"UDP error: {e}")

def runServer():
    config = loadConfig()
    network = config.get("network", {})
    useEthernet = network.get("useEthernet", True) #default to ethernet if config is bad
    port = network.get("serverPort", 5555)
    heartbeatPort = network.get("heartbeatPort", 5556)
    discoveryPort = network.get("discoveryPort", 5005)

    if not useEthernet:
        waitForPiDiscovery(discoveryPort)
    else:
        print("Skipping discovery. Binding immediately.")

    ADDRESS = f"tcp://0.0.0.0:{port}"  # Bind ZeroMQ to communicate with Pi
    HEARTBEAT_ADDRESS = f"tcp://0.0.0.0:{heartbeatPort}"

    # Create ZeroMQ context and socket
    context = zmq.Context()
    socket = context.socket(zmq.REP)
    socket.bind(ADDRESS)

    heartbeatSocket = context.socket(zmq.REP)
    heartbeatSocket.bind(HEARTBEAT_ADDRESS)
    heartbeatSocket.RCVTIMEO = 0  # non-blocking receive
    
    print(f"Server started on {ADDRESS}")
    
    poller = zmq.Poller()
    poller.register(socket, zmq.POLLIN)

    lastHeartbeatTime = time.time()
    activeProcessing = False
    try:
        startMessage = socket.recv_string() #wait for the pi to be connected
        if startMessage == "connected":
            print("Pi is connected")
            socket.send_string("awake")
        while True:
            print("Waiting for image from Raspberry Pi...")

            socks = dict(poller.poll(1000))  # 1-second timeout

            if socket in socks and socks[socket] == zmq.POLLIN:
                # Receive image data (blocking receive)
                messageParts = socket.recv_multipart()  
                try:
                    if len(messageParts) == 1:
                        # Handle control message
                        command = messageParts[0].decode()
                        if command == "start":
                            activeProcessing = True
                            print("Processing started.")
                            socket.send_string("yes")
                        elif command == "stop":
                            activeProcessing = False
                            print("Processing stopped.")
                            socket.send_string("yes")
                        else:
                            print(f"Unknown command: {command}")
                            socket.send_string("ERROR: Unknown command")
                    elif len(messageParts) == 2 and activeProcessing:
                        print(f"Request received!")
                        imageSize = struct.unpack("Q", messageParts[0])[0]
                        imageData = messageParts[1]

                        print(f"Image is {imageSize} bytes!")

                        print(f"Decoding image.")
                        # Decode image
                        image = cv2.imdecode(np.frombuffer(imageData, dtype=np.uint8), cv2.IMREAD_COLOR)
                        if image is None:
                            print("Error: Failed to decode image.")
                            socket.send_string("ERROR: Image decoding failed")
                            continue
                        
                        print(f"Image decoded. Now beginning AI processing.")

                        # Perform OCR
                        result = json.dumps(performOCR(image))

                        print(f"Processing complete. Sending result back.")
                        # Send response
                        socket.send_string(result)
                        print(f"Sent response: {result}")

                        # Reset heartbeat timer after processing completes
                        lastHeartbeatTime = time.time()
                except Exception as e:
                    print(f"Processing error: {e}")
                    socket.send_string("ERROR: Exception during processing")
            elif not activeProcessing:
                try:
                    heartbeatmessage = heartbeatSocket.recv_string(flags=zmq.NOBLOCK)
                    if heartbeatmessage == "heartbeat":
                        print("Received heartbeat")
                        heartbeatSocket.send_string("alive")
                        lastHeartbeatTime = time.time()
                except zmq.Again:
                    pass  # No heartbeat received

                if time.time() - lastHeartbeatTime > 5:
                    print("Pi disconnect detected. Restarting server...")
                    break
    except KeyboardInterrupt:
        print("Server interrupted")
    finally:
        # Clean up
        socket.close()
        heartbeatSocket.close()
        context.term()
        print("Server terminated")

if __name__ == "__main__":
    while True:
        runServer()
        print("restarting server...")
        time.sleep(2)
