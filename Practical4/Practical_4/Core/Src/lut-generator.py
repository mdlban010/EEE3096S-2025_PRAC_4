import numpy as np
import matplotlib.pyplot as plt
from scipy.io import wavfile

# Parameters
NS = 128  # Number of samples
MAX_VAL = 4095  # 12-bit resolution (2^12 - 1)

# ============================================
# Generate Waveform LUTs
# ============================================

# 1. Sine Wave
sine_wave = np.sin(2 * np.pi * np.arange(NS) / NS)
sine_wave_scaled = ((sine_wave + 1) / 2 * MAX_VAL).astype(int)

# 2. Sawtooth Wave
sawtooth_wave = np.arange(NS) / NS
sawtooth_wave_scaled = (sawtooth_wave * MAX_VAL).astype(int)

# 3. Triangle Wave
triangle_wave = 2 * np.abs(2 * (np.arange(NS) / NS - np.floor(np.arange(NS) / NS + 0.5))) - 1
triangle_wave = (triangle_wave + 1) / 2
triangle_wave_scaled = (triangle_wave * MAX_VAL).astype(int)

# ============================================
# Plot Waveforms
# ============================================
fig, axes = plt.subplots(3, 1, figsize=(10, 8))

axes[0].plot(sine_wave_scaled, 'b-', linewidth=2)
axes[0].set_title('Sine Wave LUT')
axes[0].set_ylabel('Value (0-4095)')
axes[0].grid(True)

axes[1].plot(sawtooth_wave_scaled, 'r-', linewidth=2)
axes[1].set_title('Sawtooth Wave LUT')
axes[1].set_ylabel('Value (0-4095)')
axes[1].grid(True)

axes[2].plot(triangle_wave_scaled, 'g-', linewidth=2)
axes[2].set_title('Triangle Wave LUT')
axes[2].set_ylabel('Value (0-4095)')
axes[2].set_xlabel('Sample Index')
axes[2].grid(True)

plt.tight_layout()
plt.savefig('waveforms.png', dpi=300, bbox_inches='tight')
plt.show()

# ============================================
# Process WAV Files
# ============================================
wav_files = ['drum.wav', 'guitar.wav', 'piano.wav']
wav_luts = {}

fig_wav, axes_wav = plt.subplots(3, 1, figsize=(10, 8))

for idx, wav_file in enumerate(wav_files):
    try:
        # Read WAV file
        sample_rate, data = wavfile.read(wav_file)
        
        print(f"\n{wav_file}:")
        print(f"  Sample Rate: {sample_rate} Hz")
        print(f"  Shape: {data.shape}")
        print(f"  Data type: {data.dtype}")
        
        # Convert to mono if stereo
        if len(data.shape) > 1:
            data = data.mean(axis=1)
        
        # Normalize and scale to 12-bit range
        # First normalize to [-1, 1] or [0, 1] depending on data type
        if data.dtype == np.int16:
            data_normalized = data.astype(float) / 32768.0
        elif data.dtype == np.int32:
            data_normalized = data.astype(float) / 2147483648.0
        else:
            data_normalized = data.astype(float)
        
        # Scale to [0, 4095]
        data_scaled = ((data_normalized + 1) / 2 * MAX_VAL).astype(int)
        data_scaled = np.clip(data_scaled, 0, MAX_VAL)
        
        # Resample to NS samples using linear interpolation
        indices = np.linspace(0, len(data_scaled) - 1, NS)
        lut = np.interp(indices, np.arange(len(data_scaled)), data_scaled).astype(int)
        
        wav_luts[wav_file.replace('.wav', '')] = lut
        
        # Plot
        axes_wav[idx].plot(lut, linewidth=2)
        axes_wav[idx].set_title(f'{wav_file} LUT')
        axes_wav[idx].set_ylabel('Value (0-4095)')
        axes_wav[idx].grid(True)
        
    except Exception as e:
        print(f"Error processing {wav_file}: {e}")

axes_wav[2].set_xlabel('Sample Index')
plt.tight_layout()
plt.savefig('wav_files.png', dpi=300, bbox_inches='tight')
plt.show()

# ============================================
# Print C Code Arrays
# ============================================
output_lines = []

output_lines.append("="*60)
output_lines.append("C CODE FOR main.c")
output_lines.append("="*60)

output_lines.append(f"\n// Number of samples")
output_lines.append(f"#define NS {NS}")

output_lines.append("\n// Sine Wave LUT")
output_lines.append(f"uint32_t Sin_LUT[NS] = {{")
for i in range(0, NS, 8):
    values = ', '.join(str(val) for val in sine_wave_scaled[i:min(i+8, NS)])
    output_lines.append(f"    {values}{',' if i+8 < NS else ''}")
output_lines.append("};")

output_lines.append("\n// Sawtooth Wave LUT")
output_lines.append(f"uint32_t Saw_LUT[NS] = {{")
for i in range(0, NS, 8):
    values = ', '.join(str(val) for val in sawtooth_wave_scaled[i:min(i+8, NS)])
    output_lines.append(f"    {values}{',' if i+8 < NS else ''}")
output_lines.append("};")

output_lines.append("\n// Triangle Wave LUT")
output_lines.append(f"uint32_t Triangle_LUT[NS] = {{")
for i in range(0, NS, 8):
    values = ', '.join(str(val) for val in triangle_wave_scaled[i:min(i+8, NS)])
    output_lines.append(f"    {values}{',' if i+8 < NS else ''}")
output_lines.append("};")

# Print WAV file LUTs
for name, lut in wav_luts.items():
    output_lines.append(f"\n// {name.capitalize()} LUT")
    output_lines.append(f"uint32_t {name.capitalize()}_LUT[NS] = {{")
    for i in range(0, NS, 8):
        values = ', '.join(str(val) for val in lut[i:min(i+8, NS)])
        output_lines.append(f"    {values}{',' if i+8 < NS else ''}")
    output_lines.append("};")

output_lines.append("\n" + "="*60)
output_lines.append("Copy of the LUT for each waveform, they go to main.c")
output_lines.append("="*60)

# Print to console
for line in output_lines:
    print(line)

# Save to file
with open('LUT_arrays.txt', 'w') as f:
    f.write('\n'.join(output_lines))

print("\n✓ LUT arrays saved to 'LUT_arrays.txt'")
print("✓ Waveform plots saved to 'waveforms.png' and 'wav_files.png'")