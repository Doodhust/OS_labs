async function fetchCurrentTemperature() {
    const response = await fetch('/current');
    const data = await response.json();
    document.getElementById('current-temperature').innerText = `Current Temperature: ${data.temperature}°C`;
}

async function fetchStats() {
    const response = await fetch('/stats?start=2023-10-01&end=2023-10-31');
    const data = await response.json();
    document.getElementById('stats').innerText = `Stats: ${JSON.stringify(data)}`;
}

setInterval(fetchCurrentTemperature, 1000);
fetchStats();