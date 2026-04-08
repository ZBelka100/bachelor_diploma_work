# Diploma WHT Audio Project

Исследовательский проект по теме:

**«Исследование применимости преобразования Адамара для построения псевдоспектра аудиосигналов и сравнение с классическим преобразованием Фурье»**

Проект реализует:
- кратковременное преобразование Уолша–Адамара (STWHT / WHT по кадрам),
- построение псевдоспектрограммы,
- обратное восстановление сигнала,
- сравнение с классическим STFT,
- экспорт результатов для последующего анализа и визуализации.

---

## Что делает проект

Для входного WAV-файла проект строит два представления сигнала:

1. **WHT-псевдоспектр**
   - ось X: время,
   - ось Y: секвенция (`sequency`) / псевдочастота,
   - значения: мощность и dB.

2. **STFT-спектрограмму**
   - ось X: время,
   - ось Y: линейная частота в Гц,
   - значения: мощность и dB.

Также проект умеет:
- восстанавливать сигнал из WHT-коэффициентов,
- считать метрики качества восстановления,
- сохранять результаты в CSV,
- строить PNG-графики через Python.

---

## Структура проекта

```text
.
├── apps/
│   └── main.cpp
├── include/
│   └── diploma/
│       ├── audio_io.hpp
│       ├── export.hpp
│       ├── framing.hpp
│       ├── metrics.hpp
│       ├── pseudospectrum.hpp
│       ├── reconstruction.hpp
│       ├── stft.hpp
│       ├── types.hpp
│       ├── wht.hpp
│       └── window.hpp
├── src/
│   ├── audio_io.cpp
│   ├── export.cpp
│   ├── framing.cpp
│   ├── metrics.cpp
│   ├── pseudospectrum.cpp
│   ├── reconstruction.cpp
│   ├── stft.cpp
│   ├── wht.cpp
│   └── window.cpp
├── tests/
│   ├── test_metrics.cpp
│   └── test_wht.cpp
├── scripts/
│   ├── generate_machine_noise.py
│   ├── plot_pseudospectrum.py
│   └── run_experiments.sh
├── data/
│   ├── wav/
│   ├── out/
│   └── plots/
├── CMakeLists.txt
├── requirements.txt
└── Dockerfile# bachelor_diploma_work
# bachelor_diploma_work
