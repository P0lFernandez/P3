import numpy as np
import matplotlib.pyplot as plt
from scipy.io import wavfile

fs, data = wavfile.read('prueba.wav')

if data.ndim > 1:
    data = data[:, 0]

start_sample = int(1.65 * fs)
end_sample = int(1.68 * fs)
frame = data[start_sample:end_sample]

if np.issubdtype(frame.dtype, np.integer):
    frame = frame / np.max(np.abs(frame))

autocorr = np.correlate(frame, frame, mode='full')

mid = len(autocorr) // 2
autocorr = autocorr[mid:]
lags = np.arange(0, len(autocorr))  # en muestras

time_lags_ms = (lags / fs) * 1000

plt.figure(figsize=(8, 5))
plt.plot(time_lags_ms, autocorr, linewidth=1)
plt.title('Autocorrelación de la trama (30 ms)')
plt.xlabel('Retardo (ms)')
plt.ylabel('Autocorrelación')
plt.grid(True)
plt.tight_layout()
plt.show()