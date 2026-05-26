import React, { useState, useEffect } from 'react';
import { StyleSheet, Text, View } from 'react-native';
import { Camera, useCameraDevices, useFrameProcessor } from 'react-native-vision-camera';

export default function GhostCamera() {
  const devices = useCameraDevices();
  const device = devices.front; // We need the selfie camera
  
  // UI State for the Telemetry Overlay
  const [authStatus, setAuthStatus] = useState('INITIALIZING...');
  const [telemetry, setTelemetry] = useState({ score: 0, latency: 0 });

  // ------------------------------------------------------------------
  // 1. THE INGESTION LOOP (Runs at 60 FPS on a background Worklet thread)
  // ------------------------------------------------------------------
  const frameProcessor = useFrameProcessor((frame) => {
    'worklet';
    // This runs completely independently of the UI.
    // We grab the raw memory buffer and fire it into our C++ JSI bridge.
    if (global.ghostSyncEngine) {
       // Convert frame to ArrayBuffer (handled via Vision Camera's internal hooks)
       // and push to the C++ drop-buffer.
       global.ghostSyncEngine.pushFrame(frame.toArrayBuffer(), frame.width, frame.height);
    }
  }, []);

  // ------------------------------------------------------------------
  // 2. THE UI POLLING LOOP (Runs on the main React thread)
  // ------------------------------------------------------------------
  useEffect(() => {
    setAuthStatus('SCANNING FOR FACE...');

    // Poll the C++ engine every 150ms to check the status
    const pollInterval = setInterval(() => {
      if (global.ghostSyncEngine) {
        const result = global.ghostSyncEngine.getLatestResult();
        
        // Update our debug telemetry overlay
        setTelemetry({ score: result.confidenceScore, latency: result.processingTimeMs });

        if (!result.isLive) {
           setAuthStatus('SPOOF DETECTED. PLEASE BLINK.');
        } else if (result.confidenceScore > 0.90) {
           setAuthStatus(`AUTHENTICATED: ${result.userId}`);
           
           // TODO: Trigger AWS Sync worker here!
           clearInterval(pollInterval); // Stop polling once authenticated
        }
      }
    }, 150);

    return () => clearInterval(pollInterval); // Cleanup when component unmounts
  }, []);

  // ------------------------------------------------------------------
  // RENDER THE UI
  // ------------------------------------------------------------------
  if (device == null) return <View style={styles.container}><Text>No Camera Found</Text></View>;

  return (
    <View style={styles.container}>
      {/* The actual hardware camera feed */}
      <Camera
        style={StyleSheet.absoluteFill}
        device={device}
        isActive={authStatus !== 'AUTHENTICATED'} // Turn off camera to save battery once done
        frameProcessor={frameProcessor}
        frameProcessorFps={60} 
      />

      {/* The Hacker-Style Telemetry Overlay */}
      <View style={styles.overlay}>
        <Text style={styles.statusText}>{authStatus}</Text>
        <Text style={styles.telemetryText}>MATCH CONFIDENCE: {(telemetry.score * 100).toFixed(2)}%</Text>
        <Text style={styles.telemetryText}>ENGINE LATENCY: {telemetry.latency}ms</Text>
      </View>
    </View>
  );
}

const styles = StyleSheet.create({
  container: { flex: 1, backgroundColor: 'black' },
  overlay: {
    position: 'absolute',
    bottom: 40,
    left: 20,
    right: 20,
    backgroundColor: 'rgba(0, 255, 0, 0.1)',
    borderWidth: 1,
    borderColor: '#00FF00',
    padding: 15,
  },
  statusText: { color: '#00FF00', fontSize: 20, fontWeight: 'bold', marginBottom: 10 },
  telemetryText: { color: '#00FF00', fontSize: 14, fontFamily: 'monospace' }
});