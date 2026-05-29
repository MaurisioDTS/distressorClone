# distressorClone

*clon del Empirical Labs Distressor EL8, hecho en C++ con JUCE.*

<<<<<<< HEAD
![el plugin](https://github.com/user-attachments/assets/ce82e9b4-7be7-46de-9174-3aa30965bcc6)

## esto pa k es

bueno, la idea es tener un compresor/limitador con carácter tipo **Distressor**: knee suave que se estrecha según el ratio, detección en estéreo linkada o mid/side, saturación opcional y una interfaz web embebida en el plugin (sí, webview dentro del vst).

de momento es **work in progress**. suena, comprime y tiene meters, pero no es un clon fiel del hardware ni pretende serlo.

## controles

| parámetro | rango | qué hace |
|-----------|-------|----------|
| **Input** | −24 … +24 dB | ganancia de entrada |
| **Output** | −24 … +24 dB | ganancia de salida|
| **Ratio** | 1:1 … 25:1 | no has usado un comp nunca? |
| **Attack** | 0.1 … 300 ms | subida del envelope follower |
| **Release** | 10 … 3000 ms | bajada del envelope follower |
| **Dry / Wet** | 0 … 100 % | mezcla señal seca / procesada |

threshold fijo interno a **−24 dBFS** (de patranya, como el original más o menos).

## modos de detección

| modo | descripción |
|------|-------------|
| **Unlink** | cada canal con su propio detector y cadena de efectos |
| **Link** | detector común (max L/R), misma reducción en ambos |
| **M/S** | procesa mid y side por separado (solo estéreo) |

## distorsión

tres modos cicables desde la UI (botón *Distortion*):

| modo | rollo |
|------|-------|
| **Normal** | soft clip suave, casi transparente |
| **Dist 2** | saturación tipo válvula (tanh + 2º armónico) |
| **Dist 3** | saturación tipo cinta (cúbica + tanh, armónicos impares) |

> ojo: la distorsión solo entra en la cadena en modo **Unlink**. en Link y M/S de momento solo comprime.

## interfaz

la UI vive en `Resources/` (HTML + CSS + JS) y se carga con `juce::WebBrowserComponent`. knobs, botones cycle, VU meters de entrada/salida y gain reduction por canal.

## requisitos

- [JUCE](https://juce.com/) 8.x (módulos en ruta global o relativa según tu `.jucer`)
- **Windows:** Visual Studio 2022 + WebView2
- **Linux:** toolchain de compilación + dependencias de JUCE para plugins (`freetype`, `x11`, etc.)

## compilación

### windows

1. abre `Builds/VisualStudio2022/distressorClone.sln`
2. compila **Release | x64**
3. saca el `.vst3` de la carpeta de salida y mételo en tu carpeta de VST3

### linux

```bash
cd Builds/LinuxMakefile
./buildRelease.sh
```

*(asumo que tienes JUCE en la ruta que espera el Makefile)*

## usage

1. cargas el vst3 en tu daw favorito
2. mueves los knobs
3. si no te convence, cambias ratio/modo/distorsión y repites
4. miras los meters de GR para ver si te estás pasando

también compila como **standalone** por si quieres probarlo sin daw. (no sé porque carajos querrías hacer eso pero vale yo no juzgo)

## known issues

- la distorsión y el hi-pass están a medias (el bypass ni siquiera está cableado).
- en modo **Link** y **M/S** no pasa por la cadena de distorsión.
- el oversampling se inicializa pero aún no está metido en el path de audio.
- la UI depende de WebView2 en windows.
- por lo k sea, no he comparado A/B con un EL8 de verdad.

de todas formas, **claramente este plugin no está destinado a producción profesional**. no me hago cargo de mixes destrozados, oídos fundidos ni sesiones perdidas. *hubieras estudiado*.

## built with

- [JUCE](https://juce.com/) — framework de audio y la webview
- C++ — el dsp vive en `Source/PluginProcessor.cpp`
- HTML / CSS / JS — la cara bonita (o fea, según quien mire)

## posibles mejoras

cosas que tengo en la cabeza y que irán saliendo poco a poco:

- cablear el oversampling de verdad
- hi-pass filter y bypass de distorsión como en el hardware
- distorsión también en modos Link y M/S
- threshold ajustable (ahora es fijo, aunque creo que así se va a quedar)
- presets y delta monitoring (hay código comentado por ahí)
- comparar con un distressor real y llorar

## autor

- **yo** — *ola*

## licencia

tu ere loko? esto es un proyecto personal de aprendizaje.

---

*se alejan cositas 🗣️🗣️🗣️ 🔥🔥🔥*