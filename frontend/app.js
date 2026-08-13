// ==========================================
// Configuration
// ==========================================
const DEFAULT_CONFIG = {
  apiBaseUrl: 'https://aeroshield-backend.onrender.com', // Replace with your actual Render URL
  city: 'chikodi'
};

// Load configuration from LocalStorage or fallback to Defaults
let appConfig = { ...DEFAULT_CONFIG };
const savedConfig = localStorage.getItem('aeroshield_config');
if (savedConfig) {
  try {
    appConfig = JSON.parse(savedConfig);
  } catch (e) {
    console.error("Failed to parse saved config, using defaults.", e);
  }
}

// ==========================================
// State Management
// ==========================================
let isConnected = false;
let autoSimInterval = null;
let lastValues = { temp: null, humidity: null, ppm: null, aqi: null };
let pollingInterval = null;

// Chart Instances
let tempHumChart = null;
let aqiPpmChart = null;

// DOM Elements
const connStatus = document.getElementById('connStatus');
const connStatusLabel = connStatus.querySelector('.status-label');
const currentCityText = document.getElementById('currentCityText');
const lastSyncTime = document.getElementById('lastSyncTime');

// Live metric fields
const liveAQI = document.getElementById('liveAQI');
const liveCategory = document.getElementById('liveCategory');
const aqiProgress = document.getElementById('aqiProgress');
const liveTemp = document.getElementById('liveTemp');
const tempTrend = document.getElementById('tempTrend');
const liveHumidity = document.getElementById('liveHumidity');
const humidityTrend = document.getElementById('humidityTrend');
const livePPM = document.getElementById('livePPM');
const ppmTrend = document.getElementById('ppmTrend');
const aqiCard = document.getElementById('aqiCard');

// Modal Elements
const settingsModal = document.getElementById('settingsModal');
const openSettingsBtn = document.getElementById('openSettingsBtn');
const closeSettingsBtn = document.getElementById('closeSettingsBtn');
const configForm = document.getElementById('configForm');
const resetConfigBtn = document.getElementById('resetConfigBtn');



// ==========================================
// API Functions
// ==========================================
async function fetchRealtimeData() {
  try {
    const response = await fetch(`${appConfig.apiBaseUrl}/api/cities/${appConfig.city}/realtime`);
    if (response.ok) {
      const data = await response.json();
      updateRealtimeUI(data);
      if (!isConnected) {
        updateConnectionUI(true, "Connected");
      }
      return data;
    } else {
      throw new Error(`HTTP error! status: ${response.status}`);
    }
  } catch (error) {
    console.error('Error fetching realtime data:', error);
    if (isConnected) {
      updateConnectionUI(false, "Connection Error");
    }
    return null;
  }
}

async function fetchHistoryData() {
  try {
    const response = await fetch(`${appConfig.apiBaseUrl}/api/cities/${appConfig.city}/history?limit=30`);
    if (response.ok) {
      const data = await response.json();
      updateChartsData(data);
      return data;
    } else {
      throw new Error(`HTTP error! status: ${response.status}`);
    }
  } catch (error) {
    console.error('Error fetching history data:', error);
    return null;
  }
}



// ==========================================
// Initialization
// ==========================================
function initDashboard() {
  // Set UI Header City
  currentCityText.textContent = appConfig.city;
  
  // Setup charts
  setupCharts();
  
  // Check backend health
  checkBackendHealth();
  
  // Start polling for data
  startDataPolling();
}

async function checkBackendHealth() {
  try {
    const response = await fetch(`${appConfig.apiBaseUrl}/health`);
    if (response.ok) {
      const health = await response.json();
      console.log('Backend health:', health);
      updateConnectionUI(true, "Connected");
    } else {
      throw new Error('Backend health check failed');
    }
  } catch (error) {
    console.error('Backend health check failed:', error);
    updateConnectionUI(false, "Backend Offline");
    startDemoMode();
  }
}

function startDataPolling() {
  // Initial fetch
  fetchRealtimeData();
  fetchHistoryData();
  
  // Poll every 5 seconds
  pollingInterval = setInterval(() => {
    fetchRealtimeData();
    fetchHistoryData();
  }, 5000);
}

