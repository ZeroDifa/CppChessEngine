import subprocess
import os
import signal
import sys
import time
import ctypes
import logging
import threading
import queue

# Настройка логирования
logging.basicConfig(level=logging.DEBUG, format='%(asctime)s - %(levelname)s - %(message)s', handlers=[
    logging.FileHandler("project_manager.log"),
    logging.StreamHandler()
])

def is_admin():
    try:
        return ctypes.windll.shell32.IsUserAnAdmin()
    except:
        return False

def run_as_admin():
    if not is_admin():
        logging.info("Requesting administrator privileges...")
        time.sleep(1)
        ctypes.windll.shell32.ShellExecuteW(
            None, "runas", sys.executable, " ".join(sys.argv), None, 1
        )
        sys.exit(0)

class ProjectManager:
    def __init__(self):
        self.server_process = None
        self.log_queue = queue.Queue()

    def build_cpp(self, build_script):
        try:
            logging.info(f"Building C++ program with script: {build_script}")
            result = subprocess.run(build_script, shell=True)
            if result.returncode != 0:
                logging.error("Build failed!")
                return False
            logging.info("Build succeeded!")
            return True
        except Exception as e:
            logging.error(f"An error occurred during build: {e}")
            return False

    def start_server(self):
        try:
            logging.info("Starting Node.js server...")
            # Установим рабочую директорию на корневую директорию проекта
            server_cwd = os.path.join(os.getcwd(), 'server')
            self.server_process = subprocess.Popen(
                ["node", "server.js"],
                cwd=server_cwd,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )
            logging.info("Node.js server started with PID: {}".format(self.server_process.pid))
            threading.Thread(target=self.log_server_output, daemon=True).start()
        except Exception as e:
            logging.error(f"An error occurred while starting the server: {e}")

    def stop_server(self):
        if self.server_process:
            try:
                logging.info("Stopping Node.js server...")
                self.server_process.terminate()
                self.server_process.wait()
                self.server_process = None
                logging.info("Server stopped.")
            except Exception as e:
                logging.error(f"An error occurred while stopping the server: {e}")

    def restart_server(self):
        self.stop_server()
        self.start_server()

    def log_server_output(self):
        if self.server_process:
            try:
                while True:
                    output = self.server_process.stdout.readline()
                    error_output = self.server_process.stderr.readline()
                    if output == '' and error_output == '' and self.server_process.poll() is not None:
                        break
                    if output:
                        self.log_queue.put(('info', output.strip()))
                    if error_output:
                        self.log_queue.put(('error', error_output.strip()))
                    time.sleep(0.1)
            except Exception as e:
                self.log_queue.put(('error', f"An error occurred while logging server output: {e}"))

    def process_log_queue(self):
        while not self.log_queue.empty():
            level, message = self.log_queue.get()
            if level == 'info':
                logging.info(message)
            elif level == 'error':
                logging.error(message)

    def run(self, build_scripts):
        for script in build_scripts:
            if not self.build_cpp(script):
                logging.warning("Skipping server start due to build failure.")
                return
        self.start_server()

if __name__ == "__main__":
    run_as_admin()

    build_scripts = [
        "g++ -o main.exe -O2 main.cpp",
        # Add more build scripts if needed
    ]
    manager = ProjectManager()
    manager.run(build_scripts)

    try:
        while True:
            manager.process_log_queue()
            command = input("Enter command (stop, restart, exit): ").strip().lower()
            if command == "stop":
                manager.stop_server()
            elif command == "restart":
                manager.restart_server()
            elif command == "exit":
                manager.stop_server()
                break
            else:
                logging.warning("Unknown command.")
    except KeyboardInterrupt:
        manager.stop_server()
        logging.info("\nExiting...")
    except Exception as e:
        logging.error(f"An unexpected error occurred: {e}")
        manager.stop_server()