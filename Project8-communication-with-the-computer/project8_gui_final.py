"""Project8 final Python GUI
Cross-platform GUI using Tkinter and pyserial.
Features:
- Connect/Disconnect serial port
- Send newline-terminated duration (ms)
- Background reader thread that pushes lines to GUI log
- Validates numeric input and enforces an upper bound
- Displays device state messages (0/1/2) with human text

Run: `python project8_gui_final.py`
Requires: pyserial
"""

import threading
import queue
import time
import sys
import tkinter as tk
from tkinter import ttk, messagebox, scrolledtext

try:
    import serial
    import serial.tools.list_ports
except Exception as e:
    serial = None

BAUD_RATE = 9600
READ_TIMEOUT = 0.1
LOG_MAX_LINES = 500
MAX_DURATION_MS = 600000  # 10 minutes


def list_serial_ports():
    if serial is None:
        return []
    ports = [p.device for p in serial.tools.list_ports.comports()]
    return ports


def serial_reader(ser, q, stop_event):
    try:
        while not stop_event.is_set():
            try:
                raw = ser.readline()
                if not raw:
                    continue
                line = raw.decode('ascii', errors='replace').strip()
                q.put(line)
            except Exception as e:
                q.put(f"SERIAL ERROR: {e}")
                break
    finally:
        try:
            ser.close()
        except Exception:
            pass


class App:
    def __init__(self, root):
        self.root = root
        self.root.title('Project8 - LED Duration Controller')

        self.port_var = tk.StringVar()
        self.duration_var = tk.StringVar(value='5000')

        top = ttk.Frame(root)
        top.grid(row=0, column=0, sticky='ew', padx=6, pady=6)

        ttk.Label(top, text='Serial port:').grid(row=0, column=0, sticky='w')
        self.port_combo = ttk.Combobox(top, textvariable=self.port_var, width=18)
        self.port_combo['values'] = list_serial_ports() or ['COM3', '/dev/ttyACM0']
        self.port_combo.grid(row=0, column=1, padx=4)

        self.refresh_btn = ttk.Button(top, text='Refresh', command=self.refresh_ports)
        self.refresh_btn.grid(row=0, column=2, padx=4)

        self.connect_btn = ttk.Button(top, text='Connect', command=self.connect)
        self.connect_btn.grid(row=0, column=3, padx=4)

        self.disconnect_btn = ttk.Button(top, text='Disconnect', command=self.disconnect, state='disabled')
        self.disconnect_btn.grid(row=0, column=4, padx=4)

        ttk.Label(top, text='Duration (ms):').grid(row=1, column=0, sticky='w', pady=(8,0))
        self.duration_entry = ttk.Entry(top, textvariable=self.duration_var, width=16)
        self.duration_entry.grid(row=1, column=1, padx=4, pady=(8,0))

        self.send_btn = ttk.Button(top, text='Send', command=self.send_duration, state='disabled')
        self.send_btn.grid(row=1, column=3, padx=4, pady=(8,0))

        self.log = scrolledtext.ScrolledText(root, width=80, height=20, state='disabled')
        self.log.grid(row=2, column=0, padx=6, pady=6)

        self.msg_queue = queue.Queue()
        self.stop_event = threading.Event()
        self.reader_thread = None
        self.ser = None
        self.log_lines = []

        self.root.protocol('WM_DELETE_WINDOW', self.on_close)
        self.root.after(100, self.process_queue)

    def refresh_ports(self):
        values = list_serial_ports() or ['COM3', '/dev/ttyACM0']
        self.port_combo['values'] = values
        if values:
            self.port_combo.set(values[0])
        self.log_message('Ports refreshed')

    def connect(self):
        if serial is None:
            messagebox.showerror('Missing dependency', 'pyserial is required. Install with:\n pip install pyserial')
            return
        port = self.port_var.get().strip()
        if not port:
            messagebox.showwarning('Port required', 'Enter a serial port name.')
            return
        try:
            self.ser = serial.Serial(port, BAUD_RATE, timeout=READ_TIMEOUT)
        except Exception as e:
            self.log_message(f'Connection failed: {e}')
            self.ser = None
            return

        self.stop_event.clear()
        self.reader_thread = threading.Thread(target=serial_reader, args=(self.ser, self.msg_queue, self.stop_event), daemon=True)
        self.reader_thread.start()
        self.log_message(f'Connected to {port} at {BAUD_RATE} baud')
        self.connect_btn.config(state='disabled')
        self.disconnect_btn.config(state='normal')
        self.send_btn.config(state='normal')

    def disconnect(self):
        if self.ser is None:
            self.log_message('Not connected')
            return
        self.stop_event.set()
        if self.reader_thread is not None:
            self.reader_thread.join(timeout=1.0)
        try:
            self.ser.close()
        except Exception:
            pass
        self.ser = None
        self.log_message('Disconnected')
        self.connect_btn.config(state='normal')
        self.disconnect_btn.config(state='disabled')
        self.send_btn.config(state='disabled')

    def send_duration(self):
        if self.ser is None or not self.ser.is_open:
            self.log_message('Not connected')
            return
        text = self.duration_var.get().strip()
        if not text.isdigit():
            self.log_message('Duration must be a positive integer (ms)')
            return
        value = int(text)
        if value < 0 or value > MAX_DURATION_MS:
            self.log_message(f'Duration out of bounds (0..{MAX_DURATION_MS} ms)')
            return
        payload = f'{value}\n'.encode('ascii')
        try:
            self.ser.write(payload)
            self.log_message(f'Sent duration: {value} ms')
        except Exception as e:
            self.log_message(f'Send failed: {e}')

    def process_queue(self):
        while not self.msg_queue.empty():
            msg = self.msg_queue.get_nowait()
            self.log_message(self.format_device_message(msg))
        self.root.after(100, self.process_queue)

    def format_device_message(self, msg):
        if msg == '0':
            return 'Device state: LED off'
        if msg == '1':
            return 'Device state: button and LED on'
        if msg == '2':
            return 'Device state: button off'
        return msg

    def log_message(self, text):
        timestamp = time.strftime('%H:%M:%S')
        self.log.configure(state='normal')
        self.log.insert('end', f'[{timestamp}] {text}\n')
        self.log.configure(state='disabled')
        self.log.see('end')

    def on_close(self):
        self.disconnect()
        self.root.destroy()


if __name__ == '__main__':
    root = tk.Tk()
    app = App(root)
    root.mainloop()