// ==========================================
// Demo Mode (offline fallback)
// ==========================================
function startDemoMode() {
  console.log("🎭 Starting demo mode...");
  let t = 28.5, h = 64, p = 220;
  const hist = {};

  const tick = () => {
    t = +(Math.max(20, Math.min(40, t + (Math.random()-0.5)*0.8))).toFixed(1);
    h = Math.round(Math.max(30, Math.min(95, h + (Math.random()-0.5)*3)));
    p = Math.round(Math.max(50, Math.min(700, p + (Math.random()-0.5)*25)));
    const aqi = Math.round((p/1023)*500);
    const ts = Math.round(Date.now()/1000);
    const data = { temperature: t, humidity: h, ppm: p, aqi, category: categoryAQI(aqi), timestamp: ts };
    updateRealtimeUI(data);
    hist[ts] = data;
    const keys = Object.keys(hist);
    if (keys.length > 30) delete hist[keys[0]];
    updateChartsData({ ...hist });
  };
  tick();
  setInterval(tick, 5000);
}

// ==========================================
// UI Updates
// ==========================================
function updateConnectionUI(online, label) {
  isConnected = online;
  if (online) {
    connStatus.className = "connection-status online";
    connStatusLabel.textContent = label;
  } else {
    connStatus.className = "connection-status offline";
    connStatusLabel.textContent = label;
  }
}

function updateRealtimeUI(data) {
  const { temperature, humidity, ppm, aqi, category, timestamp } = data;

  // Format timestamp
  let dateObj = timestamp ? new Date(timestamp * 1000) : new Date();
  if (isNaN(dateObj.getTime())) dateObj = new Date();
  const timeStr = dateObj.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit', second: '2-digit' });
  lastSyncTime.textContent = timeStr;

  // Set Numeric Values
  liveTemp.textContent = temperature !== undefined ? Number(temperature).toFixed(1) : "--";
  liveHumidity.textContent = humidity !== undefined ? Math.round(humidity) : "--";
  livePPM.textContent = ppm !== undefined ? ppm : "--";
  liveAQI.textContent = aqi !== undefined ? aqi : "--";
  
  const cat = category || categoryAQI(aqi || 0);
  liveCategory.textContent = cat;

  // Set AQI Progress bar
  const progressPercent = aqi !== undefined ? Math.min((aqi / 500) * 100, 100) : 0;
  aqiProgress.style.width = `${progressPercent}%`;

  // Apply AQI Category styling
  const cleanCat = cat.replace(/\s+/g, '').toLowerCase();
  liveCategory.className = `aqi-category cat-${cleanCat}`;
  
  // Update progress bar color & card glow based on category
  let progressColor = "var(--color-good)";
  let cardGlow = "var(--shadow-glow-good)";
  let topBarColor = "var(--color-good)";
  
  if (cleanCat === "good") {
    progressColor = "var(--color-good)";
    cardGlow = "var(--shadow-glow-good)";
    topBarColor = "var(--color-good)";
  } else if (cleanCat === "moderate") {
    progressColor = "var(--color-moderate)";
    cardGlow = "var(--shadow-glow-moderate)";
    topBarColor = "var(--color-moderate)";
  } else if (cleanCat === "usg") {
    progressColor = "var(--color-usg)";
    cardGlow = "var(--shadow-glow-moderate)";
    topBarColor = "var(--color-usg)";
  } else if (cleanCat === "unhealthy") {
    progressColor = "var(--color-unhealthy)";
    cardGlow = "var(--shadow-glow-unhealthy)";
    topBarColor = "var(--color-unhealthy)";
  } else if (cleanCat === "veryunhealthy") {
    progressColor = "var(--color-veryunhealthy)";
    cardGlow = "var(--shadow-glow-hazardous)";
    topBarColor = "var(--color-veryunhealthy)";
  } else { // Hazardous
    progressColor = "var(--color-hazardous)";
    cardGlow = "var(--shadow-glow-hazardous)";
    topBarColor = "var(--color-hazardous)";
  }

  aqiProgress.style.backgroundColor = progressColor;
  aqiCard.style.boxShadow = cardGlow;
  
  // Set top glow accent line on AQI card
  const topGlow = aqiCard.querySelector('.card-glow');
  if (topGlow) {
    topGlow.style.background = topBarColor;
  }

  // Calculate & Set Trends
  calculateTrend(temperature, 'temp', tempTrend, "°C");
  calculateTrend(humidity, 'humidity', humidityTrend, "%");
  calculateTrend(ppm, 'ppm', ppmTrend, " PPM");

  // Keep state variables updated
  lastValues = { temp: temperature, humidity, ppm, aqi };
}

