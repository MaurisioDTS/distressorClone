import * as Juce from "./juce/index.js";

// ------------------------------------------------------------------
//  Utilidades comunes
// ------------------------------------------------------------------

const formatValue = (value, unit) => {
    if (unit === "dB") return `${value >= 0 ? "+" : ""}${value.toFixed(1)} dB`;
    if (unit === "ms") {
        if (value >= 1000) return `${(value / 1000).toFixed(2)} s`;
        return `${value.toFixed(1)} ms`;
    }
    return value.toFixed(2);
};

// ------------------------------------------------------------------
//  Defs SVG compartidas (gradiente para las "caps" de los knobs)
// ------------------------------------------------------------------
const svgDefs = `
<defs>
    <radialGradient id="knobCapGradient" cx="35%" cy="30%" r="70%">
        <stop offset="0%" stop-color="#fefefe"/>
        <stop offset="55%" stop-color="#bfbfbf"/>
        <stop offset="100%" stop-color="#5e5e5e"/>
    </radialGradient>
</defs>`;

// ------------------------------------------------------------------
//  Rotary knob
// ------------------------------------------------------------------

class RotaryKnob {
    constructor(element, sliderState, unit) {
        this.el = element;
        this.slider = sliderState;
        this.unit = unit;

        this.minAngle = -135; // grados
        this.maxAngle = 135;

        this.el.classList.add("knob");
        this.el.innerHTML = `
            <svg viewBox="0 0 100 100" preserveAspectRatio="xMidYMid meet">
                ${svgDefs}
                <path class="track" d="${this.arcPath(this.minAngle, this.maxAngle)}"/>
                <path class="value-arc" d="${this.arcPath(this.minAngle, this.minAngle)}"/>
                <circle class="cap" cx="50" cy="50" r="26"/>
                <line class="indicator" x1="50" y1="50" x2="50" y2="30"
                      transform="rotate(${this.minAngle} 50 50)"/>
            </svg>
            <span class="knob-readout">--</span>
        `;

        this.valueArc = this.el.querySelector(".value-arc");
        this.indicator = this.el.querySelector(".indicator");
        this.readout = this.el.querySelector(".knob-readout");

        this.slider.valueChangedEvent.addListener(() => this.refresh());
        this.slider.propertiesChangedEvent.addListener(() => this.refresh());

        this.attachMouseHandlers();
        this.refresh();
    }

    arcPath(fromDeg, toDeg) {
        const r = 40;
        const cx = 50;
        const cy = 50;
        const toRad = (d) => ((d - 90) * Math.PI) / 180; // 0deg -> arriba
        const sweep = toDeg - fromDeg;
        const largeArc = Math.abs(sweep) > 180 ? 1 : 0;
        const sweepFlag = sweep >= 0 ? 1 : 0;
        const x1 = cx + r * Math.cos(toRad(fromDeg));
        const y1 = cy + r * Math.sin(toRad(fromDeg));
        const x2 = cx + r * Math.cos(toRad(toDeg));
        const y2 = cy + r * Math.sin(toRad(toDeg));
        return `M ${x1} ${y1} A ${r} ${r} 0 ${largeArc} ${sweepFlag} ${x2} ${y2}`;
    }

    normalised() {
        // SliderState expone el valor normalizado 0..1
        return this.slider.getNormalisedValue();
    }

    refresh() {
        const n = this.normalised();
        const angle = this.minAngle + n * (this.maxAngle - this.minAngle);
        this.valueArc.setAttribute("d", this.arcPath(this.minAngle, angle));
        this.indicator.setAttribute("transform", `rotate(${angle} 50 50)`);
        this.readout.textContent = formatValue(this.slider.getScaledValue(), this.unit);
    }

    attachMouseHandlers() {
        let dragging = false;
        let startY = 0;
        let startValue = 0;

        const beginDrag = (e) => {
            e.preventDefault();
            dragging = true;
            startY = e.clientY ?? (e.touches && e.touches[0].clientY) ?? 0;
            startValue = this.normalised();
            this.el.classList.add("dragging");
            this.slider.sliderDragStarted();
            window.addEventListener("mousemove", onMove);
            window.addEventListener("mouseup", endDrag);
        };

        const onMove = (e) => {
            if (!dragging) return;
            const y = e.clientY ?? (e.touches && e.touches[0].clientY) ?? startY;
            const dy = startY - y; // arriba = positivo
            // sensibilidad: 200 px de recorrido = rango completo
            let next = startValue + dy / 200;
            if (e.shiftKey) next = startValue + dy / 800; // fine-tune
            next = Math.max(0, Math.min(1, next));
            this.slider.setNormalisedValue(next);
        };

        const endDrag = () => {
            if (!dragging) return;
            dragging = false;
            this.el.classList.remove("dragging");
            this.slider.sliderDragEnded();
            window.removeEventListener("mousemove", onMove);
            window.removeEventListener("mouseup", endDrag);
        };

        const onDoubleClick = () => {
            // Doble clic: reset al default (asumimos 0 normalizado equivale a min;
            // no tenemos el default aqui, pero lo dejamos estable).
            // En JUCE el default suele estar reflejado en getScaledValue al cargar.
            // Aqui volvemos al centro del rango como heuristica.
            this.slider.sliderDragStarted();
            this.slider.setNormalisedValue(0.5);
            this.slider.sliderDragEnded();
        };

        this.el.addEventListener("mousedown", beginDrag);
        this.el.addEventListener("dblclick", onDoubleClick);
        this.el.addEventListener("wheel", (e) => {
            e.preventDefault();
            const delta = -Math.sign(e.deltaY) * (e.shiftKey ? 0.005 : 0.02);
            const next = Math.max(0, Math.min(1, this.normalised() + delta));
            this.slider.sliderDragStarted();
            this.slider.setNormalisedValue(next);
            this.slider.sliderDragEnded();
        }, { passive: false });
    }
}

