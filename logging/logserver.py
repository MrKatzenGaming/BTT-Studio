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

def log(message: str, color: str = "white") -> None:
    timestamp = datetime.now().strftime("%H:%M:%S")
    print(colored(f"[{timestamp}] {message}", color))


# ========== PACKETS ==========

PACKET_HEADER_SIZE:int = 0x10
PACKET_SIZE:int = 0x401

class PacketType(enum.IntEnum):
    NONE = 0x00
    LOG = 0x01

class RecPacketType(enum.IntEnum):
    LOG = 0x01
    
class LogType(enum.IntEnum):
    Info = 1
    Error = 2
    Warning = 3

class Packet:
    TYPENAME = "none"
    OPCODE = PacketType.NONE.value

    def __init__(self):
        pass

    def construct_header(self) -> bytearray:
        out = bytearray(PACKET_HEADER_SIZE)
        out[0] = self.OPCODE
        return out

    def construct(self) -> bytearray:
        out = bytearray()
        out.extend(self.construct_header())
        return out

class PacketLog:
    TYPENAME = "log"
    OPCODE = PacketType.LOG.value

    def __init__(self, data: bytes, log_type: LogType = LogType.Info.value):
        self.data = data
        self.log_type = log_type

    def construct_header(self) -> bytearray:
        out = bytearray(PACKET_HEADER_SIZE)
        out[0] = self.OPCODE
        out[1] = self.log_type
        out[4:8] = struct.pack("<I", min(len(self.data), 0x400))
        return out

    def construct(self) -> bytearray:
        out = bytearray()
        out.extend(self.construct_header())
        out.extend(self.data if len(self.data) < 0x400 else self.data[:0x400])
        return out


# ========== SWITCH SERVER ==========

def switch_send_func(client_sock: socket.socket, stop) -> None:
    while not stop():
        try:
            packet = msg_queue.get(timeout=0.5)
        except queue.Empty:
            continue

        log(f"sending packet type `{packet.TYPENAME}`", "green")
        client_sock.send(packet.construct())


# ========== SWITCH SERVER ==========

def console_input_func() -> None:
    """Thread function to handle console input."""
    strwarn = "warn "
    strinfo = "info "
    strerr = "err "
    while True:
        try:
            user_input = input("")
            if user_input.strip().lower() == "exit":
                log("shutting down server...", "green")
                os._exit(0)  # Forcefully exit the server
            elif user_input.strip().startswith(strerr):
                # Example: Send error message as a packet to the client
                packet = PacketLog(log_type=LogType.Error.value, data=user_input[len(strerr):].encode("utf-8"))
                msg_queue.put(packet)
            elif user_input.strip().startswith(strwarn):
                # Example: Send warning message as a packet to the client
                packet = PacketLog(log_type=LogType.Warning.value, data=user_input[len(strwarn):].encode("utf-8"))
                msg_queue.put(packet)
            elif user_input.strip().startswith(strinfo):
                # Example: Send info message as a packet to the client
                packet = PacketLog(log_type=LogType.Info.value, data=user_input[len(strinfo):].encode("utf-8"))
                msg_queue.put(packet)
            else:
                # Example: Send user input as a packet to the client
                packet = Packet()
                msg_queue.put(packet)
        except EOFError:
            log("console input closed", "green")
            break
    
def serve_switch() -> None:
    # create socket
    server_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    # bind socket to specified port
    server_address = ("0.0.0.0", SWITCH_PORT)
    server_sock.bind(server_address)

    # start listening on socket
    server_sock.listen(2)
    log(f"listening on port {SWITCH_PORT}", "green")

    while True:
        log("waiting for connection...", "green")
        client_sock, client_addr = server_sock.accept()
        log(f"connection from {client_addr[0]}", "green")

        # create thread
        stop_switch_send_thread = False
        switch_send_thread = threading.Thread(target=switch_send_func, args=(client_sock, lambda: stop_switch_send_thread))
        switch_send_thread.start()
        console_thread = threading.Thread(target=console_input_func, daemon=True)
        console_thread.start()

        try:
            while True:
                data = client_sock.recv(PACKET_SIZE)
                if data:
                    type:int = data[0]
                    try:
                        match type:
                            case RecPacketType.LOG.value:
                                # log('Switch', f"{data[1:].decode('utf-8')}", "green")
                                print(f"{data[1:].decode('utf-8')}",end='')
                            case _:
                                log(f"unknown packet type: {type} and length {len(data)}", "yellow")
                                # log("Info-SW", f"received unknown packet: {data[1:]}", "green")
                    except UnicodeDecodeError:
                        log(f"failed to decode packet: {data[1:]}", "red")
                else:
                    log(f"received 0 bytes", "green")
                    break

                # log("switch", f"{client_addr} -> {data}")
                # while not msg_queue.empty():
                #     client_sock.send(msg_queue.get())

        except ConnectionResetError:
            log("connection reset", "green")
            break

        finally:
            log("client disconnected", "green")
            stop_switch_send_thread = True
            switch_send_thread.join()
            log("send thread terminated", "green")
            client_sock.close()


def main() -> None:

    try:
        serve_switch()
    except KeyboardInterrupt:
        log("closing server...", "green")


if __name__ == "__main__":
    main()