function calculateTrend(newVal, key, targetEl, unit) {
  const prevVal = lastValues[key];
  if (prevVal === null || prevVal === undefined || prevVal === newVal) {
    targetEl.innerHTML = `<i class="fa-solid fa-arrow-right trend-stable"></i> Stable`;
    targetEl.className = "metric-trend trend-stable";
    return;
  }

  const diff = newVal - prevVal;
  const isUp = diff > 0;
  const direction = isUp ? "up" : "down";
  const icon = isUp ? "fa-arrow-trend-up" : "fa-arrow-trend-down";
  const displayDiff = Math.abs(diff).toFixed(1);

  targetEl.innerHTML = `<i class="fa-solid ${icon}"></i> ${displayDiff}${unit} ${direction}`;
  targetEl.className = `metric-trend trend-${direction}`;
}

// AQI conversion helper for Simulator
function categoryAQI(aqi) {
  if (aqi <= 50) return "Good";
  if (aqi <= 100) return "Moderate";
  if (aqi <= 150) return "USG";
  if (aqi <= 200) return "Unhealthy";
  if (aqi <= 300) return "Very Unhealthy";
  return "Hazardous";
}

function calculateAQI(ppm) {
  let aqi = Math.round((ppm / 1023) * 500);
  if (aqi < 0) aqi = 0;
  if (aqi > 500) aqi = 500;
  return aqi;
}

// ==========================================
// Charts Configurations
// ==========================================
function setupCharts() {
  const ctx1 = document.getElementById('tempHumChart').getContext('2d');
  const ctx2 = document.getElementById('aqiPpmChart').getContext('2d');

  // Chart styling defaults
  Chart.defaults.color = '#64748b';
  Chart.defaults.font.family = "'Outfit', sans-serif";

  // Temperature & Humidity
  tempHumChart = new Chart(ctx1, {
    type: 'line',
    data: {
      labels: [],
      datasets: [
        {
          label: 'Temperature (°C)',
          data: [],
          borderColor: '#38bdf8',
          backgroundColor: 'rgba(56, 189, 248, 0.05)',
          borderWidth: 2.5,
          tension: 0.4,
          fill: true,
          yAxisID: 'yTemp'
        },
        {
          label: 'Humidity (%)',
          data: [],
          borderColor: '#06b6d4',
          backgroundColor: 'rgba(6, 182, 212, 0.05)',
          borderWidth: 2.5,
          tension: 0.4,
          fill: true,
          yAxisID: 'yHum'
        }
      ]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      plugins: {
        legend: { position: 'top', labels: { boxWidth: 12, padding: 15, color: '#334155' } },
        tooltip: { padding: 10, cornerRadius: 8, backgroundColor: 'rgba(255,255,255,0.95)', titleColor: '#0f172a', bodyColor: '#475569', borderColor: 'rgba(99,102,241,0.15)', borderWidth: 1 }
      },
      scales: {
        x: { grid: { color: 'rgba(0, 0, 0, 0.05)' }, ticks: { color: '#64748b' } },
        yTemp: {
          type: 'linear',
          position: 'left',
          title: { display: true, text: 'Temp (°C)', color: '#0284c7' },
          grid: { color: 'rgba(0, 0, 0, 0.05)' },
          ticks: { color: '#0284c7' },
          suggestedMin: 15,
          suggestedMax: 40
        },
        yHum: {
          type: 'linear',
          position: 'right',
          title: { display: true, text: 'Humidity (%)', color: '#0891b2' },
          ticks: { color: '#0891b2' },
          grid: { drawOnChartArea: false },
          suggestedMin: 20,
          suggestedMax: 100
        }
      }
    }
  });

  // AQI & PPM
  aqiPpmChart = new Chart(ctx2, {
    type: 'line',
    data: {
      labels: [],
      datasets: [
        {
          label: 'AQI Index',
          data: [],
          borderColor: '#22c55e',
          backgroundColor: 'rgba(34, 197, 94, 0.05)',
          borderWidth: 2.5,
          tension: 0.4,
          fill: true,
          yAxisID: 'yAqi'
        },
        {
          label: 'Gas Output (PPM)',
          data: [],
          borderColor: '#c084fc',
          backgroundColor: 'rgba(192, 132, 252, 0.05)',
          borderWidth: 2.5,
          tension: 0.4,
          fill: true,
          yAxisID: 'yPpm'
        }
      ]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      plugins: {
        legend: { position: 'top', labels: { boxWidth: 12, padding: 15, color: '#334155' } },
        tooltip: { padding: 10, cornerRadius: 8, backgroundColor: 'rgba(255,255,255,0.95)', titleColor: '#0f172a', bodyColor: '#475569', borderColor: 'rgba(99,102,241,0.15)', borderWidth: 1 }
      },
      scales: {
        x: { grid: { color: 'rgba(0, 0, 0, 0.05)' }, ticks: { color: '#64748b' } },
        yAqi: {
          type: 'linear',
          position: 'left',
          title: { display: true, text: 'AQI Index', color: '#16a34a' },
          grid: { color: 'rgba(0, 0, 0, 0.05)' },
          ticks: { color: '#16a34a' },
          suggestedMin: 0,
          suggestedMax: 500
        },
        yPpm: {
          type: 'linear',
          position: 'right',
          title: { display: true, text: 'Gas (PPM)', color: '#7c3aed' },
          ticks: { color: '#7c3aed' },
          grid: { drawOnChartArea: false },
          suggestedMin: 0,
          suggestedMax: 1023
        }
      }
    }
  });
}

