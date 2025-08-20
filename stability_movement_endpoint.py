from fastapi import FastAPI, HTTPException
import serial
import time

app = FastAPI()

# Serial port settings
SERIAL_PORT = '/dev/ttyUSB0'  
BAUD_RATE = 115200


@app.get("/send-command")
def send_command(cmd: str = "F5"):
 # Open serial connection
    ser = serial.Serial(
        port=SERIAL_PORT,
        baudrate=BAUD_RATE,
        timeout=1,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        bytesize=serial.EIGHTBITS
    )
    time.sleep(2)  # Wait for the serial connection to initialize

    try:
        # Send command
        ser.write(cmd.encode())
        print(f"Sent: {cmd}")

        # Read response
        response = ser.readline()
        ser.close()

        return {"sent": cmd, "received": response.decode("utf-8", errors="ignore")}
    
    except serial.SerialException as e:
        raise HTTPException(status_code=500, detail=f"Serial error: {e}")
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Unexpected error: {e}")


@app.get("/ping")
def ping():
    return {"status": "ok", "port": SERIAL_PORT, "baud": BAUD_RATE}


@app.get("/health")
def health():
    """Check if serial connection is alive"""
    #Opening port 
    ser = serial.Serial(
        port=SERIAL_PORT,
        baudrate=BAUD_RATE,
        timeout=1,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        bytesize=serial.EIGHTBITS
    )

    try:
        ser.write(b"\n")
        _ = ser.readline()
        return {"status": "healthy", "serial": "connected"}
    except Exception as e:
        return {"status": "unhealthy", "error": str(e)}
