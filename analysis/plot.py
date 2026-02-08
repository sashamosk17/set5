import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import os

os.makedirs("results/plots", exist_ok=True)

#сравнение N_t и F_t^0 для одного потока
df_single = pd.read_csv("results/single_stream.csv")

plt.figure(figsize=(12, 6))
plt.plot(df_single["fraction"] * 100, df_single["exact"],
         'b-o', label=r'$F_t^0$ (точное)', markersize=4, linewidth=2)
plt.plot(df_single["fraction"] * 100, df_single["hll_estimate"],
         'r--s', label=r'$N_t$ (HyperLogLog)', markersize=4, linewidth=2)
plt.xlabel("Обработанная часть потока (%)", fontsize=13)
plt.ylabel("Количество уникальных элементов", fontsize=13)
plt.title("График 1: Сравнение оценки HyperLogLog с точным значением", fontsize=14)
plt.legend(fontsize=12)
plt.grid(True, alpha=0.3)
plt.tight_layout()
plt.savefig("results/plots/graph1_comparison.png", dpi=150)
plt.show()

# E(N_t) ± σ(N_t)
df_stats = pd.read_csv("results/statistics.csv")

plt.figure(figsize=(12, 6))

fracs = df_stats["fraction"] * 100
mean_est = df_stats["mean_estimate"]
std_est = df_stats["std_estimate"]
mean_exact = df_stats["mean_exact"]

plt.plot(fracs, mean_exact, 'b-o', label=r'Среднее $F_t^0$',
         markersize=4, linewidth=2)
plt.plot(fracs, mean_est, 'r-s', label=r'$E(N_t)$',
         markersize=4, linewidth=2)
plt.fill_between(fracs,
                 mean_est - std_est,
                 mean_est + std_est,
                 alpha=0.3, color='red',
                 label=r'$E(N_t) \pm \sigma_{N_t}$')
plt.xlabel("Обработанная часть потока (%)", fontsize=13)
plt.ylabel("Количество уникальных элементов", fontsize=13)
plt.title(r"График 2: $E(N_t)$ с областью неопределённости $\pm\sigma$", fontsize=14)
plt.legend(fontsize=12)
plt.grid(True, alpha=0.3)
plt.tight_layout()
plt.savefig("results/plots/graph2_statistics.png", dpi=150)
plt.show()

# Относительная ошибка в зависимости от B
df_b = pd.read_csv("results/b_investigation.csv")

plt.figure(figsize=(10, 6))
plt.bar(df_b["B"].astype(str), df_b["mean_relative_error"],
        yerr=df_b["std_relative_error"], capsize=5,
        alpha=0.7, color='steelblue', label="Средняя отн. ошибка")
plt.plot(df_b["B"].astype(str), df_b["theoretical_1042"],
         'r--^', label=r'$1.04/\sqrt{m}$', markersize=8, linewidth=2)
plt.plot(df_b["B"].astype(str), df_b["theoretical_132"],
         'g--v', label=r'$1.3/\sqrt{m}$', markersize=8, linewidth=2)
plt.xlabel("B (число бит индекса)", fontsize=13)
plt.ylabel("Относительная ошибка", fontsize=13)
plt.title("Влияние параметра B на точность HyperLogLog", fontsize=14)
plt.legend(fontsize=12)
plt.grid(True, alpha=0.3, axis='y')
plt.tight_layout()
plt.savefig("results/plots/graph3_b_investigation.png", dpi=150)
plt.show()


#Распределение относительных ошибок
df_all = pd.read_csv("results/main_results.csv")
final_errors = df_all[df_all["fraction"] == 1.0]["relative_error"]

plt.figure(figsize=(10, 6))
plt.hist(final_errors, bins=15, alpha=0.7, color='steelblue', edgecolor='black')
plt.axvline(final_errors.mean(), color='red', linestyle='--',
            label=f'Среднее = {final_errors.mean():.4f}')
plt.xlabel("Относительная ошибка", fontsize=13)
plt.ylabel("Частота", fontsize=13)
plt.title("Распределение относительных ошибок HyperLogLog (полный поток)", fontsize=14)
plt.legend(fontsize=12)
plt.grid(True, alpha=0.3)
plt.tight_layout()
plt.savefig("results/plots/graph4_error_distribution.png", dpi=150)
plt.show()

# loglog beta
plt.figure(figsize=(12, 6))
plt.plot(df_single["fraction"] * 100, df_single["exact"],
         'b-o', label=r'$F_t^0$ (точное)', markersize=4, linewidth=2)
plt.plot(df_single["fraction"] * 100, df_single["hll_estimate"],
         'r--s', label='HyperLogLog (стандарт)', markersize=4, linewidth=2)
plt.plot(df_single["fraction"] * 100, df_single["hll_beta_estimate"],
         'g-.^', label='HyperLogLog (LogLog-Beta)', markersize=4, linewidth=2)
plt.xlabel("Обработанная часть потока (%)", fontsize=13)
plt.ylabel("Количество уникальных элементов", fontsize=13)
plt.title("График 5: Сравнение стандартного HyperLogLog и LogLog-Beta", fontsize=14)
plt.legend(fontsize=12)
plt.grid(True, alpha=0.3)
plt.tight_layout()
plt.savefig("results/plots/graph5_beta_comparison.png", dpi=150)
plt.show()


plt.figure(figsize=(10, 6))
plt.hist(df_all["relative_error"], bins=15, alpha=0.5, color='red',
         label='Стандартный', edgecolor='black')
plt.hist(df_all["beta_relative_error"], bins=15, alpha=0.5, color='green',
         label='LogLog-Beta', edgecolor='black')
plt.axvline(df_all["relative_error"].mean(), color='red', linestyle='--',
            label=f'Среднее (стандарт) = {df_all["relative_error"].mean():.4f}')
plt.axvline(df_all["beta_relative_error"].mean(), color='green', linestyle='--',
            label=f'Среднее (beta) = {df_all["beta_relative_error"].mean():.4f}')
plt.xlabel("Относительная ошибка", fontsize=13)
plt.ylabel("Частота", fontsize=13)
plt.title("Сравнение ошибок стандартного HyperLogLog и LogLog-Beta", fontsize=14)
plt.legend(fontsize=12)
plt.grid(True, alpha=0.3)
plt.tight_layout()
plt.savefig("results/plots/graph6_beta_error_comparison.png", dpi=150)
plt.show()