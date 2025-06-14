import socket
import random
import time
import zlib
import sys
from concurrent.futures import ThreadPoolExecutor

keys = ["tree", "sky", "grass", "cloud", "flower"]

def connect():
    while True:
        try:
            s = socket.socket()
            s.connect(("localhost", 8888))
            return s
        except:
            print("Waiting for server...")
            time.sleep(1)

def run_client(client_id):
    sock = connect()
    for _ in range(10000):
        key = random.choice(keys)
        if random.random() < 0.99:
            cmd = f"$get {key}"
        else:
            value = f"{random.randint(1, 100)}"
            cmd = f"$set {key}={value}"

        try:
            compressed_cmd = zlib.compress((cmd).encode())
            sock.sendall(compressed_cmd)
            response_compressed = sock.recv(4096)
            response = zlib.decompress(response_compressed).decode()
            print(f"[Client {client_id}] {response.strip()}")
        except Exception as e:
            print(f"[Client {client_id}] Reconnecting due to error: {e}")
            sock = connect()

if __name__ == "__main__":
    if len(sys.argv) > 1:
        try:
            NUM_CLIENTS = int(sys.argv[1])
        except ValueError:
            print("Usage: python client.py [NUM_CLIENTS]")
            sys.exit(1)
    else:
        NUM_CLIENTS = 1

    with ThreadPoolExecutor(max_workers=NUM_CLIENTS) as executor:
        for i in range(NUM_CLIENTS):
            executor.submit(run_client, i)
