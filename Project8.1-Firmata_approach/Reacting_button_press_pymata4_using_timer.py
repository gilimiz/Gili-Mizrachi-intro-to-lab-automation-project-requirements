"""Firmata-based Arduino button reaction GUI.

This script uses pymata4 to communicate with an Arduino running the firmataexpress sketch.
It monitors a digital input button, turns on an LED when the button is pressed, and uses
threading.Timer to turn the LED off after the configured time interval.
"""

import threading
import time
import tkinter as tk
from tkinter import ttk, messagebox, scrolledtext

try:
    from pymata4 import pymata4
except ImportError:
    pymata4 = None

# Hardware pin configuration
LED_PIN = 4
BUTTON_PIN = 3

# Default timer duration in milliseconds
DEFAULT_DURATION_MS = 30


class FirmataApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Firmata Button Timer")

        self.board = None
        self.timer = None
        self.button_state = tk.StringVar(value="Unknown")
        self.duration_var = tk.StringVar(value=str(DEFAULT_DURATION_MS))
        self.port_var = tk.StringVar(value="COM4")

        self._build_interface()

    def _build_interface(self):
        frame = ttk.Frame(self.root, padding=10)
        frame.grid(row=0, column=0, sticky="nsew")

        ttk.Label(frame, text="Firmata COM port:").grid(row=0, column=0, sticky="w")
        self.port_entry = ttk.Entry(frame, textvariable=self.port_var, width=12)
        self.port_entry.grid(row=0, column=1, sticky="w", padx=(4, 10))

        self.connect_btn = ttk.Button(frame, text="Connect", command=self.connect)
        self.connect_btn.grid(row=0, column=2, sticky="w")

        self.disconnect_btn = ttk.Button(frame, text="Disconnect", command=self.disconnect, state="disabled")
        self.disconnect_btn.grid(row=0, column=3, sticky="w", padx=(4, 0))

        ttk.Label(frame, text="LED duration (ms):").grid(row=1, column=0, sticky="w", pady=(10, 0))
        self.duration_entry = ttk.Entry(frame, textvariable=self.duration_var, width=12)
        self.duration_entry.grid(row=1, column=1, sticky="w", padx=(4, 10), pady=(10, 0))

        self.set_btn = ttk.Button(frame, text="Set Duration", command=self.set_duration, state="disabled")
        self.set_btn.grid(row=1, column=2, sticky="w", pady=(10, 0))

        ttk.Label(frame, text="Button state:").grid(row=2, column=0, sticky="w", pady=(10, 0))
        self.state_label = ttk.Label(frame, textvariable=self.button_state)
        self.state_label.grid(row=2, column=1, columnspan=3, sticky="w", pady=(10, 0))

        self.log = scrolledtext.ScrolledText(self.root, width=60, height=16, state="disabled")
        self.log.grid(row=1, column=0, padx=10, pady=10, sticky="nsew")

        self.root.protocol("WM_DELETE_WINDOW", self.on_close)
        self.root.rowconfigure(1, weight=1)
        self.root.columnconfigure(0, weight=1)

    def connect(self):
        if pymata4 is None:
            messagebox.showerror("Missing dependency", "pymata4 is required. Install with:\n  pip install pymata4")
            return

        try:
            self.board = pymata4.Pymata4(com_port="COM4")
        except Exception as exc:
            self.log_message(f"Failed to connect: {exc}")
            self.board = None
            return

        # Configure pins on the Arduino side
        self.board.set_pin_mode_digital_output(LED_PIN)
        self.board.digital_write(LED_PIN, 0)
        self.board.set_pin_mode_digital_input_pullup(BUTTON_PIN)
        self.board.digital_read(BUTTON_PIN, self._button_callback)

        self.connect_btn.config(state="disabled")
        self.disconnect_btn.config(state="normal")
        self.set_btn.config(state="normal")
        self.log_message(f"Connected to {port}")
        self.log_message(f"Monitoring button on pin {BUTTON_PIN}, LED on pin {LED_PIN}")
        self.set_duration()

    def disconnect(self):
        if self.board:
            if self.timer:
                self.timer.cancel()
                self.timer = None
            try:
                self.board.shutdown()
            except Exception:
                pass
            self.board = None

        self.connect_btn.config(state="normal")
        self.disconnect_btn.config(state="disabled")
        self.set_btn.config(state="disabled")
        self.button_state.set("Disconnected")
        self.log_message("Disconnected")

    def set_duration(self):
        if not self.duration_var.get().strip().isdigit():
            self.log_message("Duration must be a positive integer.")
            return

        value = int(self.duration_var.get().strip())
        if value <= 0:
            self.log_message("Duration must be greater than 0.")
            return

        self.log_message(f"Timer duration set to {value} ms")

    def _button_callback(self, data):
        # data is [pin, value]
        if len(data) < 2:
            return

        _, value = data
        if value == 0:
            self.button_state.set("Pressed")
            self.log_message("Button pressed")
            self._turn_led_on()
        else:
            self.button_state.set("Released")
            self.log_message("Button released")

    def _turn_led_on(self):
        if self.board is None:
            return

        self.board.digital_write(LED_PIN, 1)
        self.log_message("LED ON")

        if self.timer:
            self.timer.cancel()

        duration_ms = int(self.duration_var.get().strip()) if self.duration_var.get().strip().isdigit() else DEFAULT_DURATION_MS
        self.timer = threading.Timer(duration_ms / 1000.0, self._turn_led_off)
        self.timer.start()

    def _turn_led_off(self):
        if self.board is None:
            return

        self.board.digital_write(LED_PIN, 0)
        self.log_message("LED OFF")
        self.timer = None

    def log_message(self, message):
        timestamp = time.strftime("%H:%M:%S")
        self.log.configure(state="normal")
        self.log.insert("end", f"[{timestamp}] {message}\n")
        self.log.configure(state="disabled")
        self.log.see("end")

    def on_close(self):
        self.disconnect()
        self.root.destroy()


def main():
    root = tk.Tk()
    app = FirmataApp(root)
    root.mainloop()


if __name__ == "__main__":
    main()
