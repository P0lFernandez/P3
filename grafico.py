import numpy as np
import matplotlib.pyplot as plt
from scipy.io import wavfile

# Leer archivo WAV
fs, data = wavfile.read('prueba.wav')

# Convertir a mono si es estéreo
if data.ndim > 1:
    data = data[:, 0]

# Extraer trama de 1.65s a 1.68s (30 ms)
start_sample = int(1.65 * fs)
end_sample = int(1.68 * fs)
frame = data[start_sample:end_sample]

# Normalizar si es necesario (por ejemplo, int16)
if np.issubdtype(frame.dtype, np.integer):
    frame = frame / np.max(np.abs(frame))

# Autocorrelación completa
autocorr = np.correlate(frame, frame, mode='full')

# Solo conservar parte positiva (lags >= 0)
mid = len(autocorr) // 2
autocorr = autocorr[mid:]
lags = np.arange(0, len(autocorr))  # en muestras

# Convertir lags a milisegundos
time_lags_ms = (lags / fs) * 1000

# Graficar
plt.figure(figsize=(8, 5))
plt.plot(time_lags_ms, autocorr, linewidth=1)
plt.title('Autocorrelación de la trama (30 ms)')
plt.xlabel('Retardo (ms)')
plt.ylabel('Autocorrelación')
plt.grid(True)
plt.tight_layout()
plt.show()