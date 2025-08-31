#!/usr/bin/env python3
"""
Vibration Data Analysis Script
Analyzes CSV data collected from ESP32 IMU vibration tracker
"""

import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from scipy import signal
from scipy.fft import fft, fftfreq
import argparse
import sys
import os

class VibrationAnalyzer:
    def __init__(self, csv_file):
        """Initialize the analyzer with CSV data file"""
        self.csv_file = csv_file
        self.data = None
        self.sample_rate = 100  # Default sample rate (Hz)
        
    def load_data(self):
        """Load and preprocess the CSV data"""
        try:
            # Read CSV file
            self.data = pd.read_csv(self.csv_file)
            
            # Check if required columns exist
            required_columns = ['Timestamp', 'AccelX', 'AccelY', 'AccelZ']
            missing_columns = [col for col in required_columns if col not in self.data.columns]
            
            if missing_columns:
                print(f"Error: Missing required columns: {missing_columns}")
                return False
                
            # Convert timestamp to relative time (seconds)
            if 'Timestamp' in self.data.columns:
                self.data['Time'] = (self.data['Timestamp'] - self.data['Timestamp'].iloc[0]) / 1000.0
            
            # Calculate acceleration magnitude
            self.data['AccelMagnitude'] = np.sqrt(
                self.data['AccelX']**2 + 
                self.data['AccelY']**2 + 
                self.data['AccelZ']**2
            )
            
            # Calculate RMS acceleration
            window_size = 100  # samples
            self.data['AccelRMS'] = self.data['AccelMagnitude'].rolling(
                window=window_size, center=True
            ).apply(lambda x: np.sqrt(np.mean(x**2)))
            
            print(f"Data loaded successfully: {len(self.data)} samples")
            print(f"Duration: {self.data['Time'].iloc[-1]:.2f} seconds")
            return True
            
        except Exception as e:
            print(f"Error loading data: {e}")
            return False
    
    def basic_statistics(self):
        """Calculate and display basic statistics"""
        print("\n=== BASIC STATISTICS ===")
        
        # Acceleration statistics
        accel_stats = self.data[['AccelX', 'AccelY', 'AccelZ', 'AccelMagnitude']].describe()
        print("\nAcceleration Statistics (m/s²):")
        print(accel_stats.round(3))
        
        # Gyroscope statistics (if available)
        if all(col in self.data.columns for col in ['GyroX', 'GyroY', 'GyroZ']):
            gyro_stats = self.data[['GyroX', 'GyroY', 'GyroZ']].describe()
            print("\nGyroscope Statistics (rad/s):")
            print(gyro_stats.round(3))
        
        # Temperature statistics (if available)
        if 'Temperature' in self.data.columns:
            temp_stats = self.data['Temperature'].describe()
            print("\nTemperature Statistics (°C):")
            print(temp_stats.round(1))
    
    def plot_time_series(self):
        """Plot time series data"""
        fig, axes = plt.subplots(2, 2, figsize=(15, 10))
        fig.suptitle('Vibration Data - Time Series Analysis', fontsize=16)
        
        # Acceleration components
        axes[0, 0].plot(self.data['Time'], self.data['AccelX'], label='X', alpha=0.7)
        axes[0, 0].plot(self.data['Time'], self.data['AccelY'], label='Y', alpha=0.7)
        axes[0, 0].plot(self.data['Time'], self.data['AccelZ'], label='Z', alpha=0.7)
        axes[0, 0].set_title('Acceleration Components')
        axes[0, 0].set_xlabel('Time (s)')
        axes[0, 0].set_ylabel('Acceleration (m/s²)')
        axes[0, 0].legend()
        axes[0, 0].grid(True, alpha=0.3)
        
        # Acceleration magnitude
        axes[0, 1].plot(self.data['Time'], self.data['AccelMagnitude'], 'r-', alpha=0.7)
        axes[0, 1].set_title('Acceleration Magnitude')
        axes[0, 1].set_xlabel('Time (s)')
        axes[0, 1].set_ylabel('Magnitude (m/s²)')
        axes[0, 1].grid(True, alpha=0.3)
        
        # RMS acceleration
        axes[1, 0].plot(self.data['Time'], self.data['AccelRMS'], 'g-', alpha=0.7)
        axes[1, 0].set_title('RMS Acceleration')
        axes[1, 0].set_xlabel('Time (s)')
        axes[1, 0].set_ylabel('RMS (m/s²)')
        axes[1, 0].grid(True, alpha=0.3)
        
        # Gyroscope (if available)
        if all(col in self.data.columns for col in ['GyroX', 'GyroY', 'GyroZ']):
            axes[1, 1].plot(self.data['Time'], self.data['GyroX'], label='X', alpha=0.7)
            axes[1, 1].plot(self.data['Time'], self.data['GyroY'], label='Y', alpha=0.7)
            axes[1, 1].plot(self.data['Time'], self.data['GyroZ'], label='Z', alpha=0.7)
            axes[1, 1].set_title('Gyroscope Components')
            axes[1, 1].set_xlabel('Time (s)')
            axes[1, 1].set_ylabel('Angular Velocity (rad/s)')
            axes[1, 1].legend()
        else:
            # Temperature plot if gyroscope not available
            if 'Temperature' in self.data.columns:
                axes[1, 1].plot(self.data['Time'], self.data['Temperature'], 'orange', alpha=0.7)
                axes[1, 1].set_title('Temperature')
                axes[1, 1].set_xlabel('Time (s)')
                axes[1, 1].set_ylabel('Temperature (°C)')
            else:
                axes[1, 1].text(0.5, 0.5, 'No additional data available', 
                               ha='center', va='center', transform=axes[1, 1].transAxes)
                axes[1, 1].set_title('Additional Data')
        
        axes[1, 1].grid(True, alpha=0.3)
        
        plt.tight_layout()
        plt.show()
    
    def frequency_analysis(self):
        """Perform frequency domain analysis"""
        print("\n=== FREQUENCY ANALYSIS ===")
        
        # Calculate FFT for acceleration magnitude
        accel_mag = self.data['AccelMagnitude'].values
        n_samples = len(accel_mag)
        
        # Apply window to reduce spectral leakage
        windowed_data = accel_mag * signal.windows.hann(n_samples)
        
        # Calculate FFT
        fft_data = fft(windowed_data)
        freqs = fftfreq(n_samples, 1/self.sample_rate)
        
        # Only use positive frequencies
        positive_freqs = freqs[:n_samples//2]
        positive_fft = np.abs(fft_data[:n_samples//2])
        
        # Find dominant frequencies
        peak_indices = signal.find_peaks(positive_fft, height=np.max(positive_fft)*0.1)[0]
        dominant_freqs = positive_freqs[peak_indices]
        dominant_amplitudes = positive_fft[peak_indices]
        
        print(f"Dominant frequencies found:")
        for i, (freq, amp) in enumerate(zip(dominant_freqs[:10], dominant_amplitudes[:10])):
            print(f"  {i+1}. {freq:.2f} Hz (amplitude: {amp:.3f})")
        
        # Plot frequency spectrum
        plt.figure(figsize=(12, 6))
        plt.subplot(1, 2, 1)
        plt.plot(positive_freqs, positive_fft)
        plt.title('Frequency Spectrum')
        plt.xlabel('Frequency (Hz)')
        plt.ylabel('Amplitude')
        plt.grid(True, alpha=0.3)
        plt.xlim(0, self.sample_rate/2)
        
        # Plot spectrogram
        plt.subplot(1, 2, 2)
        f, t, Sxx = signal.spectrogram(accel_mag, self.sample_rate, nperseg=256)
        plt.pcolormesh(t, f, 10 * np.log10(Sxx), shading='gouraud')
        plt.title('Spectrogram')
        plt.xlabel('Time (s)')
        plt.ylabel('Frequency (Hz)')
        plt.colorbar(label='Power (dB)')
        
        plt.tight_layout()
        plt.show()
    
    def vibration_metrics(self):
        """Calculate vibration-specific metrics"""
        print("\n=== VIBRATION METRICS ===")
        
        # Peak acceleration
        peak_accel = self.data['AccelMagnitude'].max()
        print(f"Peak Acceleration: {peak_accel:.3f} m/s²")
        
        # RMS acceleration
        rms_accel = np.sqrt(np.mean(self.data['AccelMagnitude']**2))
        print(f"Overall RMS Acceleration: {rms_accel:.3f} m/s²")
        
        # Crest factor (peak/RMS ratio)
        crest_factor = peak_accel / rms_accel if rms_accel > 0 else 0
        print(f"Crest Factor: {crest_factor:.2f}")
        
        # Kurtosis (measure of peakiness)
        kurtosis = self.data['AccelMagnitude'].kurtosis()
        print(f"Kurtosis: {kurtosis:.2f}")
        
        # Vibration intensity levels
        print(f"\nVibration Intensity Assessment:")
        if rms_accel < 0.5:
            print("  Level: Very Low")
        elif rms_accel < 1.0:
            print("  Level: Low")
        elif rms_accel < 2.0:
            print("  Level: Moderate")
        elif rms_accel < 5.0:
            print("  Level: High")
        else:
            print("  Level: Very High")
    
    def export_analysis(self, output_file=None):
        """Export analysis results to file"""
        if output_file is None:
            base_name = os.path.splitext(self.csv_file)[0]
            output_file = f"{base_name}_analysis.txt"
        
        with open(output_file, 'w') as f:
            f.write("VIBRATION DATA ANALYSIS REPORT\n")
            f.write("=" * 40 + "\n\n")
            
            f.write(f"Data File: {self.csv_file}\n")
            f.write(f"Samples: {len(self.data)}\n")
            f.write(f"Duration: {self.data['Time'].iloc[-1]:.2f} seconds\n")
            f.write(f"Sample Rate: {self.sample_rate} Hz\n\n")
            
            # Basic statistics
            f.write("BASIC STATISTICS\n")
            f.write("-" * 20 + "\n")
            accel_stats = self.data[['AccelX', 'AccelY', 'AccelZ', 'AccelMagnitude']].describe()
            f.write(accel_stats.round(3).to_string())
            f.write("\n\n")
            
            # Vibration metrics
            f.write("VIBRATION METRICS\n")
            f.write("-" * 20 + "\n")
            peak_accel = self.data['AccelMagnitude'].max()
            rms_accel = np.sqrt(np.mean(self.data['AccelMagnitude']**2))
            crest_factor = peak_accel / rms_accel if rms_accel > 0 else 0
            kurtosis = self.data['AccelMagnitude'].kurtosis()
            
            f.write(f"Peak Acceleration: {peak_accel:.3f} m/s²\n")
            f.write(f"RMS Acceleration: {rms_accel:.3f} m/s²\n")
            f.write(f"Crest Factor: {crest_factor:.2f}\n")
            f.write(f"Kurtosis: {kurtosis:.2f}\n")
        
        print(f"Analysis report saved to: {output_file}")
    
    def run_full_analysis(self):
        """Run complete analysis"""
        if not self.load_data():
            return False
        
        self.basic_statistics()
        self.vibration_metrics()
        self.frequency_analysis()
        self.plot_time_series()
        self.export_analysis()
        
        return True

def main():
    parser = argparse.ArgumentParser(description='Analyze vibration data from ESP32 IMU')
    parser.add_argument('csv_file', help='CSV file containing vibration data')
    parser.add_argument('--sample-rate', type=int, default=100, 
                       help='Sample rate in Hz (default: 100)')
    parser.add_argument('--no-plots', action='store_true', 
                       help='Skip plotting (useful for headless operation)')
    parser.add_argument('--export-only', action='store_true',
                       help='Only export analysis report, no plots')
    
    args = parser.parse_args()
    
    if not os.path.exists(args.csv_file):
        print(f"Error: File '{args.csv_file}' not found")
        sys.exit(1)
    
    analyzer = VibrationAnalyzer(args.csv_file)
    analyzer.sample_rate = args.sample_rate
    
    if args.export_only:
        if analyzer.load_data():
            analyzer.basic_statistics()
            analyzer.vibration_metrics()
            analyzer.export_analysis()
    else:
        analyzer.run_full_analysis()

if __name__ == "__main__":
    main()