// ------------------------------------------------------------------
//  Button groups conectados a ComboBoxState (choice parameters)
// ------------------------------------------------------------------

class ChoiceButtonGroup {
    constructor(container, comboState) {
        this.container = container;
        this.combo = comboState;
        this.buttons = [];

        this.combo.propertiesChangedEvent.addListener(() => this.renderButtons());
        this.combo.valueChangedEvent.addListener(() => this.refreshActive());

        this.renderButtons();
    }

    renderButtons() {
        this.container.innerHTML = "";
        this.buttons = [];
        const choices = this.combo.properties.choices || [];
        choices.forEach((label, idx) => {
            const btn = document.createElement("button");
            btn.className = "pill-btn";
            btn.textContent = label;
            btn.addEventListener("click", () => this.combo.setChoiceIndex(idx));
            this.container.appendChild(btn);
            this.buttons.push(btn);
        });
        this.refreshActive();
    }

    refreshActive() {
        const selected = this.combo.getChoiceIndex();
        this.buttons.forEach((b, i) => {
            b.classList.toggle("active", i === selected);
        });
    }
}

// ------------------------------------------------------------------
//  Ratio ciclo y pantallita LCD
// ------------------------------------------------------------------

class RatioCycleControl {
    constructor(button, displayValueEl, sliderState, presets) {
        this.button = button;
        this.display = displayValueEl;
        this.slider = sliderState;
        this.presets = presets; // valores en unidades del par�metro

        this.slider.valueChangedEvent.addListener(() => this.refreshDisplay());
        this.slider.propertiesChangedEvent.addListener(() => this.refreshDisplay());

        this.button.addEventListener("click", () => this.cycleNext());
        this.refreshDisplay();
    }

    setRatio(value) {
        const { start, end, skew } = this.slider.properties;
        if (!(end > start)) return;
        const clamped = Math.max(start, Math.min(end, value));
        const normalised = Math.pow((clamped - start) / (end - start), skew || 1);
        this.slider.sliderDragStarted();
        this.slider.setNormalisedValue(normalised);
        this.slider.sliderDragEnded();
    }

    cycleNext() {
        const current = this.slider.getScaledValue();
        // siguiente preset estrictamente mayor que el actual (con tolerancia);
        // si no hay, vuelve al primero (wrap-around).
        const next = this.presets.find(v => v > current + 0.01);
        this.setRatio(next !== undefined ? next : this.presets[0]);
    }

    refreshDisplay() {
        const v = this.slider.getScaledValue();
        // si el valor es practicamente entero, lo mostramos sin decimales
        const text = Math.abs(v - Math.round(v)) < 0.05
            ? `${Math.round(v)}`
            : v.toFixed(1);
        this.display.textContent = text;
    }
}

// ------------------------------------------------------------------
//  Inicializacion cuando __JUCE__ esta listo
// ------------------------------------------------------------------

const bootstrap = () => {
    // --- knobs ---
    document.querySelectorAll(".knob").forEach((el) => {
        const sliderName = el.dataset.slider;
        const unit = el.dataset.unit || "";
        const state = Juce.getSliderState(sliderName);
        new RotaryKnob(el, state, unit);
    });

    // --- choice groups (distortionMode, mode) ---
    new ChoiceButtonGroup(
        document.getElementById("distortionButtons"),
        Juce.getComboBoxState("distortionMode")
    );

    new ChoiceButtonGroup(
        document.getElementById("modeButtons"),
        Juce.getComboBoxState("mode")
    );

    // --- ratio: bot�n unico ciclico + display LCD ---
    new RatioCycleControl(
        document.getElementById("ratioCycleBtn"),
        document.querySelector("#ratioDisplay .lcd-value"),
        Juce.getSliderState("ratio"),
        [2, 4, 6, 10, 20]
    );

    // --- VU meters ---
    const meterFills = {
        inL: document.querySelector('[data-meter="inL"] .meter-fill'),
        inR: document.querySelector('[data-meter="inR"] .meter-fill'),
        outL: document.querySelector('[data-meter="outL"] .meter-fill'),
        outR: document.querySelector('[data-meter="outR"] .meter-fill'),
        gr: document.querySelector('[data-meter="gr"] .meter-fill'),
    };

    // Mapea dB (-60..+6) a porcentaje (0..100) para meters in/out
    const dbToPercent = (db, minDb = -60, maxDb = 6) => {
        if (!isFinite(db)) return 0;
        const clamped = Math.max(minDb, Math.min(maxDb, db));
        return ((clamped - minDb) / (maxDb - minDb)) * 100;
    };

    window.__JUCE__.backend.addEventListener("meters", (payload) => {
        if (!payload) return;
        if (typeof payload.inL === "number") meterFills.inL.style.height = dbToPercent(payload.inL) + "%";
        if (typeof payload.inR === "number") meterFills.inR.style.height = dbToPercent(payload.inR) + "%";
        if (typeof payload.outL === "number") meterFills.outL.style.height = dbToPercent(payload.outL) + "%";
        if (typeof payload.outR === "number") meterFills.outR.style.height = dbToPercent(payload.outR) + "%";
        // GR viene en dB positivos (0 = sin reduccion, 24 = maxima)
        if (typeof payload.gr === "number") {
            const pct = Math.max(0, Math.min(1, payload.gr / 24)) * 100;
            meterFills.gr.style.height = pct + "%";
        }
    });
};

if (window.__JUCE__ && window.__JUCE__.backend) {
    bootstrap();
} else {
    // fallback por si el navegador cargase antes que el backend este listo
    window.addEventListener("DOMContentLoaded", bootstrap);
}
