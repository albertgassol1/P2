import matplotlib.pyplot as plt
import numpy as np
import wave
import sys


spf = wave.open("pav_2341.wav", "r")
fig, axs = plt.subplots(2)
fig.suptitle('Senyal normal y senyal con ceros')
# Extract Raw Audio from Wav File
signal = spf.readframes(-1)
signal = np.fromstring(signal, "Int16")


# If Stereo
if spf.getnchannels() == 2:
    print("Just mono files")
    sys.exit(0)

axs[0].plot(signal, 'tab:green')

spf2 = wave.open("pav_2341_silence.wav", "r")
# Extract Raw Audio from Wav File
signal2 = spf2.readframes(-1)
signal2 = np.fromstring(signal2, "Int16")
if spf2.getnchannels() == 2:
    print("Just mono files")
    sys.exit(0)


axs[1].plot(signal2, 'tab:pink')

plt.show()