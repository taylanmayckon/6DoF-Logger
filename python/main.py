import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

# AJUSTA O NOME DO ARQUIVO
filename = 'python/LOG9.csv'
dt = 0.1

# Verifica se achou o arquivo
try:
    df_leitura = pd.read_csv(filename)
except FileNotFoundError:
    print(f"O arquivo '{filename}' não foi encontrado.")
    exit()

df_leitura['tempo'] = df_leitura['index'] * dt # Gera label do tempo


# -> ACELEROMETRO
plt.figure(figsize=(12, 6))
plt.plot(df_leitura['tempo'], df_leitura['accel_x'], label='Aceleração X (g)')
plt.plot(df_leitura['tempo'], df_leitura['accel_y'], label='Aceleração Y (g)')
plt.plot(df_leitura['tempo'], df_leitura['accel_z'], label='Aceleração Z (g)')
plt.title('Acelerômetro', fontsize=16)
plt.xlabel('Tempo (s)', fontsize=12)
plt.ylabel('Aceleração (g)', fontsize=12)
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig('python/generated/acelerometro.png')

# -> GIROSCÓPIO
plt.figure(figsize=(12, 6))
plt.plot(df_leitura['tempo'], df_leitura['gyro_x'], label='Giroscópio X (°/s)')
plt.plot(df_leitura['tempo'], df_leitura['gyro_y'], label='Giroscópio Y (°/s)')
plt.plot(df_leitura['tempo'], df_leitura['gyro_z'], label='Giroscópio Z (°/s)')
plt.title('Giroscópio', fontsize=16)
plt.xlabel('Tempo (s)', fontsize=12)
plt.ylabel('Velocidade Angular (°/s)', fontsize=12)
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig('python/generated/giroscopio.png')

# -> COMPARATIVO PITCH E ROLL PURO COM O FILTRO DE KALMANN
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 10), sharex=True)
# Subplot para o Pitch
ax1.plot(df_leitura['tempo'], df_leitura['pitch'], label='Pitch (Original)', linestyle=':')
ax1.plot(df_leitura['tempo'], df_leitura['kf_pitch'], label='Pitch (Filtro de Kalman)', linewidth=2)
ax1.set_title('Pitch', fontsize=14)
ax1.set_ylabel('Ângulo (°)', fontsize=12)
ax1.legend()
ax1.grid(True)
# Subplot para o Roll
ax2.plot(df_leitura['tempo'], df_leitura['roll'], label='Roll (Original)', linestyle=':')
ax2.plot(df_leitura['tempo'], df_leitura['kf_roll'], label='Roll (Filtro de Kalman)', linewidth=2)
ax2.set_title('Roll', fontsize=14)
ax2.set_xlabel('Tempo (s)', fontsize=12)
ax2.set_ylabel('Ângulo (°)', fontsize=12)
ax2.legend()
ax2.grid(True)

plt.tight_layout()
plt.savefig('python/generated/comparativo_kalmann_filter.png')

plt.show()

print("Gráficos gerados e salvos no path 'python/generated'.")