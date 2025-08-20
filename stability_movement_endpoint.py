from fastapi import FastAPI
import serial
import time

# --- Serial setup ---
PORT = "/dev/ttyUSB0"  
BAUD = 115200
ser = serial.Serial(PORT, BAUD, timeout=1)
time.sleep(2)  # reset Arduino

# --- FastAPI app ---
app = FastAPI()

@app.get("/send")
def send_command(cmd: str):
    if not cmd:
        return {"error": "No command provided"}

    ser.write((cmd + "\n").encode())
    response = ser.readline().decode(errors="ignore").strip()
    return {"sent": cmd, "response": response}

@app.get("/ping")
def ping():
    return {"status": "ok", "port": PORT, "baud": BAUD}

@app.get("/health")
def health():
    """Check if serial connection is alive"""
    try:
        ser.write(b"\n")
        _ = ser.readline()
        return {"status": "healthy", "serial": "connected"}
    except Exception as e:
        return {"status": "unhealthy", "error": str(e)}
