import tkinter as tk
from tkinter import ttk, messagebox
import serial
import serial.tools.list_ports

BAUD = 115200

def list_ports():
    return [p.device for p in serial.tools.list_ports.comports()]

class App:
    def __init__(self, root):
        self.root = root
        root.title("Conveyor Jog (X & Y)")
        root.geometry("620x320")
        root.minsize(620, 320)

        # --- top: port picker ---
        top = ttk.Frame(root, padding=8)
        top.pack(fill="x")
        ttk.Label(top, text="Port:").pack(side="left")
        self.port_var = tk.StringVar()
        self.cmb = ttk.Combobox(top, textvariable=self.port_var, width=20, state="readonly")
        self.refresh_ports()
        self.cmb.pack(side="left", padx=6)
        ttk.Button(top, text="Refresh", command=self.refresh_ports).pack(side="left", padx=4)
        ttk.Button(top, text="Connect", command=self.connect).pack(side="left", padx=8)
        self.status = tk.StringVar(value="Not connected")
        ttk.Label(top, textvariable=self.status).pack(side="right")

        # --- middle: 4 arrow buttons in a cross layout ---
        mid = ttk.Frame(root, padding=16)
        mid.pack(expand=True)

        self.btn_up    = tk.Button(mid, text="⬆ UP",    font=("Segoe UI", 24), width=10)
        self.btn_down  = tk.Button(mid, text="⬇ DOWN",  font=("Segoe UI", 24), width=10)
        self.btn_left  = tk.Button(mid, text="⬅ LEFT",  font=("Segoe UI", 24), width=10)
        self.btn_right = tk.Button(mid, text="RIGHT ➡", font=("Segoe UI", 24), width=10)

        # grid cross
        self.btn_up.grid(   row=0, column=1, padx=10, pady=10)
        self.btn_left.grid( row=1, column=0, padx=10, pady=10)
        self.btn_right.grid(row=1, column=2, padx=10, pady=10)
        self.btn_down.grid( row=2, column=1, padx=10, pady=10)

        # mouse hold behavior
        self.btn_left.bind("<ButtonPress-1>",   lambda e: self.send("XL"))
        self.btn_left.bind("<ButtonRelease-1>", lambda e: self.send("XS"))
        self.btn_right.bind("<ButtonPress-1>",   lambda e: self.send("XR"))
        self.btn_right.bind("<ButtonRelease-1>", lambda e: self.send("XS"))
        self.btn_up.bind("<ButtonPress-1>",     lambda e: self.send("YU"))
        self.btn_up.bind("<ButtonRelease-1>",   lambda e: self.send("YS"))
        self.btn_down.bind("<ButtonPress-1>",   lambda e: self.send("YD"))
        self.btn_down.bind("<ButtonRelease-1>", lambda e: self.send("YS"))

        # keyboard (window must be focused)
        root.bind_all("<KeyPress-Left>",   lambda e: self.send("XL"))
        root.bind_all("<KeyRelease-Left>", lambda e: self.send("XS"))
        root.bind_all("<KeyPress-Right>",  lambda e: self.send("XR"))
        root.bind_all("<KeyRelease-Right>",lambda e: self.send("XS"))

        root.bind_all("<KeyPress-Up>",     lambda e: self.send("YU"))
        root.bind_all("<KeyRelease-Up>",   lambda e: self.send("YS"))
        root.bind_all("<KeyPress-Down>",   lambda e: self.send("YD"))
        root.bind_all("<KeyRelease-Down>", lambda e: self.send("YS"))

        # serial handle
        self.ser = None
        root.after(200, root.focus_force)

    def refresh_ports(self):
        ports = list_ports()
        self.cmb["values"] = ports
        self.port_var.set(ports[0] if ports else "")

    def connect(self):
        port = self.port_var.get().strip()
        if not port:
            messagebox.showwarning("No port", "Plug Arduino, click Refresh, choose a port.")
            return
        try:
            if self.ser and self.ser.is_open:
                self.ser.close()
            self.ser = serial.Serial(port, BAUD, timeout=0.02)
            self.status.set(f"Connected: {port}")
        except Exception as e:
            self.status.set(f"Connect error: {e}")
            messagebox.showerror("Connection failed", str(e))

    def send(self, cmd):
        if self.ser and self.ser.is_open:
            try:
                # commands are 2 bytes exactly: XR XL XS | YU YD YS
                self.ser.write(cmd.encode("ascii"))
                self.status.set(f"{self.ser.port}  sent: {cmd}")
            except Exception as e:
                self.status.set(f"Serial error: {e}")
        else:
            self.status.set("Not connected (choose port → Connect)")

if __name__ == "__main__":
    root = tk.Tk()
    App(root)
    root.mainloop()
