#!/usr/bin/env python3
"""
Script de control automático para ESP32 via Arduino
Automatiza el proceso de subida de firmware al ESP32

Uso:
    python esp32_uploader.py [--port COM5] [--action upload|uploadfs]

Requiere:
    pip install pyserial
"""

import serial
import subprocess
import time
import sys
import argparse
from pathlib import Path

class ESP32Controller:
    def __init__(self, arduino_port='COM7', baud_rate=9600):
        self.arduino_port = arduino_port
        self.baud_rate = baud_rate
        self.arduino = None
        
    def connect_arduino(self):
        """Conectar al Arduino controlador"""
        try:
            print(f"🔌 Intentando conectar al Arduino en {self.arduino_port}...")
            self.arduino = serial.Serial(self.arduino_port, self.baud_rate, timeout=2)
            time.sleep(3)  # Esperar inicialización del Arduino
            
            # Limpiar buffer
            self.arduino.reset_input_buffer()
            
            # Leer mensajes de inicialización con timeout
            start_time = time.time()
            while time.time() - start_time < 2:  # 2 segundos para inicialización
                if self.arduino.in_waiting > 0:
                    try:
                        line = self.arduino.readline().decode('utf-8', errors='ignore').strip()
                        if line:
                            print(f"Arduino: {line}")
                    except:
                        break
                else:
                    time.sleep(0.1)
                
            print(f"✅ Conectado al Arduino en {self.arduino_port}")
            
            # Probar comunicación básica
            print("🧪 Probando comunicación...")
            if self.send_command("STATUS"):
                print("✅ Comunicación con Arduino OK")
                return True
            else:
                print("⚠️ Arduino conectado pero no responde correctamente")
                return True  # Intentar continuar de todas formas
            
        except serial.SerialException as e:
            print(f"❌ Error conectando al Arduino: {e}")
            return False
    
    def send_command(self, command):
        """Enviar comando al Arduino y leer respuesta"""
        if not self.arduino:
            print("❌ Arduino no conectado")
            return False
            
        try:
            # Limpiar buffer de entrada
            self.arduino.reset_input_buffer()
            
            # Enviar comando
            print(f"📤 Enviando comando: {command}")
            self.arduino.write(f"{command}\n".encode())
            self.arduino.flush()
            time.sleep(0.5)
            
            # Leer respuesta con timeout más agresivo
            responses = []
            start_time = time.time()
            timeout = 5  # 5 segundos timeout
            
            while time.time() - start_time < timeout:
                try:
                    if self.arduino.in_waiting > 0:
                        line = self.arduino.readline().decode('utf-8', errors='ignore').strip()
                        if line:
                            responses.append(line)
                            print(f"Arduino: {line}")
                            
                            # Si recibimos "OK" o "ERROR", salir del bucle
                            if "OK" in line.upper() or "ERROR" in line.upper():
                                break
                    else:
                        time.sleep(0.1)
                except serial.SerialException as e:
                    print(f"⚠️ Error de comunicación serial: {e}")
                    break
                    
            if not responses:
                print(f"⚠️ Sin respuesta del Arduino después de {timeout}s")
                return False
                
            return True
            
        except Exception as e:
            print(f"❌ Error enviando comando: {e}")
            return False
    
    def enter_download_mode(self):
        """Poner ESP32 en modo download"""
        print("🔄 Poniendo ESP32 en modo download...")
        return self.send_command("DOWNLOAD")
    
    def release_esp32(self):
        """Liberar ESP32 a modo normal"""
        print("🔄 Liberando ESP32 a modo normal...")
        return self.send_command("RELEASE")
    
    def get_status(self):
        """Obtener estado del sistema"""
        print("📊 Consultando estado...")
        return self.send_command("STATUS")
    
    def disconnect(self):
        """Desconectar del Arduino"""
        if self.arduino:
            self.arduino.close()
            self.arduino = None
            print("🔌 Desconectado del Arduino")

def run_platformio_command(action='upload'):
    """Ejecutar comando de PlatformIO"""
    if action == 'upload':
        cmd = ['pio', 'run', '-t', 'upload']
        print("📦 Subiendo firmware...")
    elif action == 'uploadfs':
        cmd = ['pio', 'run', '-t', 'uploadfs']
        print("📁 Subiendo sistema de archivos...")
    else:
        print(f"❌ Acción no reconocida: {action}")
        return False
    
    try:
        print(f"▶️  Ejecutando: {' '.join(cmd)}")
        result = subprocess.run(cmd, capture_output=True, text=True, cwd=Path.cwd())
        
        # Mostrar salida
        if result.stdout:
            print("📤 Salida:")
            print(result.stdout)
            
        if result.stderr:
            print("⚠️  Errores:")
            print(result.stderr)
        
        success = result.returncode == 0
        if success:
            print("✅ Comando PlatformIO exitoso")
        else:
            print(f"❌ Comando PlatformIO falló (código: {result.returncode})")
            
        return success
        
    except FileNotFoundError:
        print("❌ PlatformIO no encontrado. ¿Está instalado y en el PATH?")
        return False
    except Exception as e:
        print(f"❌ Error ejecutando PlatformIO: {e}")
        return False

def main():
    parser = argparse.ArgumentParser(description='Controlador automático ESP32')
    parser.add_argument('--port', default='COM7', help='Puerto del Arduino (default: COM7)')
    parser.add_argument('--action', choices=['upload', 'uploadfs'], default='upload',
                       help='Acción a realizar (default: upload)')
    parser.add_argument('--status', action='store_true', help='Solo mostrar estado')
    
    args = parser.parse_args()
    
    print("🚀 ESP32 Automatic Upload Controller")
    print("====================================")
    
    # Crear controlador
    controller = ESP32Controller(args.port)
    
    try:
        # Conectar al Arduino
        if not controller.connect_arduino():
            return 1
        
        # Solo mostrar estado si se pidió
        if args.status:
            controller.get_status()
            return 0
        
        # Proceso completo de upload
        print(f"\n🎯 Iniciando {args.action}...")
        
        # 1. Poner en modo download
        if not controller.enter_download_mode():
            print("❌ Error poniendo ESP32 en modo download")
            return 1
        
        # 2. Esperar un poco
        print("⏳ Esperando estabilización...")
        time.sleep(2)
        
        # 3. Ejecutar PlatformIO
        success = run_platformio_command(args.action)
        
        # 4. Liberar ESP32 (siempre, aunque falle el upload)
        print("\n🔄 Liberando ESP32...")
        controller.release_esp32()
        
        # 5. Resultado final
        if success:
            print("\n🎉 ¡Upload completado exitosamente!")
            return 0
        else:
            print("\n💥 Upload falló")
            return 1
            
    except KeyboardInterrupt:
        print("\n⚠️  Interrumpido por usuario")
        controller.release_esp32()
        return 1
        
    except Exception as e:
        print(f"\n❌ Error inesperado: {e}")
        controller.release_esp32()
        return 1
        
    finally:
        controller.disconnect()

if __name__ == "__main__":
    exit(main())