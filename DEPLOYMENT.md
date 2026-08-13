# Deployment Guide

This guide will help you deploy AeroShield to production.

## Prerequisites

- GitHub account
- Render account (for backend)
- Vercel account (for frontend)
- HiveMQ Cloud account (already configured)

## Step 1: Push to GitHub

1. Initialize git repository:
   ```bash
   git init
   git add .
   git commit -m "Initial commit: AeroShield MQTT integration"
   ```

2. Create repository on GitHub
3. Push code:
   ```bash
   git remote add origin https://github.com/YOUR_USERNAME/aeroshield-project.git
   git branch -M main
   git push -u origin main
   ```

## Step 2: Deploy Backend to Render

1. Go to [render.com](https://render.com) and sign up
2. Click "New +" → "Web Service"
3. Connect your GitHub repository
4. Configure:
   - **Name**: aeroshield-backend
   - **Root Directory**: `backend`
   - **Build Command**: `npm install`
   - **Start Command**: `node server.js`
   - **Instance Type**: Free

5. Add Environment Variables:
   ```
   PORT=3000
   MQTT_BROKER=97f6bb83bebf454c86a437210b5379b9.s1.eu.hivemq.cloud
   MQTT_PORT=8883
   MQTT_USERNAME=97f6bb83bebf454c86a437210b5379b9
   MQTT_PASSWORD=
   MQTT_TOPIC=aeroshield/sensors/#
   ```

6. Click "Deploy Web Service"
7. Wait for deployment and note your backend URL (e.g., `https://aeroshield-backend.onrender.com`)

## Step 3: Deploy Frontend to Vercel

1. Go to [vercel.com](https://vercel.com) and sign up
2. Click "Add New Project"
3. Import your GitHub repository
4. Configure:
   - **Framework Preset**: Other
   - **Root Directory**: `frontend`
   - **Build Command**: (leave empty)
   - **Output Directory**: `./`

5. Add Environment Variables (optional):
   ```
   NEXT_PUBLIC_API_URL=https://aeroshield-backend.onrender.com
   ```

6. Click "Deploy"
7. Wait for deployment and note your frontend URL

## Step 4: Update Frontend Configuration

After deployment, you need to configure the frontend to use your backend URL:

1. Open your deployed frontend
2. Click the settings gear icon
3. Enter your Render backend URL in "API Base URL"
4. Save configuration

Alternatively, update the default in `frontend/app.js` before deploying:
```javascript
const DEFAULT_CONFIG = {
  apiBaseUrl: 'https://aeroshield-backend.onrender.com', // Your Render URL
  city: 'chikodi'
};
```

## Step 5: Flash ESP8266 Firmware

1. Open Arduino IDE
2. Install required libraries (see README.md)
3. Open `AeroShield.ino` (in project root)
4. Select NodeMCU 1.0 board
5. Upload to your ESP8266

## Step 6: Configure ESP8266 WiFi

1. Power on your ESP8266
2. Connect to WiFi network "AeroShield-Setup"
3. Password: `aeroshield123`
4. Browser should open configuration portal automatically
5. Enter your WiFi credentials
6. Optionally set your city name
7. Save - device will restart and connect

## Step 7: Test the System

1. Check Serial Monitor (115200 baud) for connection status
2. Verify backend logs show MQTT connection
3. Open your frontend dashboard
4. Should see real-time data updating every 10 seconds

## Troubleshooting

### Backend Not Connecting to MQTT
- Check Render logs for connection errors
- Verify MQTT credentials in environment variables
- Ensure HiveMQ Cloud cluster is running

### Frontend Not Fetching Data
- Check browser console for CORS errors
- Verify API URL is correct
- Check backend is running and accessible

### ESP8266 Not Publishing Data
- Check Serial Monitor for errors
- Verify WiFi connection
- Ensure MQTT broker address is correct
- Check sensor connections

## Cost Summary

- **Render**: Free tier available
- **Vercel**: Free tier available  
- **HiveMQ Cloud**: Free tier available (your current plan)

Total: $0/month for basic usage

## Scaling Considerations

For production use, consider:
- Upgrade to paid Render/Vercel plans for better performance
- Add authentication to your API
- Implement data persistence in backend
- Add error handling and retry logic
- Monitor system health with uptime monitoring
