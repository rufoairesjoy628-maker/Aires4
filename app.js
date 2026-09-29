
import { initializeApp } from "https://www.gstatic.com/firebasejs/12.7.0/firebase-app.js";
import {
    getDatabase,
    ref,
    onValue
} from "https://www.gstatic.com/firebasejs/12.7.0/firebase-database.js";

// =========================================================
// FIREBASE CONFIG - AIRES
// =========================================================

const firebaseConfig = {
    apiKey: "AIzaSyB0Yy_lveQZwd5rYiuYZglmGXvJ-aglpXY",
    authDomain: "aires-web.firebaseapp.com",
    databaseURL: "https://aires-web-default-rtdb.europe-west1.firebasedatabase.app",
    projectId: "aires-web",
    storageBucket: "aires-web.firebasestorage.app",
    messagingSenderId: "565744090451",
    appId: "1:565744090451:web:3f72fd1c6445e559cf75a6",
    measurementId: "G-4DGQ5ZH5JR"
};

const firebaseApp = initializeApp(firebaseConfig);
const database = getDatabase(firebaseApp);
const dataRef = ref(database, "ESP32_Data");

// =========================================================
// GLOBALS
// =========================================================

let allSensorData = {};
let selectedDate = "";
let sensorChart = null;

// =========================================================
// DOM
// =========================================================

const statusElement = document.getElementById("firebaseStatus");
const currentTemperature = document.getElementById("currentTemperature");
const currentHumidity = document.getElementById("currentHumidity");
const latestDateTime = document.getElementById("latestDateTime");
const graphDate = document.getElementById("graphDate");
const historyDate = document.getElementById("historyDate");
const historyBody = document.getElementById("historyTableBody");
const recordCount = document.getElementById("recordCount");
const toggleHistory = document.getElementById("toggleHistory");
const historyContent = document.getElementById("historyContent");

// =========================================================
// STATUS
// =========================================================

function setStatus(text, connected) {
    if (!statusElement) return;

    statusElement.textContent = text;

    statusElement.classList.toggle("status-connected", connected);
    statusElement.classList.toggle("status-error", !connected);

    const statusSmall = document.getElementById("statusSmall");
    if (statusSmall) {
        statusSmall.textContent = connected ? "REALTIME DATABASE ONLINE" : "CONNECTION ERROR";
    }
}

// =========================================================
// HELPERS
// =========================================================

function toNumber(value) {
    const number = Number(value);
    return Number.isFinite(number) ? number : null;
}

function formatNumber(value) {
    if (value === null || value === undefined) return "--";
    const number = Number(value);
    return Number.isFinite(number) ? number.toFixed(1) : "--";
}

function getDateList(data) {
    return Object.keys(data || {})
        .filter(key =>
            data[key] &&
            typeof data[key] === "object"
        )
        .sort()
        .reverse();
}

// =========================================================
// LATEST READING
// =========================================================

function getLatestReading(data) {
    let latest = null;

    const dates = Object.keys(data || {}).sort();

    for (const date of dates) {
        const times = Object.keys(data[date] || {}).sort();

        for (const time of times) {
            const reading = data[date][time];

            if (!reading || typeof reading !== "object") continue;

            const temperature = toNumber(reading.temperature);
            const humidity = toNumber(reading.humidity);

            if (temperature === null && humidity === null) continue;

            latest = {
                date,
                time,
                temperature,
                humidity
            };
        }
    }

    return latest;
}

// =========================================================
// READINGS FOR DATE
// =========================================================

function getReadingsForDate(date) {
    const result = [];

    if (!date || !allSensorData[date]) return result;

    const dayData = allSensorData[date];

    Object.keys(dayData)
        .filter(time =>
            dayData[time] &&
            typeof dayData[time] === "object"
        )
        .sort()
        .forEach(time => {
            const reading = dayData[time];

            const temperature = toNumber(reading.temperature);
            const humidity = toNumber(reading.humidity);

            if (temperature === null && humidity === null) return;

            result.push({
                time,
                temperature,
                humidity
            });
        });

    return result;
}

// =========================================================
// DATE SELECTS
// =========================================================

function populateDateSelects() {
    const dates = getDateList(allSensorData);

    [graphDate, historyDate].forEach(select => {
        if (!select) return;

        select.innerHTML = "";

        if (dates.length === 0) {
            const option = document.createElement("option");
            option.value = "";
            option.textContent = "No dates available";
            select.appendChild(option);
            return;
        }

        dates.forEach(date => {
            const option = document.createElement("option");
            option.value = date;
            option.textContent = date;
            select.appendChild(option);
        });

        select.value = selectedDate || dates[0];
    });
}

// =========================================================
// CURRENT READING
// =========================================================

function updateCurrentReading() {
    const latest = getLatestReading(allSensorData);

    if (!latest) {
        if (currentTemperature) currentTemperature.textContent = "-- °C";
        if (currentHumidity) currentHumidity.textContent = "-- %";
        if (latestDateTime) latestDateTime.textContent = "Waiting for sensor data";
        return;
    }

    if (currentTemperature) {
        currentTemperature.textContent = `${formatNumber(latest.temperature)} °C`;
    }

    if (currentHumidity) {
        currentHumidity.textContent = `${formatNumber(latest.humidity)} %`;
    }

    if (latestDateTime) {
        latestDateTime.textContent = `${latest.date}  /  ${latest.time}`;
    }
}

// =========================================================
// CHART
// =========================================================

