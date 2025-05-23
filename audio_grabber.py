import numpy as np
import sounddevice as sd
import time
import os
import serial

# Configuration
NUM_BARS = 50
SAMPLE_RATE = 44100
FFT_SIZE = 1024
SILENCE_THRESHOLD = 1e-4

# Frequency range
MIN_FREQ = 50       # Hz
MAX_FREQ = 3000     # Hz

# Audio sensitivity scaling (adjust to your system)
SENSITIVITY = 1.0   # 1.0 = normal, increase to amplify, decrease to reduce

# Smoothing parameters
ATTACK_RATE = 0.8
DECAY_RATE = 0.2

# Serial configuration
SERIAL_PORT = 'COM3'  # Change to your ESP32 serial port
BAUD_RATE = 460800

# Global variable to store smoothed magnitudes
smoothed_magnitudes = np.zeros(NUM_BARS)

# Initialize serial
try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0.01)
    print(f"Opened serial port {SERIAL_PORT}")
except Exception as e:
    print(f"Could not open serial port {SERIAL_PORT}: {e}")
    ser = None

def send_to_esp32(data):
    if ser and ser.is_open:
        try:
            byte_data = bytes([int(val * 255) for val in data])
            ser.write(byte_data)
        except Exception as e:
            print(f"Error sending to ESP32: {e}")

def audio_callback(indata, frames, time_, status):
    global smoothed_magnitudes
    if status:
        print(f"Sounddevice status: {status}")

    audio_data = indata[:, 0]
    signal_power = np.sqrt(np.mean(audio_data ** 2))

    if signal_power < SILENCE_THRESHOLD:
        target_magnitudes = np.zeros(NUM_BARS)
    else:
        fft_result = np.fft.rfft(audio_data)
        magnitudes = np.abs(fft_result)

        freq_resolution = SAMPLE_RATE / FFT_SIZE
        min_bin = int(MIN_FREQ / freq_resolution)
        max_bin = int(MAX_FREQ / freq_resolution)
        magnitudes = magnitudes[min_bin:max_bin + 1]

        # Apply sensitivity scaling BEFORE interpolation and normalization
        magnitudes = magnitudes * SENSITIVITY

        x_old = np.linspace(0, 1, len(magnitudes))
        x_new = np.linspace(0, 1, NUM_BARS)
        target_magnitudes = np.interp(x_new, x_old, magnitudes)

        max_mag = np.max(target_magnitudes)
        target_magnitudes = target_magnitudes / max_mag if max_mag > 0 else np.zeros(NUM_BARS)

    for i in range(NUM_BARS):
        if target_magnitudes[i] > smoothed_magnitudes[i]:
            smoothed_magnitudes[i] = (
                ATTACK_RATE * target_magnitudes[i] +
                (1 - ATTACK_RATE) * smoothed_magnitudes[i]
            )
        else:
            smoothed_magnitudes[i] = (
                DECAY_RATE * target_magnitudes[i] +
                (1 - DECAY_RATE) * smoothed_magnitudes[i]
            )

    send_to_esp32(smoothed_magnitudes)

def build_bars(height=20):
    bars = []
    bar_heights = (smoothed_magnitudes * height).astype(int)
    for row in range(height, 0, -1):
        line = ""
        for bh in bar_heights:
            line += "|" if bh >= row else " "
        bars.append(line)
    return bars

def main():
    global smoothed_magnitudes
    print("Starting audio stream... Press Ctrl+C to stop.")

    try:
        with sd.InputStream(channels=1, samplerate=SAMPLE_RATE, blocksize=FFT_SIZE, callback=audio_callback):
            os.system('cls' if os.name == 'nt' else 'clear')
            while True:
                bars_to_print = build_bars(height=20)
                print('\033[H', end='')  # Move cursor to top-left
                for line in bars_to_print:
                    print(line)
                time.sleep(0.01)
    except KeyboardInterrupt:
        print("\nStopped.")
    finally:
        if ser and ser.is_open:
            ser.close()

if __name__ == "__main__":
    main()