function updateChartsData(historyData) {
  // historyData is a map of timestamps -> record objects
  const records = Object.values(historyData);
  
  // Sort by timestamp asc
  records.sort((a, b) => a.timestamp - b.timestamp);

  const labels = [];
  const temps = [];
  const hums = [];
  const aqis = [];
  const ppms = [];

  records.forEach(r => {
    const timeVal = r.timestamp ? new Date(r.timestamp * 1000) : new Date();
    const formattedTime = timeVal.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' });
    labels.push(formattedTime);
    temps.push(r.temperature !== undefined ? r.temperature : null);
    hums.push(r.humidity !== undefined ? r.humidity : null);
    aqis.push(r.aqi !== undefined ? r.aqi : null);
    ppms.push(r.ppm !== undefined ? r.ppm : null);
  });

  // Update Temp & Hum Chart
  tempHumChart.data.labels = labels;
  tempHumChart.data.datasets[0].data = temps;
  tempHumChart.data.datasets[1].data = hums;
  tempHumChart.update('none'); // silent update

  // Update AQI & PPM Chart
  aqiPpmChart.data.labels = labels;
  aqiPpmChart.data.datasets[0].data = aqis;
  // Dynamically update AQI dataset border color based on the last reading
  if (aqis.length > 0) {
    const lastAqi = aqis[aqis.length - 1];
    let aqiColor = '#22c55e';
    if (lastAqi > 300) aqiColor = '#881337';
    else if (lastAqi > 200) aqiColor = '#a855f7';
    else if (lastAqi > 150) aqiColor = '#ef4444';
    else if (lastAqi > 100) aqiColor = '#f97316';
    else if (lastAqi > 50) aqiColor = '#eab308';
    
    aqiPpmChart.data.datasets[0].borderColor = aqiColor;
  }
  aqiPpmChart.data.datasets[1].data = ppms;
  aqiPpmChart.update('none');
}



// ==========================================
// Settings Modal Controls
// ==========================================
function openSettings() {
  document.getElementById('cfgApiKey').value = appConfig.apiBaseUrl;
  document.getElementById('cfgDbUrl').value = appConfig.apiBaseUrl;
  document.getElementById('cfgAuthEmail').value = '';
  document.getElementById('cfgAuthPass').value = '';
  document.getElementById('cfgCity').value = appConfig.city;
  
  settingsModal.classList.add('active');
}

function closeSettings() {
  settingsModal.classList.remove('active');
}

function saveConfig(e) {
  e.preventDefault();
  
  const newConfig = {
    apiBaseUrl: document.getElementById('cfgApiKey').value.trim(),
    city: document.getElementById('cfgCity').value.trim().toLowerCase()
  };

  localStorage.setItem('aeroshield_config', JSON.stringify(newConfig));
  closeSettings();
  
  // Reload page to apply new configuration
  window.location.reload();
}

function resetConfig() {
  if (confirm("Are you sure you want to reset configuration back to defaults?")) {
    localStorage.removeItem('aeroshield_config');
    window.location.reload();
  }
}

// ==========================================
// Event Bindings
// ==========================================
function bindEvents() {
  // Settings modal
  openSettingsBtn.addEventListener('click', openSettings);
  closeSettingsBtn.addEventListener('click', closeSettings);
  settingsModal.addEventListener('click', (e) => {
    if (e.target === settingsModal) closeSettings();
  });
  configForm.addEventListener('submit', saveConfig);
  resetConfigBtn.addEventListener('click', resetConfig);
}

// ==========================================
// Start Dashboard
// ==========================================
document.addEventListener('DOMContentLoaded', () => {
  bindEvents();
  initDashboard();
});