function updateChart() {
    if (typeof Chart === "undefined") {
        console.error("Chart.js is not loaded.");
        return;
    }

    const date = graphDate?.value || selectedDate;
    const readings = getReadingsForDate(date);

    const canvas = document.getElementById("sensorChart");
    if (!canvas) return;

    if (sensorChart) {
        sensorChart.destroy();
        sensorChart = null;
    }

    sensorChart = new Chart(canvas, {
        type: "line",
        data: {
            labels: readings.map(item => item.time),
            datasets: [
                {
                    label: "Temperature (°C)",
                    data: readings.map(item => item.temperature),
                    yAxisID: "temperature",
                    tension: 0.3,
                    borderWidth: 3,
                    pointRadius: 3,
                    pointHoverRadius: 6,
                    borderColor: "#e96b9b",
                    backgroundColor: "rgba(233,107,155,0.10)",
                    spanGaps: true
                },
                {
                    label: "Humidity (%)",
                    data: readings.map(item => item.humidity),
                    yAxisID: "humidity",
                    tension: 0.3,
                    borderWidth: 3,
                    pointRadius: 3,
                    pointHoverRadius: 6,
                    borderColor: "#a98bd4",
                    backgroundColor: "rgba(169,139,212,0.10)",
                    spanGaps: true
                }
            ]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            animation: { duration: 300 },
            interaction: {
                mode: "index",
                intersect: false
            },
            plugins: {
                legend: {
                    labels: {
                        font: {
                            family: "JetBrains Mono"
                        }
                    }
                }
            },
            scales: {
                x: {
                    ticks: {
                        font: {
                            family: "JetBrains Mono",
                            size: 9
                        }
                    },
                    grid: {
                        color: "rgba(216,91,136,0.08)"
                    }
                },
                temperature: {
                    type: "linear",
                    position: "left",
                    title: {
                        display: true,
                        text: "Temperature (°C)"
                    },
                    grid: {
                        color: "rgba(216,91,136,0.08)"
                    }
                },
                humidity: {
                    type: "linear",
                    position: "right",
                    title: {
                        display: true,
                        text: "Humidity (%)"
                    },
                    grid: {
                        drawOnChartArea: false
                    }
                }
            }
        }
    });
}

// =========================================================
// HISTORY
// =========================================================

function updateHistory() {
    if (!historyBody) return;

    const date = historyDate?.value || selectedDate;
    const readings = getReadingsForDate(date);

    historyBody.innerHTML = "";

    if (readings.length === 0) {
        const row = document.createElement("tr");
        const cell = document.createElement("td");

        cell.colSpan = 4;
        cell.className = "empty-cell";
        cell.textContent = "No sensor data available.";

        row.appendChild(cell);
        historyBody.appendChild(row);

        if (recordCount) recordCount.textContent = "0 records";
        return;
    }

    readings.slice().reverse().forEach((reading, index) => {
        const row = document.createElement("tr");

        const numberCell = document.createElement("td");
        const timeCell = document.createElement("td");
        const temperatureCell = document.createElement("td");
        const humidityCell = document.createElement("td");

        numberCell.textContent = index + 1;
        timeCell.textContent = reading.time;
        temperatureCell.textContent = `${formatNumber(reading.temperature)} °C`;
        humidityCell.textContent = `${formatNumber(reading.humidity)} %`;

        row.append(
            numberCell,
            timeCell,
            temperatureCell,
            humidityCell
        );

        historyBody.appendChild(row);
    });

    if (recordCount) {
        recordCount.textContent =
            `${readings.length} ${readings.length === 1 ? "record" : "records"}`;
    }
}

// =========================================================
// DASHBOARD
// =========================================================

function updateDashboard() {
    const dates = getDateList(allSensorData);

    if (dates.length === 0) {
        updateCurrentReading();
        populateDateSelects();
        updateChart();
        updateHistory();
        return;
    }

    if (!selectedDate || !dates.includes(selectedDate)) {
        selectedDate = dates[0];
    }

    populateDateSelects();
    updateCurrentReading();
    updateChart();
    updateHistory();

    const updateStatus = document.getElementById("updateStatus");
    if (updateStatus) {
        updateStatus.textContent = "Last realtime update received";
    }

    const topStatus = document.getElementById("updateStatusTop");
    if (topStatus) {
        topStatus.textContent = "REALTIME";
    }
}

// =========================================================
// DATE EVENTS
// =========================================================

if (graphDate) {
    graphDate.addEventListener("change", () => {
        selectedDate = graphDate.value;

        if (historyDate) {
            historyDate.value = selectedDate;
        }

        updateChart();
        updateHistory();
    });
}

if (historyDate) {
    historyDate.addEventListener("change", () => {
        selectedDate = historyDate.value;

        if (graphDate) {
            graphDate.value = selectedDate;
        }

        updateHistory();
        updateChart();
    });
}

// =========================================================
// HISTORY TOGGLE
// =========================================================

if (toggleHistory) {
    toggleHistory.addEventListener("click", () => {
        const hidden = historyContent.classList.contains("hidden");

        if (hidden) {
            historyContent.classList.remove("hidden");
            toggleHistory.textContent = "HIDE HISTORY";
            updateHistory();
        } else {
            historyContent.classList.add("hidden");
            toggleHistory.textContent = "SHOW HISTORY";
        }
    });
}

// =========================================================
// FIREBASE LISTENER
// =========================================================

setStatus("CONNECTING...", false);

onValue(
    dataRef,
    snapshot => {
        allSensorData = snapshot.val() || {};

        setStatus("CONNECTED", true);
        updateDashboard();

        console.log("Firebase data received:", allSensorData);
    },
    error => {
        console.error("Firebase read error:", error);
        setStatus("ERROR", false);
    }
);
