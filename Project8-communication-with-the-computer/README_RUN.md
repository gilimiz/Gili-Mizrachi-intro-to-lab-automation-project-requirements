Run instructions for Project8 final GUI

1. (Optional) Create and activate conda env:

```powershell
conda create --name intro python=3.11
conda activate intro
```

2. Install requirements:

```powershell
pip install -r requirements.txt
```

3. Upload `project8/project8.ino` to your Arduino using the Arduino IDE.

4. Run the GUI:

```powershell
python project8_gui_final.py
```

Notes:
- The GUI expects newline-terminated integer durations in milliseconds.
- Device state messages are single-line tokens `0`, `1`, `2`.
- If `pyserial` is not installed, the GUI will prompt to install it.
