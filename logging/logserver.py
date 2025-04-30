import enum
import queue
import socket
import struct
import threading
from datetime import datetime
from termcolor import colored

import os
os.chdir(os.path.dirname(__file__))

SWITCH_PORT:int = 8171

msg_queue:queue.Queue = queue.Queue()


# ========== LOGGING ==========

# TODO: remove
# _print = print
# print = None

def log(user: str, message: str, color: str = "white") -> None:
    timestamp = datetime.now().strftime("%H:%M:%S")
    print(f"{colored(f'[{timestamp} - {user}]', color)} {message}")


# ========== PACKETS ==========

PACKET_HEADER_SIZE:int = 0x10
PACKET_SIZE:int = 0x401

class PacketType(enum.IntEnum):
    Script = 0x01

class RecPacketType(enum.IntEnum):
    LogInfo = 0x01
    LogError = 0x02
    LogWarning = 0x03

class PacketScript:
    TYPENAME = "script"
    OPCODE = PacketType.Script.value

    def __init__(self, name: str, data: bytes):
        self.name = name
        self.data = data
    
    def construct_header(self) -> bytearray:
        out = bytearray(PACKET_HEADER_SIZE)
        out[0] = self.OPCODE
        out[1] = min(len(self.name), 0xff)
        out[4:8] = struct.pack("!I", len(self.data))
        return out

    def construct(self) -> bytearray:
        out = bytearray()
        out.extend(self.construct_header())
        out.extend(struct.pack("255s", bytes(self.name, "utf-8")))
        out.extend(self.data)
        return out




# ========== SWITCH SERVER ==========

def switch_send_func(client_sock: socket.socket, stop) -> None:
    while not stop():
        try:
            packet = msg_queue.get(timeout=0.5)
        except queue.Empty:
            continue

        log("Info", f"sending packet type `{packet.TYPENAME}`", "white")
        client_sock.send(packet.construct())


# ========== SWITCH SERVER ==========

def serve_switch() -> None:
    # create socket
    server_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    # bind socket to specified port
    server_address = ("0.0.0.0", SWITCH_PORT)
    server_sock.bind(server_address)

    # start listening on socket
    server_sock.listen(2)
    log("Info", f"listening on port {SWITCH_PORT}", "white")

    while True:
        log("Info", "waiting for connection...", "white")
        client_sock, client_addr = server_sock.accept()
        log("Info", f"connection from {client_addr[0]}", "white")

        # create thread
        stop_switch_send_thread = False
        switch_send_thread = threading.Thread(target=switch_send_func, args=(client_sock, lambda: stop_switch_send_thread))
        switch_send_thread.start()

        try:
            while True:
                data = client_sock.recv(PACKET_SIZE)
                if data:
                    type:int = data[0]
                    try:
                        match type:
                            case RecPacketType.LogInfo.value:
                                log('Info-SW', f"{data[1:].decode('utf-8')}", "green")
                            case RecPacketType.LogError.value:
                                log('Error-SW', f"{data[1:].decode('utf-8')}", 'red')
                            case RecPacketType.LogWarning.value:
                                log('Warn-SW', f"{data[1:].decode('utf-8')}", 'yellow')
                            case _:
                                log("Warn-SW", f"unknown packet type: {type} and length {len(data)}", "yellow")
                                # log("Info-SW", f"received unknown packet: {data[1:]}", "green")
                    except UnicodeDecodeError:
                        log("Error", f"failed to decode packet: {data[1:]}", "light_red")
                else:
                    log("Info", f"received 0 bytes", "white")
                    break

                # log("switch", f"{client_addr} -> {data}")
                # while not msg_queue.empty():
                #     client_sock.send(msg_queue.get())

        except ConnectionResetError:
            log("Info", "connection reset", "white")
            break

        finally:
            log("Info", "client disconnected", "white")
            stop_switch_send_thread = True
            switch_send_thread.join()
            log("Info", "send thread terminated", "white")
            client_sock.close()


def main() -> None:

    try:
        serve_switch()
    except KeyboardInterrupt:
        log("Info", "closing server...", "white")


if __name__ == "__main__":
    main()
